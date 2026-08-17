#!/usr/bin/env python3
"""
Renames raw compiler builtins back to the standard library name they stand
in for, e.g. `__builtin_offsetof(T, m)` -> `offsetof(T, m)`.

These leaked in because the source of truth for this tree is the
*preprocessed* amalgamation (see docs/architecture.md): <stddef.h>'s
`offsetof`, <math.h>'s `isnan`, and glibc's `INFINITY` are themselves
macros that expand to a compiler builtin, and preprocessing baked in the
expanded (and, for parenthesized macros, oddly line-broken) form instead of
the readable standard name.

Handles, per occurrence, via balanced-paren extraction (safe with nested
parens in the arguments):
  __builtin_offsetof(A, B)  -> offsetof(A, B)
  __builtin_isnan(X)        -> isnan(X)
  __builtin_inff()          -> INFINITY

Also collapses each rewritten call onto one line (the preprocessed source
often spreads a single call across many blank-line-separated lines) and
adds the required standard header (<stddef.h> / <math.h>) to any file that
gains a use of one of these names without already including it.

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

# name -> (standard replacement name, is_function_like, required header)
BUILTINS = {
    "__builtin_offsetof": ("offsetof", True, "stddef.h"),
    "__builtin_isnan": ("isnan", True, "math.h"),
    "__builtin_inff": ("INFINITY", False, "math.h"),
}


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


def find_matching_paren(text, open_paren_idx):
    """Given the index of a '(', return the index of its matching ')'."""
    depth = 0
    i = open_paren_idx
    while i < len(text):
        if text[i] == "(":
            depth += 1
        elif text[i] == ")":
            depth -= 1
            if depth == 0:
                return i
        i += 1
    raise ValueError("unbalanced parens")


def rewrite_builtins(text):
    changed = 0
    used_headers = set()
    for name, (repl, is_func, header) in BUILTINS.items():
        out = []
        pos = 0
        while True:
            idx = text.find(name, pos)
            if idx == -1:
                out.append(text[pos:])
                break
            # require the name isn't a prefix of a longer identifier
            after = idx + len(name)
            if after < len(text) and (text[after].isalnum() or text[after] == "_"):
                out.append(text[pos:after])
                pos = after
                continue
            paren_idx = text.find("(", after)
            if paren_idx == -1 or text[after:paren_idx].strip() != "":
                out.append(text[pos:after])
                pos = after
                continue
            close_idx = find_matching_paren(text, paren_idx)
            inner = text[paren_idx + 1:close_idx]
            out.append(text[pos:idx])
            if is_func:
                args = [re.sub(r"\s+", " ", a).strip() for a in split_top_level(inner)]
                out.append(f"{repl}({', '.join(args)})")
            else:
                out.append(repl)
            pos = close_idx + 1
            changed += 1
            used_headers.add(header)
        text = "".join(out)
    return text, changed, used_headers


def split_top_level(s):
    """Split a comma-separated argument list, ignoring commas inside parens."""
    parts = []
    depth = 0
    cur = []
    for ch in s:
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        if ch == "," and depth == 0:
            parts.append("".join(cur))
            cur = []
        else:
            cur.append(ch)
    parts.append("".join(cur))
    return parts


INCLUDE_RE = re.compile(r'^#include\s*[<"]([^>"]+)[>"]', re.MULTILINE)
LAST_SYSTEM_INCLUDE_RE = re.compile(r'^#include\s*<[^>]+>[ \t]*\n', re.MULTILINE)


def ensure_headers(text, headers):
    existing = set(INCLUDE_RE.findall(text))
    for header in sorted(headers):
        if header in existing:
            continue
        matches = list(LAST_SYSTEM_INCLUDE_RE.finditer(text))
        line = f"#include <{header}>\n"
        if matches:
            insert_at = matches[-1].end()
            text = text[:insert_at] + line + text[insert_at:]
        else:
            text = line + text
    return text


def process_file(path, dry_run=False):
    with open(path, "r", encoding="utf-8", errors="surrogateescape") as f:
        original = f.read()

    if not any(name in original for name in BUILTINS):
        return 0

    new_content, n, headers = rewrite_builtins(original)
    if n == 0:
        return 0

    new_content = ensure_headers(new_content, headers)

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

    total = 0
    changed_files = 0
    for path in targets:
        n = process_file(path, dry_run=args.dry_run)
        if n:
            changed_files += 1
            total += n
            rel = os.path.relpath(path, root)
            verb = "Would rename" if args.dry_run else "Renamed"
            print(f"{verb} {n} builtin call(s) in {rel}")

    print(f"\n{total} call(s) renamed across {changed_files} file(s)"
          + (" (dry run, no files written)" if args.dry_run else ""))


if __name__ == "__main__":
    main()
