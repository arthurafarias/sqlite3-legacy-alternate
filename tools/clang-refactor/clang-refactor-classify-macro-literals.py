#!/usr/bin/env python3
"""
Classify integer-literal call sites in src/sqlite/*.c that correspond to a
`#define` constant in the original (unprocessed) SQLite amalgamation and to
an enum constant that already exists somewhere in src/sqlite/*.h.

Why this exists: the seed pass (clang-refactor-functions-to-containers.py)
split src/sqlite/ out of a *preprocessed* amalgamation dump, so every
`#define`-based constant used inside a function body was already expanded to
its raw numeric value before that split ever happened -- the symbolic name
was gone from the token stream by the time this tree existed. A later,
separate pass ("removing preprocessor trash") re-created those constant
groups as proper C `enum`s under src/sqlite/*.h, using the exact original
macro names as enumerator names -- but that pass only touched declarations,
not the numeric literals already sitting in function bodies. This script
finds the call sites where a body still holds the bare number and an enum
constant with a matching name and value already exists to replace it with.

Method: for every function defined in the real, unmodified amalgamation
(--original-src, a normal --original-src build of sqlite3.c that has never
been preprocessed -- macro invocations still appear as plain identifier
tokens to a lexer, since libclang tokenizes source *spelling*, not macro
expansions), and the same-named function as it exists today in this tree,
align the two token streams with a sequence diff. Wherever the diff finds a
single-token substitution of an IDENTIFIER (in the original) for a numeric
LITERAL (today) -- and that identifier names a known enum constant whose
value equals the literal -- that is a high-confidence restoration site.
Anything less exact (multi-token replacements, unresolved names, functions
that don't line up 1:1 between the two trees at all) is left alone and
counted, not guessed at.

Two-phase, matching every other pass under tools/clang-refactor/: this
script only classifies and writes a JSON report; clang-refactor-apply-
macro-literals.py performs the substitutions.
"""
import argparse
import gc
import json
import os
import re
import subprocess
import sys
from collections import defaultdict
from difflib import SequenceMatcher

import clang.cindex

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, "..", ".."))
SRC_DIR = os.path.join(REPO_ROOT, "src")
SQLITE_DIR = os.path.join(SRC_DIR, "sqlite")

DEFAULT_ORIGINAL_SRC = "/home/arthur/Downloads/sqlite-src-3530400/sqlite3.c"
DEFAULT_COMPILE_COMMANDS = os.path.normpath(os.path.join(REPO_ROOT, "build", "compile_commands.json"))

INT_LITERAL_RE = re.compile(r"^0[xX][0-9a-fA-F]+[uUlL]*$|^0[bB][01]+[uUlL]*$|^\d+[uUlL]*$")


def find_libclang():
    for cand in ["/usr/lib/libclang.so"]:
        if os.path.exists(cand):
            return cand
    return None


def real_path(p):
    return os.path.realpath(os.path.abspath(p))


def clang_system_includes():
    out = subprocess.run(["clang", "-E", "-x", "c", "-v", "/dev/null"], capture_output=True, text=True)
    lines = out.stderr.splitlines()
    paths, capture = [], False
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


def parse_int_literal(spelling):
    s = spelling.rstrip("uUlL")
    try:
        if s.lower().startswith("0x"):
            return int(s, 16)
        if s.lower().startswith("0b"):
            return int(s, 2)
        if s.startswith("0") and len(s) > 1 and s.isdigit():
            return int(s, 8)
        return int(s, 10)
    except ValueError:
        return None


def build_known_constants(sysinc):
    """Scan every src/sqlite/*.h for enum constants. Return name -> {value, header}."""
    index = clang.cindex.Index.create()
    args = ["-x", "c", "-std=c23", f"-I{SRC_DIR}"] + sum([["-isystem", p] for p in sysinc], [])

    constants = {}
    headers = sorted(f for f in os.listdir(SQLITE_DIR) if f.endswith(".h"))
    for hname in headers:
        hpath = os.path.join(SQLITE_DIR, hname)
        tu = index.parse(hpath, args=args)
        errs = [d for d in tu.diagnostics if d.severity >= clang.cindex.Diagnostic.Error]
        if errs:
            print(f"WARNING: {len(errs)} error(s) parsing {hpath}:", file=sys.stderr)
            for d in errs[:3]:
                print(f"  {d}", file=sys.stderr)
            continue

        hreal = real_path(hpath)

        def walk(cursor):
            for child in cursor.get_children():
                if (
                    child.kind == clang.cindex.CursorKind.ENUM_CONSTANT_DECL
                    and child.location.file is not None
                    and real_path(child.location.file.name) == hreal
                ):
                    name = child.spelling
                    if name not in constants:
                        constants[name] = {"value": child.enum_value, "header": f"sqlite/{hname}"}
                walk(child)

        walk(tu.cursor)

    return constants


def tokenize_function(cursor):
    """Return list of (kind_name, spelling, start_offset, end_offset).

    clang_tokenize occasionally ignores its own extent argument for a
    handful of cursors in the real amalgamation (observed on
    sqlite3PagerCommitPhaseOne: a 219-line, ~9KB extent came back with
    228,659 tokens -- effectively "rest of the 9.5MB file") -- a libclang
    lexer quirk, not a real function body. Tokens whose own extent falls
    outside the cursor's extent are dropped rather than trusted.
    """
    lo, hi = cursor.extent.start.offset, cursor.extent.end.offset
    out = []
    for tok in cursor.get_tokens():
        tstart, tend = tok.extent.start.offset, tok.extent.end.offset
        if tstart < lo or tend > hi:
            continue
        out.append((tok.kind.name, tok.spelling, tstart, tend))
    return out


def collect_original_functions(original_src, sysinc):
    index = clang.cindex.Index.create()
    args = ["-std=c17", "-w"] + sum([["-isystem", p] for p in sysinc], [])
    tu = index.parse(original_src, args=args)
    errs = [d for d in tu.diagnostics if d.severity >= clang.cindex.Diagnostic.Error]
    if errs:
        print(f"WARNING: {len(errs)} error(s) parsing original source {original_src}:", file=sys.stderr)
        for d in errs[:10]:
            print(f"  {d}", file=sys.stderr)

    oreal = real_path(original_src)
    funcs = {}
    dupes = set()
    i = 0
    for c in tu.cursor.get_children():
        if c.kind != clang.cindex.CursorKind.FUNCTION_DECL or not c.is_definition():
            continue
        if c.location.file is None or real_path(c.location.file.name) != oreal:
            continue
        if c.spelling in funcs:
            dupes.add(c.spelling)
            continue
        funcs[c.spelling] = tokenize_function(c)
        i += 1
        if i % 250 == 0:
            print(f"  tokenized {i} functions", file=sys.stderr)
    for name in dupes:
        funcs.pop(name, None)
    return funcs, len(dupes)


def load_compile_args(compile_commands_path, extra_flags):
    with open(compile_commands_path) as f:
        entries = json.load(f)
    out = {}
    for e in entries:
        fpath = real_path(e["file"])
        if not fpath.startswith(SQLITE_DIR + os.sep):
            continue
        if not fpath.endswith(".c"):
            continue
        import shlex
        args = shlex.split(e["command"])
        cleaned, skip_next = [], False
        for a in args[1:]:
            if skip_next:
                skip_next = False
                continue
            if a == "-o":
                skip_next = True
                continue
            if a == "-c":
                continue
            if (real_path(a) if os.path.isabs(a) else a) == (fpath if os.path.isabs(a) else e["file"]):
                continue
            cleaned.append(a)
        cleaned += extra_flags
        out[fpath] = cleaned
    return out


def collect_current_functions(compile_args_by_file, progress=False):
    """Return name -> (file_path, tokens) for every top-level function definition.

    A fresh Index is created per file and explicitly dropped afterward --
    libclang's Python bindings don't expose TranslationUnit.dispose() in
    this environment, and reusing one Index across ~260 large TUs (each
    pulling in most of src/sqlite/*.h) grows resident memory unboundedly
    over the run otherwise.
    """
    funcs = {}
    dupes = set()
    parse_errors = 0
    items = sorted(compile_args_by_file.items())
    for i, (fpath, cargs) in enumerate(items):
        index = clang.cindex.Index.create()
        tu = index.parse(fpath, args=cargs)
        errs = [d for d in tu.diagnostics if d.severity >= clang.cindex.Diagnostic.Error]
        if errs:
            parse_errors += 1
            print(f"WARNING: {len(errs)} error(s) parsing {fpath}:", file=sys.stderr)
            for d in errs[:3]:
                print(f"  {d}", file=sys.stderr)
        for c in tu.cursor.get_children():
            if c.kind != clang.cindex.CursorKind.FUNCTION_DECL or not c.is_definition():
                continue
            if c.location.file is None or real_path(c.location.file.name) != fpath:
                continue
            if c.spelling in funcs:
                dupes.add(c.spelling)
                continue
            funcs[c.spelling] = (fpath, tokenize_function(c))
        del tu, index
        gc.collect()
        if progress and (i + 1) % 25 == 0:
            print(f"  parsed {i + 1}/{len(items)} files", file=sys.stderr)
    for name in dupes:
        funcs.pop(name, None)
    return funcs, len(dupes), parse_errors


def classify(original_funcs, current_funcs, known_constants):
    candidates = []
    stats = defaultdict(int)

    common = set(original_funcs) & set(current_funcs)
    stats["functions_in_both_trees"] = len(common)
    stats["functions_only_in_original"] = len(set(original_funcs) - set(current_funcs))
    stats["functions_only_in_current"] = len(set(current_funcs) - set(original_funcs))

    for name in sorted(common):
        otoks = original_funcs[name]
        fpath, ctoks = current_funcs[name]
        ospell = [t[1] for t in otoks]
        cspell = [t[1] for t in ctoks]

        sm = SequenceMatcher(None, ospell, cspell, autojunk=False)
        found_any = False
        for tag, i1, i2, j1, j2 in sm.get_opcodes():
            if tag != "replace":
                continue
            if (i2 - i1) != 1 or (j2 - j1) != 1:
                stats["multi_token_replace_blocks_skipped"] += 1
                continue

            okind, ospelling, _, _ = otoks[i1]
            ckind, cspelling, cstart, cend = ctoks[j1]

            if okind != "IDENTIFIER":
                continue
            if ckind != "LITERAL" or not INT_LITERAL_RE.match(cspelling):
                continue

            const = known_constants.get(ospelling)
            if const is None:
                stats["identifier_not_a_known_constant"] += 1
                continue

            cur_value = parse_int_literal(cspelling)
            if cur_value is None or cur_value != const["value"]:
                stats["value_mismatch"] += 1
                continue

            candidates.append({
                "function": name,
                "file": os.path.relpath(fpath, REPO_ROOT),
                "constant": ospelling,
                "header": const["header"],
                "old_literal": cspelling,
                "offset_start": cstart,
                "offset_end": cend,
            })
            found_any = True

        if found_any:
            stats["functions_with_candidates"] += 1

    return candidates, stats


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--original-src", default=DEFAULT_ORIGINAL_SRC,
                     help="Path to the real, un-preprocessed sqlite3.c amalgamation")
    ap.add_argument("--compile-commands", default=DEFAULT_COMPILE_COMMANDS)
    ap.add_argument("--libclang", default=find_libclang())
    ap.add_argument("-o", "--output", default=os.path.join(SCRIPT_DIR, "macro-literals-report.json"))
    args = ap.parse_args()

    if args.libclang:
        clang.cindex.Config.set_library_file(args.libclang)

    if not os.path.isfile(args.original_src):
        print(f"Error: original amalgamation not found at '{args.original_src}'. "
              f"Pass --original-src explicitly.", file=sys.stderr)
        return 1
    if not os.path.isfile(args.compile_commands):
        print(f"Error: compile_commands.json not found at '{args.compile_commands}'. "
              f"Configure the CMake build first (CMAKE_EXPORT_COMPILE_COMMANDS=ON).", file=sys.stderr)
        return 1

    sysinc = clang_system_includes()

    print("Building known-constant table from src/sqlite/*.h ...", file=sys.stderr)
    known_constants = build_known_constants(sysinc)
    print(f"  {len(known_constants)} enum constants found", file=sys.stderr)

    print(f"Parsing original amalgamation {args.original_src} ...", file=sys.stderr)
    original_funcs, original_dupes = collect_original_functions(args.original_src, sysinc)
    print(f"  {len(original_funcs)} function definitions ({original_dupes} duplicate name(s) dropped)",
          file=sys.stderr)

    print("Parsing current tree via compile_commands.json ...", file=sys.stderr)
    extra_flags = [f"-resource-dir={subprocess.check_output(['clang', '-print-resource-dir']).decode().strip()}"]
    compile_args_by_file = load_compile_args(args.compile_commands, extra_flags)
    current_funcs, current_dupes, parse_errors = collect_current_functions(compile_args_by_file, progress=True)
    print(f"  {len(current_funcs)} function definitions across {len(compile_args_by_file)} files "
          f"({current_dupes} duplicate name(s) dropped, {parse_errors} file(s) with parse errors)",
          file=sys.stderr)

    print("Classifying ...", file=sys.stderr)
    candidates, stats = classify(original_funcs, current_funcs, known_constants)

    report = {
        "candidates": candidates,
        "stats": dict(stats),
    }
    with open(args.output, "w") as f:
        json.dump(report, f, indent=2, sort_keys=True)

    print(f"\n{len(candidates)} candidate substitution site(s) across "
          f"{stats.get('functions_with_candidates', 0)} function(s)")
    for k in sorted(stats):
        print(f"  {k:38s} {stats[k]}")
    print(f"Report written to {args.output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
