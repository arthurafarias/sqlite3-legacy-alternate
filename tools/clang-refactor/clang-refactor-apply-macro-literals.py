#!/usr/bin/env python3
"""
Apply the candidates in tools/clang-refactor/macro-literals-report.json
(written by clang-refactor-classify-macro-literals.py): replace a bare
integer literal inside a function body with the already-existing enum
constant name the real SQLite amalgamation used at that same call site.

Per file: candidates are deduplicated and spliced in descending source-
offset order (no dependency on clang-apply-replacements, matching
clang-refactor-struct-to-header.py and clang-refactor-prefix-types.py), then
any header a candidate needs that the file doesn't already #include is added
next to the file's other `sqlite/*.h` includes. clang-format runs over every
touched file afterward.

Read-only by default; pass --apply to write.
"""
import argparse
import json
import os
import re
import subprocess
import sys
from collections import defaultdict

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, "..", ".."))
DEFAULT_REPORT = os.path.join(SCRIPT_DIR, "macro-literals-report.json")

INCLUDE_RE = re.compile(r'^#include\s+"(sqlite/[^"]+\.h)"\s*$', re.MULTILINE)


def ensure_include(text, header):
    """Return text with `#include "header"` present, inserted next to any
    existing sqlite/*.h includes (after the last one, preserving whatever
    order the file already uses)."""
    if re.search(r'^#include\s+"' + re.escape(header) + r'"\s*$', text, re.MULTILINE):
        return text

    matches = list(INCLUDE_RE.finditer(text))
    line = f'#include "{header}"\n'
    if matches:
        insert_at = matches[-1].end()
        if not text[insert_at - 1:insert_at] == "\n":
            insert_at += 1
        return text[:insert_at] + line + text[insert_at:]

    # No sqlite/*.h include exists yet in this file; drop it after any
    # leading #pragma once / extern "C" guard, else at the very top.
    pragma = re.search(r'^#pragma once\s*$', text, re.MULTILINE)
    insert_at = pragma.end() + 1 if pragma else 0
    return text[:insert_at] + line + text[insert_at:]


def apply_to_file(fpath, candidates, dry_run):
    with open(fpath, "r", encoding="utf-8") as f:
        text = f.read()

    # Sanity-check every candidate against the live file before touching
    # anything -- offsets drift if the file changed since classify ran.
    for c in sorted(candidates, key=lambda c: c["offset_start"]):
        actual = text[c["offset_start"]:c["offset_end"]]
        if actual != c["old_literal"]:
            print(f"  SKIP stale offset in {fpath}: expected {c['old_literal']!r} at "
                  f"{c['offset_start']}, found {actual!r}", file=sys.stderr)
            return False, 0

    applied = 0
    for c in sorted(candidates, key=lambda c: -c["offset_start"]):
        text = text[:c["offset_start"]] + c["constant"] + text[c["offset_end"]:]
        applied += 1

    headers_needed = sorted({c["header"] for c in candidates})
    for header in headers_needed:
        text = ensure_include(text, header)

    if not dry_run:
        with open(fpath, "w", encoding="utf-8") as f:
            f.write(text)

    return True, applied


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--report", default=DEFAULT_REPORT)
    ap.add_argument("--apply", action="store_true", help="Write changes (default: dry run)")
    ap.add_argument("--no-format", action="store_true", help="Skip clang-format after applying")
    args = ap.parse_args()

    if not os.path.isfile(args.report):
        print(f"Error: report not found at '{args.report}'. Run "
              f"clang-refactor-classify-macro-literals.py first.", file=sys.stderr)
        return 1

    with open(args.report) as f:
        report = json.load(f)

    by_file = defaultdict(list)
    for c in report["candidates"]:
        by_file[c["file"]].append(c)

    total_applied = 0
    touched_files = []
    for relpath, candidates in sorted(by_file.items()):
        fpath = os.path.join(REPO_ROOT, relpath)
        ok, applied = apply_to_file(fpath, candidates, dry_run=not args.apply)
        if ok:
            total_applied += applied
            touched_files.append(fpath)
            print(f"  {'would apply' if not args.apply else 'applied'} {applied:3d} site(s) in {relpath}")

    print(f"\n{total_applied} substitution(s) across {len(touched_files)} file(s)"
          f"{' (dry run -- pass --apply to write)' if not args.apply else ''}")

    if args.apply and touched_files and not args.no_format:
        fmt = subprocess.run(["clang-format", "-i"] + touched_files)
        if fmt.returncode != 0:
            print("WARNING: clang-format failed on one or more files", file=sys.stderr)

    return 0


if __name__ == "__main__":
    sys.exit(main())
