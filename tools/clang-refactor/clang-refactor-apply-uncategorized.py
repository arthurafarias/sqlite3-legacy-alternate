#!/usr/bin/env python3
"""
Apply the categorization produced by clang-refactor-categorize-uncategorized.py
to actually refactor src/sqlite/_Uncategorized.h.

For each function declared in _Uncategorized.h:

  single  - only ever referenced from inside its own defining .c file.
            The function is made `static`, and a `static` forward
            declaration is inserted at the top of that .c file (right
            after its preprocessor preamble, before the first top-level
            declaration/definition). The entry is removed from
            _Uncategorized.h.

  multi   - referenced from more than one .c file. In every case found in
            this codebase the function is *already* declared in its
            defining .c file's own paired header (e.g. strHash's
            declaration already lives in Hash.h, since strHash is defined
            in Hash.c) and every caller already includes that header. The
            entry in _Uncategorized.h is therefore pure dead duplication
            and is simply removed.

            If a future run finds a "multi" function that is NOT already
            declared in its own paired header, this script refuses to
            touch it (see --allow-undeclared-multi) since deciding where
            such a declaration belongs, and fixing up every caller's
            #include list, needs a human judgment call.

  no_definition / duplicate_definition - left untouched; printed as
            warnings for manual review.

Usage:
  clang-refactor-apply-uncategorized.py --report uncategorized-report.json
"""
import argparse
import json
import os
import re
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
    with open(compile_commands_path) as f:
        entries = json.load(f)

    src_dir = real_path(src_dir)
    out = {}
    for e in entries:
        fpath = real_path(e["file"])
        if not fpath.startswith(src_dir + os.sep) or not fpath.endswith(".c"):
            continue
        args = shlex.split(e["command"])
        cleaned = []
        skip_next = False
        for a in args[1:]:
            if skip_next:
                skip_next = False
                continue
            if a == "-o":
                skip_next = True
                continue
            if a == "-c":
                continue
            if (real_path(a) == fpath) if os.path.isabs(a) else (a == e["file"]):
                continue
            cleaned.append(a)
        cleaned += extra_flags
        out[fpath] = cleaned
    return out


def strip_removed_lines(header_path, removed_ranges):
    """Remove 1-based inclusive [start,end] line ranges from header_path."""
    with open(header_path, "r", encoding="utf-8") as f:
        lines = f.readlines()

    skip = [False] * (len(lines) + 1)
    for start, end in removed_ranges:
        for ln in range(start, end + 1):
            if 1 <= ln <= len(lines):
                skip[ln] = True

    kept = [line for i, line in enumerate(lines, start=1) if not skip[i]]

    # Collapse runs of 3+ blank lines left behind by the removals down to 1.
    collapsed = []
    blank_run = 0
    for line in kept:
        if line.strip() == "":
            blank_run += 1
            if blank_run > 1:
                continue
        else:
            blank_run = 0
        collapsed.append(line)

    with open(header_path, "w", encoding="utf-8") as f:
        f.writelines(collapsed)


def make_static(source, def_offset):
    """Insert 'static ' immediately before def_offset, unless already static."""
    lookback = source[max(0, def_offset - 200):def_offset]
    # Only care whether the declaration's own text already starts with static;
    # the definition text itself starts exactly at def_offset.
    if source[def_offset:def_offset + 7] == "static ":
        return source, False
    return source[:def_offset] + "static " + source[def_offset:], True


def signature_text(source, def_cursor):
    """Return 'static <ret> name(args)' text for a function DEFINITION cursor,
    derived from its own already-compiling source text (not from any other
    file), so any type spelling quirks (e.g. 'struct Foo' vs a typedef that
    may not be visible here) match what this file already uses."""
    body = None
    for child in def_cursor.get_children():
        if child.kind == clang.cindex.CursorKind.COMPOUND_STMT:
            body = child
    start = def_cursor.extent.start.offset
    end = body.extent.start.offset if body is not None else def_cursor.extent.end.offset
    text = source[start:end].strip()
    if body is None:
        # No compound-statement body found (shouldn't happen for a
        # definition); fall back to the full extent minus a trailing brace.
        text = text.rstrip("{").strip()
    return text


def process_single_file(fpath, names_and_decltext, compile_args, dry_run):
    """names_and_decltext: list of (name, decl_text) for functions defined in fpath.
    decl_text (from _Uncategorized.h) is only used to know which names to look
    for; the actual forward-declaration text is regenerated from each
    function's own definition so it matches types as spelled in this file."""
    index = clang.cindex.Index.create()
    tu = index.parse(fpath, args=compile_args)
    errors = [d for d in tu.diagnostics if d.severity >= clang.cindex.Diagnostic.Error]
    if errors:
        print(f"WARNING: {len(errors)} error(s) parsing {fpath}:", file=sys.stderr)
        for d in errors[:5]:
            print(f"  {d}", file=sys.stderr)

    top_level = list(tu.cursor.get_children())
    file_cursors = [c for c in top_level if c.location.file is not None and real_path(c.location.file.name) == fpath]
    if not file_cursors:
        print(f"WARNING: no top-level declarations found in {fpath}; skipping.", file=sys.stderr)
        return

    file_cursors.sort(key=lambda c: c.extent.start.offset)
    insertion_offset = file_cursors[0].extent.start.offset

    names = {n for n, _ in names_and_decltext}
    def_cursors = {}
    for c in file_cursors:
        if c.kind != clang.cindex.CursorKind.FUNCTION_DECL or not c.is_definition():
            continue
        if c.spelling in names and c.spelling not in def_cursors:
            def_cursors[c.spelling] = c

    missing = names - set(def_cursors.keys())
    if missing:
        print(f"WARNING: could not re-locate definition(s) {sorted(missing)} in {fpath}; leaving them untouched.",
              file=sys.stderr)

    with open(fpath, "r", encoding="utf-8") as f:
        source = f.read()

    # Capture each function's forward-declaration text from its own
    # definition BEFORE any 'static ' insertion shifts offsets.
    signatures = {name: signature_text(source, c) for name, c in def_cursors.items()}

    # Apply 'static ' insertions at each definition, from the end of the file
    # backwards so earlier offsets stay valid.
    made_static = []
    for name, c in sorted(def_cursors.items(), key=lambda kv: -kv[1].extent.start.offset):
        source, changed = make_static(source, c.extent.start.offset)
        if changed:
            made_static.append(name)

    # Build the forward-declaration block (only for functions we actually
    # located above).
    decl_lines = []
    for name in sorted(def_cursors.keys()):
        text = signatures[name]
        if not text.startswith("static"):
            text = "static " + text
        decl_lines.append(text + ";")

    if decl_lines:
        block = ("/* Private helpers, formerly declared in _Uncategorized.h. */\n"
                  + "\n".join(decl_lines) + "\n\n")
        source = source[:insertion_offset] + block + source[insertion_offset:]

    if not dry_run:
        with open(fpath, "w", encoding="utf-8") as f:
            f.write(source)

    print(f"{os.path.relpath(fpath)}: made {len(made_static)} function(s) static, "
          f"inserted {len(decl_lines)} private declaration(s).")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    here = os.path.dirname(os.path.abspath(__file__))
    default_src_dir = os.path.normpath(os.path.join(here, "..", "..", "src", "sqlite"))
    default_header = os.path.join(default_src_dir, "_Uncategorized.h")
    default_cc = os.path.normpath(os.path.join(here, "..", "..", "..", "..", "build", "compile_commands.json"))
    default_report = os.path.join(here, "uncategorized-report.json")

    ap.add_argument("--report", default=default_report, help="JSON report from the categorize script")
    ap.add_argument("--header", default=default_header, help="Path to _Uncategorized.h")
    ap.add_argument("--src-dir", default=default_src_dir, help="Directory containing the .c/.h files")
    ap.add_argument("--compile-commands", default=default_cc, help="Path to compile_commands.json")
    ap.add_argument("--allow-undeclared-multi", action="store_true",
                     help="Also strip 'multi' entries that are NOT already declared in their own "
                          "paired header (unsafe: does not fix up caller #includes). Off by default.")
    ap.add_argument("--dry-run", action="store_true", help="Report what would change without writing files")
    args = ap.parse_args()

    with open(args.report) as f:
        report = json.load(f)

    extra_flags = [f"-resource-dir={resource_dir()}"]
    compile_args_by_file = load_compile_args(args.compile_commands, args.src_dir, extra_flags)

    removed_ranges = []
    single_by_file = defaultdict(list)
    multi_removed = 0
    multi_skipped = 0

    for name, v in report.items():
        cat = v["category"]
        line_range = (v["header_line_start"], v["header_line_end"])

        if cat == "no_definition":
            print(f"SKIP (no definition found anywhere): {name}", file=sys.stderr)
            continue
        if cat == "duplicate_definition":
            print(f"SKIP (defined in multiple files: {v['definition_files']}): {name}", file=sys.stderr)
            continue

        if cat == "single":
            def_file = v["definition_files"][0]
            single_by_file[def_file].append((name, v["decl_text"]))
            removed_ranges.append(line_range)
            continue

        if cat == "multi":
            def_file = v["definition_files"][0]
            hpath = def_file[:-2] + ".h"
            already_declared = os.path.exists(hpath) and re.search(
                r"\b" + re.escape(name) + r"\s*\(", open(hpath).read()
            )
            if already_declared or args.allow_undeclared_multi:
                removed_ranges.append(line_range)
                multi_removed += 1
            else:
                print(f"SKIP (multi-use but not declared in its own header {hpath}; "
                      f"needs manual placement): {name}", file=sys.stderr)
                multi_skipped += 1
            continue

    for def_file, names_and_decltext in sorted(single_by_file.items()):
        compile_args = compile_args_by_file.get(def_file)
        if compile_args is None:
            print(f"WARNING: no compile args found for {def_file}; skipping its {len(names_and_decltext)} "
                  f"function(s).", file=sys.stderr)
            continue
        process_single_file(def_file, names_and_decltext, compile_args, args.dry_run)

    if not args.dry_run:
        strip_removed_lines(args.header, removed_ranges)

    print()
    print(f"single-use functions made static:      {sum(len(v) for v in single_by_file.values())}")
    print(f"multi-use duplicate decls removed:      {multi_removed}")
    print(f"multi-use decls left for manual review: {multi_skipped}")
    print(f"_Uncategorized.h entries removed total: {len(removed_ranges)}"
          + (" (dry run, not written)" if args.dry_run else f" -> {args.header}"))

    return 0


if __name__ == "__main__":
    sys.exit(main())
