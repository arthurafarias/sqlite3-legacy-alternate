#!/usr/bin/env python3
"""
Categorize the function declarations in src/sqlite/_Uncategorized.h by how
many distinct compilation units (.c files) reference each function.

Every .c file in src/sqlite already has a matching, same-named .h file
(e.g. Hash.c / Hash.h). _Uncategorized.h is the odd one out: a single
catch-all header holding declarations for functions actually implemented
all over the tree, most of which are private helpers that leaked into it
during an earlier automated pass. This script figures out, for each
declared function, whether it is:

  single      - only ever referenced (called, or its address taken) from
                inside its own defining .c file. It should become a private
                `static` function, declared at the top of that .c file
                instead of in _Uncategorized.h.

  multi       - referenced from more than one .c file. It's a genuine
                cross-translation-unit API and should be declared in the
                defining .c file's own paired header (e.g. strHash's
                declaration belongs in Hash.h, since strHash is defined in
                Hash.c) instead of in the catch-all header.

  no_definition       - declared in _Uncategorized.h but no matching
                        definition was found under --src-dir. Needs manual
                        review; left untouched by the apply step.

  duplicate_definition - more than one .c file defines a function with this
                          name. Needs manual review; left untouched.

Output is a JSON report consumed by clang-refactor-apply-uncategorized.py.
"""
import argparse
import json
import os
import shlex
import subprocess
import sys
from collections import defaultdict

import clang.cindex


def resource_dir():
    return subprocess.check_output(["clang", "-print-resource-dir"]).decode().strip()


def real_path(p):
    return os.path.realpath(os.path.abspath(p))


def load_compile_args(compile_commands_path, src_dir, extra_flags):
    """Map real .c file path -> list of compiler args (minus compiler/-o/-c/file)."""
    with open(compile_commands_path) as f:
        entries = json.load(f)

    src_dir = real_path(src_dir)
    out = {}
    for e in entries:
        fpath = real_path(e["file"])
        if not fpath.startswith(src_dir + os.sep):
            continue
        if not fpath.endswith(".c"):
            continue
        args = shlex.split(e["command"])
        cleaned = []
        skip_next = False
        for a in args[1:]:  # drop compiler binary
            if skip_next:
                skip_next = False
                continue
            if a == "-o":
                skip_next = True
                continue
            if a == "-c":
                continue
            if real_path(a) == fpath if os.path.isabs(a) else a == e["file"]:
                continue
            cleaned.append(a)
        cleaned += extra_flags
        out[fpath] = cleaned
    return out


def extract_declared_functions(header_path, clang_args):
    """Return {name: {"text": <decl source text>, "line_start": int, "line_end": int}}."""
    index = clang.cindex.Index.create()
    tu = index.parse(header_path, args=["-x", "c"] + clang_args)
    errors = [d for d in tu.diagnostics if d.severity >= clang.cindex.Diagnostic.Error]
    if errors:
        print(f"WARNING: {len(errors)} error(s) parsing {header_path}:", file=sys.stderr)
        for d in errors:
            print(f"  {d}", file=sys.stderr)

    header_real = real_path(header_path)
    with open(header_path, "r", encoding="utf-8") as f:
        source = f.read()
    lines = source.split("\n")

    decls = {}
    for c in tu.cursor.get_children():
        if c.kind != clang.cindex.CursorKind.FUNCTION_DECL:
            continue
        if c.is_definition():
            continue
        if c.location.file is None or real_path(c.location.file.name) != header_real:
            continue
        start_line = c.extent.start.line  # 1-based
        end_line = c.extent.end.line
        text = "\n".join(lines[start_line - 1:end_line])
        decls[c.spelling] = {
            "text": text,
            "line_start": start_line,  # 1-based, inclusive
            "line_end": end_line,      # 1-based, inclusive
        }
    return decls


def scan_source_tree(compile_args_by_file, target_names):
    """Return (defs, usages).

    defs: name -> list of .c files that DEFINE this function.
    usages: name -> set of .c files that REFERENCE this function (call it, or
            take its address) via a DeclRefExpr physically written in that
            file. Does not include the defining file's own signature.
    """
    index = clang.cindex.Index.create()
    defs = defaultdict(list)
    usages = defaultdict(set)

    for fpath, args in sorted(compile_args_by_file.items()):
        tu = index.parse(fpath, args=args)
        errors = [d for d in tu.diagnostics if d.severity >= clang.cindex.Diagnostic.Error]
        if errors:
            print(f"WARNING: {len(errors)} error(s) parsing {fpath}:", file=sys.stderr)
            for d in errors[:5]:
                print(f"  {d}", file=sys.stderr)

        # Top-level definitions belonging to this exact file.
        for c in tu.cursor.get_children():
            if c.kind != clang.cindex.CursorKind.FUNCTION_DECL:
                continue
            if not c.is_definition():
                continue
            if c.location.file is None or real_path(c.location.file.name) != fpath:
                continue
            if c.spelling in target_names:
                defs[c.spelling].append(fpath)

        # Any reference (call, address-of, etc.) physically written in this file.
        def walk(cursor):
            if cursor.kind == clang.cindex.CursorKind.DECL_REF_EXPR:
                referenced = cursor.referenced
                if (
                    referenced is not None
                    and referenced.kind == clang.cindex.CursorKind.FUNCTION_DECL
                    and referenced.spelling in target_names
                    and cursor.location.file is not None
                    and real_path(cursor.location.file.name) == fpath
                ):
                    usages[referenced.spelling].add(fpath)
            for child in cursor.get_children():
                walk(child)

        walk(tu.cursor)

    return defs, usages


def categorize(decls, defs, usages):
    report = {}
    for name, decl in decls.items():
        def_files = sorted(set(defs.get(name, [])))
        use_files = usages.get(name, set())

        if len(def_files) == 0:
            category = "no_definition"
        elif len(def_files) > 1:
            category = "duplicate_definition"
        else:
            def_file = def_files[0]
            other_users = use_files - {def_file}
            category = "multi" if other_users else "single"

        report[name] = {
            "category": category,
            "decl_text": decl["text"],
            "header_line_start": decl["line_start"],
            "header_line_end": decl["line_end"],
            "definition_files": def_files,
            "using_files": sorted(use_files),
        }
    return report


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    here = os.path.dirname(os.path.abspath(__file__))
    default_src_dir = os.path.normpath(os.path.join(here, "..", "..", "src", "sqlite"))
    default_header = os.path.join(default_src_dir, "_Uncategorized.h")
    default_cc = os.path.normpath(os.path.join(here, "..", "..", "..", "..", "build", "compile_commands.json"))

    ap.add_argument("--header", default=default_header, help="Path to _Uncategorized.h")
    ap.add_argument("--src-dir", default=default_src_dir, help="Directory containing the .c/.h files to scan")
    ap.add_argument("--compile-commands", default=default_cc, help="Path to compile_commands.json")
    ap.add_argument("-o", "--output", default=os.path.join(here, "uncategorized-report.json"),
                     help="Where to write the JSON report")
    args = ap.parse_args()

    if not os.path.isfile(args.compile_commands):
        print(f"Error: compile_commands.json not found at '{args.compile_commands}'. "
              f"Configure the CMake build first (it must be generated with "
              f"CMAKE_EXPORT_COMPILE_COMMANDS=ON).", file=sys.stderr)
        return 1

    extra_flags = [f"-resource-dir={resource_dir()}"]
    compile_args_by_file = load_compile_args(args.compile_commands, args.src_dir, extra_flags)
    if not compile_args_by_file:
        print(f"Error: no .c files under '{args.src_dir}' found in '{args.compile_commands}'.", file=sys.stderr)
        return 1

    header_args = [f"-I{os.path.dirname(args.src_dir)}", "-std=gnu23"] + extra_flags
    decls = extract_declared_functions(args.header, header_args)
    if not decls:
        print(f"Error: no function declarations found in '{args.header}'.", file=sys.stderr)
        return 1

    defs, usages = scan_source_tree(compile_args_by_file, set(decls.keys()))
    report = categorize(decls, defs, usages)

    with open(args.output, "w") as f:
        json.dump(report, f, indent=2, sort_keys=True)

    counts = defaultdict(int)
    for v in report.values():
        counts[v["category"]] += 1

    print(f"Analysed {len(report)} function declaration(s) from '{args.header}':")
    for cat in ("single", "multi", "no_definition", "duplicate_definition"):
        print(f"  {cat:22s} {counts.get(cat, 0)}")
    print(f"Report written to '{args.output}'")

    return 0


if __name__ == "__main__":
    sys.exit(main())
