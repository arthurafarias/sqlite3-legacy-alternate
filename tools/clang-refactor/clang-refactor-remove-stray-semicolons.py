#!/usr/bin/env python3
"""
Removes dead lone `;` statements left behind by macros (assert()/testcase()
and friends) that expand to nothing under the active build config.

Companion to clang-refactor-remove-void-noop.py, which handles the same
leftover but for the `((void)(0));` spelling; this handles the bare `;`
spelling, e.g.:

    if (cond) {

      ;
      doSomething();
    }

A line consisting of only `;` is a genuinely dead statement only if the
nearest preceding non-blank line already ends a statement or block (i.e.
ends with `;`, `{`, or `}`) -- meaning nothing was left dangling for this
semicolon to terminate. It is kept in two cases instead:

  - The nearest preceding non-blank line is a control-flow header ending
    in `)` with no trailing `;` or `{` (a real `for (...)\n  ;` or
    `while (...)\n  ;` empty-body loop) -- the semicolon is the loop body.
  - The nearest preceding non-blank line does NOT end a statement (e.g.
    `rc =`, or a call spread across lines like `isnan(x)` with its
    closing `;` pushed to its own line) -- this lone `;` is that
    statement's own terminator, not a dead extra one.

Collapses any resulting run of 3+ blank lines down to one afterward.

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

LONE_SEMI_RE = re.compile(r"^[ \t]*;[ \t]*$")
CTRL_HEADER_RE = re.compile(r"\b(if|for|while|else)\b.*\)[ \t]*$")
BLANK_RUN_RE = re.compile(r"\n{3,}")


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
    """Nearest preceding non-blank line's stripped text, or None."""
    j = i - 1
    while j >= 0:
        stripped = lines[j].strip()
        if stripped:
            return stripped
        j -= 1
    return None


def should_remove(lines, i):
    """A lone ';' at lines[i] is dead (removable) only if the nearest
    preceding non-blank line already closed a statement/block. If that
    line is a dangling control header, this ';' is a real loop body and
    must stay; if it doesn't end in ';'/'{'/'}' at all, this ';' is that
    line's own terminator (pushed onto its own line) and must stay too."""
    prev = nearest_preceding_content(lines, i)
    if prev is None:
        return False
    if CTRL_HEADER_RE.search(prev):
        return False
    return prev.endswith((";", "{", "}"))


def process_file(path, dry_run=False):
    with open(path, "r", encoding="utf-8", errors="surrogateescape") as f:
        original = f.read()

    lines = original.splitlines(keepends=True)
    kept = []
    removed = 0
    for i, line in enumerate(lines):
        if LONE_SEMI_RE.match(line) and should_remove(lines, i):
            removed += 1
            continue
        kept.append(line)

    if removed == 0:
        return 0

    new_content = "".join(kept)
    new_content = BLANK_RUN_RE.sub("\n\n", new_content)

    if not dry_run:
        with open(path, "w", encoding="utf-8", errors="surrogateescape") as f:
            f.write(new_content)

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
            print(f"{verb} {n} stray semicolon(s) from {rel}")

    print(f"\n{total} stray semicolon(s) removed across {changed_files} file(s)"
          + (" (dry run, no files written)" if args.dry_run else ""))


if __name__ == "__main__":
    main()
