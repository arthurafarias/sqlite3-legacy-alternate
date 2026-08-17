#!/usr/bin/env python3
"""
Eliminates the use of sqlite/_All.h as a compilation-unit include.

For every src/sqlite/*.c file (which today only does
`#include "sqlite/_All.h"`), this script uses libclang to determine exactly
which sqlite headers and which system headers that translation unit actually
needs, and replaces the _All.h include with the precise set.

Resolution strategy:
  - Type names spelled anywhere in the .c file (variable/param/cast/sizeof
    types, stripped of pointer/const/volatile/array) are matched against the
    set of "container" headers (one header per struct/typedef in
    src/sqlite/*.h, an existing 1:1 convention in this codebase). This avoids
    relying on clang's canonical-declaration choice among the many duplicate
    `typedef struct X X;` forward declarations scattered across headers.
  - Non-type symbols (function calls, global variables, struct field
    accesses, enum constants) are resolved via the referenced declaration's
    source location, which is reliable because those are declared exactly
    once (unlike the forward-declare typedefs).
  - Symbols resolved to a system header (outside src/) are re-mapped by name
    against a symbol table built from the original fixed system-header list
    in _All.h, so that glibc's fortified inline wrappers (bits/*.h) don't leak
    into the includes -- we always emit the canonical <string.h>-style header.
"""
import argparse
import os
import re
import sys
import subprocess
import json

import clang.cindex

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, "..", ".."))
SRC_DIR = os.path.join(REPO_ROOT, "src")
SQLITE_DIR = os.path.join(SRC_DIR, "sqlite")
ALL_H = os.path.join(SQLITE_DIR, "_All.h")

# Original system headers pulled in by _All.h (order preserved for the symbol
# table priority below -- earlier headers win on name collisions).
SYSTEM_HEADERS = [
    "stddef.h", "stdint.h", "stdio.h", "stdlib.h", "string.h", "stdarg.h",
    "assert.h", "ctype.h", "limits.h", "float.h", "math.h", "errno.h",
    "time.h", "sys/types.h", "unistd.h", "pthread.h", "sys/stat.h",
    "sys/time.h", "sys/select.h", "sys/uio.h", "fcntl.h", "dlfcn.h",
    "sched.h", "sys/ioctl.h", "utime.h", "dirent.h", "sys/mman.h",
]

TYPE_STRIP_RE = re.compile(r"\b(const|volatile|struct|union|enum)\b")


def find_libclang():
    for cand in [
        "/usr/lib/libclang.so",
        "/usr/lib/libclang.so.22.1.8",
    ]:
        if os.path.exists(cand):
            return cand
    out = subprocess.run(["clang", "-print-resource-dir"], capture_output=True, text=True)
    raise RuntimeError("libclang.so not found; resource dir=" + out.stdout.strip())


def clang_system_includes():
    out = subprocess.run(
        ["clang", "-E", "-x", "c", "-v", "/dev/null"],
        capture_output=True, text=True,
    )
    lines = out.stderr.splitlines()
    paths = []
    capture = False
    for line in lines:
        if "#include <...> search starts here:" in line:
            capture = True
            continue
        if line.startswith("End of search list"):
            capture = False
            continue
        if capture:
            paths.append(line.strip())
    return paths


def base_type_name(spelling):
    s = TYPE_STRIP_RE.sub("", spelling)
    s = s.replace("*", " ").strip()
    # collapse multiple words (e.g. "unsigned int") -- containers are always
    # single identifiers, so if there's whitespace left it's not a container.
    if not s:
        return None
    parts = s.split()
    return parts[0] if len(parts) == 1 else None


def build_container_set():
    names = set()
    for f in os.listdir(SQLITE_DIR):
        if f.endswith(".h") and f != "_All.h":
            names.add(f[:-2])
    return names


SYMBOL_KINDS = {
    clang.cindex.CursorKind.FUNCTION_DECL,
    clang.cindex.CursorKind.VAR_DECL,
    clang.cindex.CursorKind.TYPEDEF_DECL,
    clang.cindex.CursorKind.MACRO_DEFINITION,
    clang.cindex.CursorKind.STRUCT_DECL,
    clang.cindex.CursorKind.UNION_DECL,
    clang.cindex.CursorKind.ENUM_DECL,
    clang.cindex.CursorKind.ENUM_CONSTANT_DECL,
    clang.cindex.CursorKind.FIELD_DECL,
}


def build_system_symbol_table(clang_args):
    """Map symbol name -> standard header name, by parsing each standard
    header in isolation (cumulatively, each on top of the previous ones so
    later headers don't re-claim symbols already attributed earlier) and
    recording every declaration reachable from it, at any nesting depth --
    covers struct fields and macros that live in glibc's internal bits/*.h
    implementation headers, which we never want to #include directly."""
    table = {}
    tmp_path = "/tmp/_iwyu_probe.c"

    def walk(cursor):
        for c in cursor.get_children():
            if c.kind in SYMBOL_KINDS and c.spelling and c.spelling not in table:
                table[c.spelling] = hdr
            walk(c)

    included_so_far = []
    for hdr in SYSTEM_HEADERS:
        included_so_far.append(hdr)
        src = "".join(f"#include <{h}>\n" for h in included_so_far)
        with open(tmp_path, "w") as f:
            f.write(src)
        idx = clang.cindex.Index.create()
        tu = idx.parse(tmp_path, args=clang_args)
        walk(tu.cursor)
    os.remove(tmp_path)
    return table


NON_TYPE_KINDS = {
    clang.cindex.CursorKind.DECL_REF_EXPR,
    clang.cindex.CursorKind.MEMBER_REF_EXPR,
    clang.cindex.CursorKind.CALL_EXPR,
}


def analyze_file(cfile, clang_args, containers, sys_symbols, verbose=False):
    idx = clang.cindex.Index.create()
    tu = idx.parse(cfile, args=clang_args,
                    options=clang.cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)
    errors = [d for d in tu.diagnostics if d.severity >= clang.cindex.Diagnostic.Error]

    main_real = os.path.realpath(cfile)
    needed_sqlite = set()
    needed_system = set()
    unresolved = set()

    def consider_type(spelling):
        name = base_type_name(spelling)
        if not name:
            return
        if name in containers:
            needed_sqlite.add(name + ".h")
            return
        # not a sqlite container -- might be a system typedef (intptr_t,
        # size_t, ...) spelled directly as a cast/decl type, which never
        # shows up as a DECL_REF_EXPR/MEMBER_REF_EXPR/CALL_EXPR node.
        hdr = sys_symbols.get(name)
        if hdr:
            needed_system.add(hdr)

    def consider_decl_location(decl_cursor):
        if decl_cursor is None or decl_cursor.location.file is None:
            return
        path = os.path.realpath(decl_cursor.location.file.name)
        if path == main_real:
            return
        if path.startswith(SQLITE_DIR + os.sep) or path == ALL_H:
            base = os.path.basename(path)
            if base != "_All.h":
                needed_sqlite.add(base)
            return
        # system symbol -- remap by name to dodge glibc fortify wrappers
        name = decl_cursor.spelling
        hdr = sys_symbols.get(name)
        if hdr:
            needed_system.add(hdr)
        else:
            unresolved.add((name, path))

    def walk(node):
        if node.location.file is not None and os.path.realpath(node.location.file.name) == main_real:
            t = node.type
            if t is not None and t.spelling:
                consider_type(t.spelling)

            if node.kind in NON_TYPE_KINDS:
                consider_decl_location(node.referenced)
        for c in node.get_children():
            walk(c)

    walk(tu.cursor)

    own_header = os.path.basename(cfile)[:-2] + ".h"
    if os.path.exists(os.path.join(SQLITE_DIR, own_header)):
        needed_sqlite.discard(own_header)
    else:
        own_header = None

    return {
        "errors": [str(e) for e in errors],
        "own_header": own_header,
        "sqlite_headers": sorted(needed_sqlite),
        "system_headers": sorted(needed_system),
        "unresolved": sorted(unresolved),
    }


def render_includes(result):
    # System headers must come first: some sqlite headers declare functions
    # taking bare system struct-tag pointers (e.g. `struct flock *`) without
    # including the defining system header themselves, which would otherwise
    # create a locally-scoped incomplete type distinct from the real one and
    # trip a "conflicting types" error once the real header is later seen.
    lines = ["#define _GNU_SOURCE 1", ""]
    if result["system_headers"]:
        for h in result["system_headers"]:
            lines.append(f"#include <{h}>")
        lines.append("")
    if result["own_header"]:
        lines.append(f'#include "sqlite/{result["own_header"]}"')
    if result["sqlite_headers"]:
        if result["own_header"]:
            lines.append("")
        for h in result["sqlite_headers"]:
            lines.append(f'#include "sqlite/{h}"')
    return "\n".join(lines) + "\n"


def apply_to_file(cfile, result, dry_run=False):
    with open(cfile, "r", encoding="utf-8") as f:
        content = f.read()

    include_re = re.compile(r'#include\s+"sqlite/_All\.h"\s*\n')
    if not include_re.search(content):
        return False

    new_includes = render_includes(result)
    new_content = include_re.sub(new_includes, content, count=1)

    if dry_run:
        print(f"--- {cfile} ---")
        print(new_includes)
        return True

    with open(cfile, "w", encoding="utf-8") as f:
        f.write(new_content)
    return True


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("files", nargs="*", help="Specific .c files to process (default: all src/sqlite/*.c using _All.h)")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--report", help="Write JSON report to this path")
    ap.add_argument("--libclang", default=None)
    args = ap.parse_args()

    clang.cindex.Config.set_library_file(args.libclang or find_libclang())

    sysinc = clang_system_includes()
    clang_args = [f"-I{SRC_DIR}", "-std=c23", "-D_GNU_SOURCE=1"]
    for p in sysinc:
        clang_args += ["-isystem", p]

    containers = build_container_set()
    print(f"Found {len(containers)} container headers", file=sys.stderr)

    print("Building system symbol table...", file=sys.stderr)
    sys_symbols = build_system_symbol_table(clang_args)
    print(f"Indexed {len(sys_symbols)} system symbols", file=sys.stderr)

    if args.files:
        targets = [os.path.abspath(f) for f in args.files]
    else:
        targets = []
        for f in sorted(os.listdir(SQLITE_DIR)):
            if not f.endswith(".c"):
                continue
            path = os.path.join(SQLITE_DIR, f)
            with open(path) as fh:
                if '_All.h"' in fh.read():
                    targets.append(path)

    print(f"Processing {len(targets)} files", file=sys.stderr)

    report = {}
    all_unresolved = {}
    for i, cfile in enumerate(targets):
        result = analyze_file(cfile, clang_args, containers, sys_symbols)
        report[os.path.relpath(cfile, REPO_ROOT)] = result
        if result["errors"]:
            print(f"[{i+1}/{len(targets)}] PARSE ERRORS in {cfile}:", file=sys.stderr)
            for e in result["errors"]:
                print("   ", e, file=sys.stderr)
        if result["unresolved"]:
            for name, path in result["unresolved"]:
                all_unresolved.setdefault(name, set()).add(path)
        applied = apply_to_file(cfile, result, dry_run=args.dry_run)
        if not applied:
            print(f"[{i+1}/{len(targets)}] SKIP (no _All.h include found): {cfile}", file=sys.stderr)
        elif (i + 1) % 25 == 0 or (i + 1) == len(targets):
            print(f"[{i+1}/{len(targets)}] {os.path.basename(cfile)}", file=sys.stderr)

    if all_unresolved:
        print(f"\n{len(wall_unresolved)} unresolved system symbols (not found in symbol table):", file=sys.stderr)
        for name, paths in sorted(wall_unresolved.items()):
            print(f"   {name}: {sorted(paths)[0]}", file=sys.stderr)

    if args.report:
        with open(args.report, "w") as f:
            json.dump(report, f, indent=2)
        print(f"Report written to {args.report}", file=sys.stderr)


if __name__ == "__main__":
    main()
