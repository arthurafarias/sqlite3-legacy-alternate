#!/usr/bin/env python3
"""
Removes dead `((void)(0));` no-op statements from the codebase.

These are leftovers from macros (e.g. assert()/testcase()) that expand to
nothing under the active build config, but were left behind spread across
three lines by a previous refactor/preprocessing pass:



This script deletes each such block, then collapses any resulting run of
3+ blank lines down to a single blank line so removal doesn't leave gaps.

By default it operates on every tracked *.c/*.h/*.cpp/*.hpp/*.cc/*.hh file
in the git repo (via `git ls-files`, so build artifacts and untracked/vendor
trees are skipped). Pass explicit paths to restrict the scope.
"""
import argparse
import os
import re
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# Matches one `((void)(0))` block spread across 3 lines, including its
# leading indentation.
BLOCK_RE = re.compile(r"[ \t]*\(\(void\)\(0\)\)[ \t]*\n\n[ \t]*;[ \t]*\n")
BLANK_RUN_RE = re.compile(r"\n{3,}")

SOURCE_EXTS = (".c", ".h", ".cpp", ".hpp", ".cc", ".hh")


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
    files = []
    for rel in out.stdout.splitlines():
        if rel.endswith(SOURCE_EXTS):
            files.append(os.path.join(root, rel))
    return files


def process_file(path, dry_run=False):
    with open(path, "r", encoding="utf-8", errors="surrogateescape") as f:
        original = f.read()

    if "((void)(0))" not in original:
        return 0

    new_content, n = BLOCK_RE.subn("", original)
    new_content = BLANK_RUN_RE.sub("\n\n", new_content)

    remaining = new_content.count("((void)(0))")
    if remaining:
        print(f"WARNING: {path}: {remaining} occurrence(s) did not match "
              f"the expected 3-line pattern and were left in place",
              file=sys.stderr)

    if n == 0:
        return 0

    if not dry_run:
        with open(path, "w", encoding="utf-8", errors="surrogateescape") as f:
            f.write(new_content)

    return n


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

    total_blocks = 0
    changed_files = 0
    for path in targets:
        n = process_file(path, dry_run=args.dry_run)
        if n:
            changed_files += 1
            total_blocks += n
            rel = os.path.relpath(path, root)
            verb = "Would remove" if args.dry_run else "Removed"
            print(f"{verb} {n} block(s) from {rel}")

    print(f"\n{total_blocks} block(s) removed across {changed_files} file(s)"
          + (" (dry run, no files written)" if args.dry_run else ""))


if __name__ == "__main__":
    main()
