#!/usr/bin/env python3
"""
Removes blank lines that sit inside a still-open (unterminated) statement,
e.g.:

    rc =

        isnan(x)

        ;

left behind by the preprocessed-amalgamation source of truth (see
docs/architecture.md) spreading a single statement or call across many
lines. clang-format will NOT join these on its own -- it treats a blank
line as an intentional break and never reflows tokens across one, even
mid-statement -- so this has to run as a text-level pass first.

A blank line is removed only when the nearest preceding non-blank line does
NOT already end a statement or block (i.e. does not end with `;`, `{`, or
`}`) -- meaning the statement is still open and this blank line is noise in
the middle of it, not a real gap between statements. Blank lines that
follow a properly closed statement/block are left alone; clang-format's own
MaxEmptyLinesToKeep already caps those to a single blank line.

Run this before clang-format, and after
clang-refactor-remove-stray-semicolons.py (so dead `;` lines are already
gone and don't get mistaken for statement terminators).

By default it operates on every tracked *.c/*.h/*.cpp/*.hpp/*.cc/*.hh file
in the git repo (via `git ls-files`). Pass explicit paths to restrict scope.
"""
import argparse
import os
import re
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SOURCE_EXTS = (".c", ".h", ".cpp", ".hpp", ".cc", ".hh")

BLANK_RE = re.compile(r"^[ \t]*$")


def repo_root():
    out = subprocess.run(
        ["git", "-C", SCRIPT_DIR, "rev-parse", "--show-toplevel"],
        capture_output=True, text=True, check=True,
    )
    return out.stdout.strip()


def tracked_source_files(root):
    out = subprocess.run(
        ["git", "-C", root, "ls-files"],
        capture_output=True, text=True, check=True,
    )
    return [os.path.join(root, rel) for rel in out.stdout.splitlines()
            if rel.endswith(SOURCE_EXTS)]


def nearest_preceding_content(lines, i):
    j = i - 1
    while j >= 0:
        stripped = lines[j].strip()
        if stripped:
            return stripped
        j -= 1
    return None


def process_file(path, dry_run=False):
    with open(path, "r", encoding="utf-8", errors="surrogateescape") as f:
        original = f.read()

    lines = original.splitlines(keepends=True)
    kept = []
    removed = 0
    for i, line in enumerate(lines):
        if BLANK_RE.match(line):
            prev = nearest_preceding_content(lines, i)
            if prev is not None and not prev.endswith((";", "{", "}")):
                removed += 1
                continue
        kept.append(line)

    if removed == 0:
        return 0

    if not dry_run:
        with open(path, "w", encoding="utf-8", errors="surrogateescape") as f:
            f.write("".join(kept))

    return removed


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("files", nargs="*",
                     help="Specific files to process (default: all tracked "
                          "source files in the repo)")
    ap.add_argument("--dry-run", action="store_true",
                     help="Report what would change without writing")
    args = ap.parse_args()

    root = repo_root()
    targets = [os.path.abspath(f) for f in args.files] if args.files \
        else tracked_source_files(root)

    total = 0
    changed_files = 0
    for path in targets:
        n = process_file(path, dry_run=args.dry_run)
        if n:
            changed_files += 1
            total += n
            rel = os.path.relpath(path, root)
            verb = "Would remove" if args.dry_run else "Removed"
            print(f"{verb} {n} mid-statement blank line(s) from {rel}")

    print(f"\n{total} mid-statement blank line(s) removed across "
          f"{changed_files} file(s)"
          + (" (dry run, no files written)" if args.dry_run else ""))


if __name__ == "__main__":
    main()
