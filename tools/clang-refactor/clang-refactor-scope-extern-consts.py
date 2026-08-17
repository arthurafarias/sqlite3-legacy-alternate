#!/usr/bin/env python3
"""
Push global-scope `extern const` declarations out of a shared/dumping-ground
header ("_Uncategorized.h" style) into the compilation unit(s) that actually
need them.

For every top-level `extern const ...;` declared in --header, this looks at
every .c file that references the symbol and asks libclang where that symbol
*actually* resolves from (its own definition, another header already in
scope, or nothing). Three outcomes:

  redundant   -- every using file already resolves the symbol on its own
                 (self-contained definition, or an existing dedicated header).
                 The declaration in --header is dead weight -> delete it.

  single-use  -- exactly one .c file needs a declaration for the symbol and
                 has none available -> move the `extern const ...;` line into
                 that file (top, after its #includes) and delete it from
                 --header.

  shared      -- two or more .c files need a declaration and none is
                 available -> create a dedicated "<Name>.h" using this
                 codebase's standard header template, point every user file
                 (and the definition's provider file, if found) at it, and
                 delete the entry from --header. With --extract-definitions,
                 if exactly one non-static provider of the symbol exists, its
                 initializer is additionally moved into a new dedicated
                 "<Name>.c" translation unit.

Run with --dry-run (default) first; nothing is written until --apply.
clang-format (if available) is run over every touched file afterward.
"""
import argparse
import glob
import os
import re
import subprocess
import sys

import clang.cindex as cc


def real_path(p):
    return os.path.realpath(os.path.abspath(p))


def system_include_args():
    """libclang's bundled headers often lack a working stddef.h etc.; borrow
    the real system clang's search path so parses don't abort early."""
    try:
        resource_dir = subprocess.run(
            ["clang", "-print-resource-dir"], capture_output=True, text=True, check=True
        ).stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        return []
    args = []
    if resource_dir:
        args += ["-isystem", os.path.join(resource_dir, "include")]
    for d in ("/usr/local/include", "/usr/include"):
        if os.path.isdir(d):
            args += ["-isystem", d]
    return args


# --------------------------------------------------------------------------
# Step 1: enumerate top-level `extern const` VAR_DECLs in the source header.
# --------------------------------------------------------------------------

class Target:
    def __init__(self, name, text, start, end, line_no):
        self.name = name
        self.text = text          # verbatim declaration text, e.g. "extern const char foo[8];"
        self.start = start        # byte offset in header source
        self.end = end
        self.line_no = line_no
        # filled in during analysis
        self.self_contained_files = set()   # .c files that resolve it themselves
        self.covered_by = {}                # .c file -> other header already declaring it
        self.orphan_files = set()           # .c files with no resolution at all
        self.providers = set()              # (file, is_static) definitions found anywhere


def parse_header_targets(header_path, index, extra_args):
    src = open(header_path, encoding="utf-8").read()
    tu = index.parse(header_path, args=extra_args + ["-x", "c"])
    fatal = [d for d in tu.diagnostics if d.severity >= cc.Diagnostic.Error]
    if fatal:
        print(f"warning: {len(fatal)} error(s) parsing {header_path}:", file=sys.stderr)
        for d in fatal:
            print(f"  {d}", file=sys.stderr)

    hp = real_path(header_path)
    targets = []
    for c in tu.cursor.get_children():
        if c.kind != cc.CursorKind.VAR_DECL:
            continue
        if c.location.file is None or real_path(c.location.file.name) != hp:
            continue
        if c.storage_class != cc.StorageClass.EXTERN:
            continue
        if not c.type.is_const_qualified() and "const" not in c.type.spelling:
            # array-of-const / pointer-to-const types don't always report
            # is_const_qualified() on the outer VAR_DECL, so also sanity
            # check the raw text below.
            pass
        start, end = c.extent.start.offset, c.extent.end.offset
        while end < len(src) and src[end] in " \t\r":
            end += 1
        if end < len(src) and src[end] == ";":
            end += 1
        text = src[start:end].strip()
        if not text.startswith("extern") or "const" not in text.split("=")[0]:
            continue
        targets.append(Target(c.spelling, text, start, end, c.location.line))
    return src, targets


# --------------------------------------------------------------------------
# Step 2: for every .c file that mentions a target name, ask libclang where
# the symbol actually resolves to.
# --------------------------------------------------------------------------

def candidate_c_files(src_dir, names):
    """Pre-filter with grep -lw so we only libclang-parse files that matter."""
    all_c = sorted(glob.glob(os.path.join(src_dir, "*.c")))
    if not names:
        return []
    result = subprocess.run(
        ["grep", "-lwE", "|".join(re.escape(n) for n in names)] + all_c,
        capture_output=True, text=True,
    )
    return sorted(result.stdout.split())


def compile_args_for(comp_db, c_file):
    fallback = ["-I", os.path.dirname(os.path.dirname(real_path(c_file))), "-std=gnu23"] + system_include_args()
    if comp_db is None:
        return fallback
    cmds = comp_db.getCompileCommands(c_file)
    if not cmds:
        return fallback
    cmd = cmds[0]
    args = list(cmd.arguments)[1:]  # drop compiler executable
    out = list(system_include_args())
    skip_next = False
    for a in args:
        if skip_next:
            skip_next = False
            continue
        if a in ("-c", "-o"):
            skip_next = a == "-o"
            continue
        if a.endswith(".c"):
            continue
        out.append(a)
    return out


def analyze_file(c_file, targets_by_name, index, comp_db):
    args = compile_args_for(comp_db, c_file)
    tu = index.parse(c_file, args=args)
    fatal = [d for d in tu.diagnostics if d.severity >= cc.Diagnostic.Error]
    if fatal:
        print(f"warning: {c_file} has {len(fatal)} pre-existing error(s); "
              f"results for this file may be unreliable", file=sys.stderr)

    fp = real_path(c_file)
    names = set(targets_by_name)

    def walk(cursor):
        loc = cursor.location.file
        if loc is not None and real_path(loc.name) != fp:
            return  # prune: don't descend into other files (headers, etc.)

        if cursor.kind == cc.CursorKind.VAR_DECL and cursor.spelling in names:
            if cursor.is_definition():
                t = targets_by_name[cursor.spelling]
                t.providers.add((c_file, cursor.storage_class == cc.StorageClass.STATIC))

        if cursor.kind == cc.CursorKind.DECL_REF_EXPR and cursor.spelling in names:
            t = targets_by_name[cursor.spelling]
            ref = cursor.referenced
            if ref is None or ref.location.file is None:
                t.orphan_files.add(c_file)
            else:
                rp = real_path(ref.location.file.name)
                if rp == fp:
                    t.self_contained_files.add(c_file)
                elif rp == real_path(targets_by_name[cursor.spelling]._header_path):
                    # currently resolves through the very header we're about
                    # to prune declarations from -- treat as orphan so a
                    # replacement gets put in place before we delete it.
                    t.orphan_files.add(c_file)
                else:
                    t.covered_by[c_file] = ref.location.file.name

        for child in cursor.get_children():
            walk(child)

    walk(tu.cursor)


# --------------------------------------------------------------------------
# Step 3: classify + apply.
# --------------------------------------------------------------------------

HEADER_TEMPLATE = """#pragma once

#ifdef __cplusplus
extern "C" {{
#endif

{decls}

#ifdef __cplusplus
}}
#endif
"""


def insert_after_includes(src, line):
    lines = src.splitlines(keepends=True)
    last_include = -1
    for i, l in enumerate(lines):
        if l.lstrip().startswith("#include"):
            last_include = i
    insert_at = last_include + 1 if last_include >= 0 else 0
    lines.insert(insert_at, line.rstrip("\n") + "\n")
    return "".join(lines)


def add_include_if_missing(src, include_line):
    if include_line in src:
        return src
    return insert_after_includes(src, include_line)


def classify(target):
    users = target.self_contained_files | set(target.covered_by) | target.orphan_files
    if not target.orphan_files:
        return "redundant", users
    if len(target.orphan_files) == 1:
        return "single-use", users
    return "shared", users


def run_clang_format(paths, binary):
    paths = [p for p in paths if os.path.exists(p)]
    if not paths or not binary:
        return
    subprocess.run([binary, "-i"] + paths)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--header", required=True, help="Header to scope extern consts out of (e.g. _Uncategorized.h)")
    ap.add_argument("--src-dir", required=True, help="Directory containing the .c/.h files")
    ap.add_argument("--compile-db", help="Directory containing compile_commands.json (recommended)")
    ap.add_argument("--include-prefix", default="sqlite", help="#include \"<prefix>/Name.h\" prefix used by this codebase")
    ap.add_argument("--extract-definitions", action="store_true",
                     help="For 'shared' symbols, also relocate the initializer into a new dedicated .c file "
                          "when exactly one non-static provider exists")
    ap.add_argument("--apply", action="store_true", help="Write changes (default is dry-run/report only)")
    ap.add_argument("--clang-format", default="clang-format", help="clang-format binary to run over touched files")
    args = ap.parse_args()

    header_path = real_path(args.header)
    src_dir = real_path(args.src_dir)

    index = cc.Index.create()
    base_args = ["-I", os.path.dirname(src_dir), "-std=gnu23"] + system_include_args()

    header_src, targets = parse_header_targets(header_path, index, base_args)
    if not targets:
        print("No top-level `extern const` declarations found.")
        return 0

    for t in targets:
        t._header_path = header_path
    by_name = {t.name: t for t in targets}

    comp_db = None
    if args.compile_db:
        try:
            comp_db = cc.CompilationDatabase.fromDirectory(args.compile_db)
        except cc.CompilationDatabaseError:
            print(f"warning: could not load compile_commands.json from {args.compile_db}", file=sys.stderr)

    files = candidate_c_files(src_dir, list(by_name))
    print(f"Scanning {len(files)} candidate .c file(s) for {len(targets)} symbol(s)...")
    for f in files:
        analyze_file(f, by_name, index, comp_db)

    touched_files = {}  # path -> new source (str) staged for writing

    def get_src(path):
        if path not in touched_files:
            touched_files[path] = open(path, encoding="utf-8").read()
        return touched_files[path]

    def set_src(path, new_src):
        touched_files[path] = new_src

    header_deletions = []  # (start, end) spans to remove from header_src, in order

    n_redundant = n_single = n_shared = 0

    for t in targets:
        kind, users = classify(t)
        provider_files = {f for f, _static in t.providers}
        non_static_providers = {f for f, static in t.providers if not static}

        if kind == "redundant":
            n_redundant += 1
            print(f"[redundant]  {t.name:28} already resolved in every user "
                  f"({len(users)} file(s)) -> delete from header")
            header_deletions.append((t.start, t.end))

        elif kind == "single-use":
            n_single += 1
            (dest,) = t.orphan_files
            print(f"[single-use] {t.name:28} -> move into {os.path.relpath(dest, src_dir)}")
            header_deletions.append((t.start, t.end))
            src = get_src(dest)
            set_src(dest, insert_after_includes(src, t.text))

        else:
            n_shared += 1
            dest_files = sorted(t.orphan_files)
            header_name = f"{t.name}.h"
            header_out = os.path.join(src_dir, header_name)
            print(f"[shared]     {t.name:28} used in {len(dest_files)} files with no shared header "
                  f"-> create {header_name}, point {', '.join(os.path.basename(d) for d in dest_files)} at it")
            header_deletions.append((t.start, t.end))

            decl_block = "  " + t.text
            new_header_text = HEADER_TEMPLATE.format(decls=decl_block)
            include_line = f'#include "{args.include_prefix}/{header_name}"'

            touched_files[header_out] = new_header_text
            for d in dest_files:
                set_src(d, add_include_if_missing(get_src(d), include_line))

            if args.extract_definitions and len(non_static_providers) == 1:
                (provider,) = non_static_providers
                psrc = get_src(provider)
                pindex = cc.Index.create()
                ptu = pindex.parse(provider, args=compile_args_for(comp_db, provider))
                pfp = real_path(provider)
                def_cursor = None
                for c in ptu.cursor.get_children():
                    if (c.kind == cc.CursorKind.VAR_DECL and c.spelling == t.name
                            and c.is_definition() and c.location.file
                            and real_path(c.location.file.name) == pfp):
                        def_cursor = c
                        break
                if def_cursor is not None:
                    dstart, dend = def_cursor.extent.start.offset, def_cursor.extent.end.offset
                    while dend < len(psrc) and psrc[dend] in " \t\r":
                        dend += 1
                    if dend < len(psrc) and psrc[dend] == ";":
                        dend += 1
                    def_text = psrc[dstart:dend].strip()
                    new_c_path = os.path.join(src_dir, f"{t.name}.c")
                    touched_files[new_c_path] = f'#include "{args.include_prefix}/{header_name}"\n\n{def_text}\n'
                    psrc = get_src(provider)  # re-fetch in case add_include_if_missing already touched it
                    psrc = psrc[:dstart] + psrc[dend:]
                    psrc = add_include_if_missing(psrc, include_line)
                    set_src(provider, psrc)
                    print(f"             also extracted definition -> {t.name}.c")
                else:
                    print(f"             (could not locate definition text for {t.name} in {provider}; "
                          f"leaving definition in place)")

            if provider_files - non_static_providers - set(dest_files):
                print(f"             note: {t.name} also has other/static definitions in "
                      f"{sorted(provider_files - non_static_providers)}; review manually")

    header_deletions.sort()
    new_header_src = header_src
    for start, end in sorted(header_deletions, reverse=True):
        new_header_src = new_header_src[:start] + new_header_src[end:]
    touched_files[header_path] = new_header_src

    print(f"\nSummary: {n_redundant} redundant, {n_single} single-use, {n_shared} shared "
          f"({len(targets)} total)")

    if not args.apply:
        print("\nDry run only -- pass --apply to write these changes.")
        return 0

    for path, content in touched_files.items():
        with open(path, "w", encoding="utf-8") as f:
            f.write(content)
        print(f"wrote {path}")

    run_clang_format(list(touched_files), args.clang_format)
    print("\nDone. Rebuild the project to verify.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
