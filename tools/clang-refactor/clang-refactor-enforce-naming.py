#!/usr/bin/env python3
"""
Enforce this codebase's identifier-naming convention under src/sqlite:

  - snake_case   for functions and type names (struct/union/enum tags,
                  typedefs, type aliases, and C++ classes)
  - SCREAMING_SNAKE_CASE for enumeration constants

This wraps clang-tidy's `readability-identifier-naming` check instead of
hand-rolling a libclang rename: for a given translation unit it already
finds every declaration *and* every reference consistently in one pass,
and re-running it later against new/changed code catches drift for free
(it's a lint, not a one-shot migration).

Two-phase, like the other scripts in this directory:

  1. Every target .c file is parsed with `--export-fixes` (read-only --
     nothing is written) and the proposed renames are merged into one
     report. Renames are grouped by (kind, new name); if two distinct
     symbols of the same kind would collide on the same new name, every
     edit for both is held back and reported under "collisions" for
     manual review instead of being applied.
  2. With --apply, the surviving edits are deduplicated per file (the
     same header edited by many TUs proposes byte-identical replacements)
     and spliced in offset-descending order -- the same technique
     clang-refactor-struct-to-header.py and
     clang-refactor-scope-extern-consts.py already use in this directory,
     kept here so this script has no dependency on the separate
     clang-apply-replacements binary. clang-format is run over every
     touched file afterward.

Run with no --apply first: it only reports. Nothing is written until
you pass --apply.
"""
import argparse
import json
import os
import subprocess
import sys
import tempfile
from collections import defaultdict

import yaml

HERE = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))  # libsqlite3-legacy-amalgamation
REPO_ROOT = os.path.normpath(os.path.join(PROJECT_ROOT, "..", ".."))
DEFAULT_SRC_DIR = os.path.join(PROJECT_ROOT, "src", "sqlite")
DEFAULT_COMPILE_DB_DIR = os.path.join(REPO_ROOT, "build")

NAMING_OPTIONS = {
    "readability-identifier-naming.FunctionCase": "lower_case",
    "readability-identifier-naming.StructCase": "lower_case",
    "readability-identifier-naming.UnionCase": "lower_case",
    "readability-identifier-naming.EnumCase": "lower_case",
    "readability-identifier-naming.TypedefCase": "lower_case",
    "readability-identifier-naming.TypeAliasCase": "lower_case",
    "readability-identifier-naming.ClassCase": "lower_case",
    "readability-identifier-naming.EnumConstantCase": "UPPER_CASE",
}


def build_config():
    opts = ", ".join(f"{k}: {v}" for k, v in NAMING_OPTIONS.items())
    return "{CheckOptions: {%s}}" % opts


def real_path(p):
    return os.path.realpath(os.path.abspath(p))


def target_files(compile_db_path, src_dir, explicit_files):
    if explicit_files:
        return sorted(real_path(f) for f in explicit_files)
    with open(compile_db_path) as f:
        entries = json.load(f)
    src_dir = real_path(src_dir)
    out = {
        real_path(e["file"])
        for e in entries
        if real_path(e["file"]).startswith(src_dir + os.sep) and e["file"].endswith(".c")
    }
    return sorted(out)


def chunk(seq, n):
    n = max(1, n)
    size = (len(seq) + n - 1) // n
    return [seq[i:i + size] for i in range(0, len(seq), size)] or [[]]


def run_clang_tidy_shard(files, compile_db_dir, config, clang_tidy_bin, out_dir, shard_id):
    # Always read-only: shards run concurrently and a shared header (e.g. a
    # struct's paired .h, #included from many .c files that land in
    # different shards) would otherwise get --fix-ed by several clang-tidy
    # processes at once, racing to rewrite the same file. Writing is done as
    # a separate, single-threaded step in apply_edits() from the merged,
    # deduplicated replacement set below.
    export_path = os.path.join(out_dir, f"fixes-{shard_id}.yaml")
    cmd = [
        clang_tidy_bin,
        f"-p={compile_db_dir}",
        "--checks=-*,readability-identifier-naming",
        f"--config={config}",
        f"--export-fixes={export_path}",
        "--quiet",
    ]
    cmd += files
    proc = subprocess.run(cmd, capture_output=True, text=True)
    diags = []
    if os.path.exists(export_path):
        with open(export_path) as f:
            doc = yaml.safe_load(f) or {}
        diags = doc.get("Diagnostics") or []
    return diags, proc.returncode, proc.stderr


KIND_RE_MESSAGE_PREFIX = "invalid case style for "


def parse_diagnostic(diag):
    msg = diag["DiagnosticMessage"]
    text = msg["Message"]
    if not text.startswith(KIND_RE_MESSAGE_PREFIX):
        return None
    rest = text[len(KIND_RE_MESSAGE_PREFIX):]
    kind, _, quoted = rest.partition(" '")
    old_name = quoted.rstrip("'")
    replacements = msg.get("Replacements") or []
    if not replacements:
        return None
    new_name = replacements[0]["ReplacementText"]
    return kind, old_name, new_name, replacements


def collect_diagnostics(files, compile_db_dir, config, clang_tidy_bin, jobs):
    import concurrent.futures

    all_diags = []
    errors = []
    with tempfile.TemporaryDirectory() as tmp:
        shards = chunk(files, jobs)
        with concurrent.futures.ThreadPoolExecutor(max_workers=len(shards)) as pool:
            futures = [
                pool.submit(run_clang_tidy_shard, shard, compile_db_dir, config,
                            clang_tidy_bin, tmp, i)
                for i, shard in enumerate(shards) if shard
            ]
            for fut in concurrent.futures.as_completed(futures):
                diags, rc, stderr = fut.result()
                all_diags.extend(diags)
                if rc not in (0, 1):
                    errors.append(stderr.strip())
    return all_diags, errors


def build_report(diags):
    """Group by (kind, old_name) -> new_name, and detect same-kind collisions
    where two distinct old names would land on the same new name."""
    by_symbol = {}  # (kind, old_name) -> {"new_name": str, "edits": {(file, offset, length): text}}
    for diag in diags:
        parsed = parse_diagnostic(diag)
        if parsed is None:
            continue
        kind, old_name, new_name, replacements = parsed
        key = (kind, old_name)
        entry = by_symbol.setdefault(key, {"new_name": new_name, "edits": {}})
        assert entry["new_name"] == new_name, (
            f"clang-tidy proposed two different names for {kind} '{old_name}': "
            f"'{entry['new_name']}' vs '{new_name}'"
        )
        for r in replacements:
            entry["edits"][(r["FilePath"], r["Offset"], r["Length"])] = r["ReplacementText"]

    by_target = defaultdict(list)  # (kind, new_name) -> [old_name, ...]
    for (kind, old_name), entry in by_symbol.items():
        by_target[(kind, entry["new_name"])].append(old_name)

    collisions = {k: v for k, v in by_target.items() if len(v) > 1}
    collision_symbols = {(kind, old) for (kind, _new), olds in collisions.items()
                          for old in olds for kind in [kind]}

    clean, held_back = {}, {}
    for (kind, old_name), entry in by_symbol.items():
        target = (kind, old_name) in collision_symbols
        (held_back if target else clean)[(kind, old_name)] = entry

    return clean, held_back, collisions


def apply_edits(clean, clang_format_bin):
    edits_by_file = defaultdict(dict)  # file -> {(offset, length): text}
    for entry in clean.values():
        for (fpath, offset, length), text in entry["edits"].items():
            existing = edits_by_file[fpath].get((offset, length))
            if existing is not None and existing != text:
                print(f"warning: conflicting edits at {fpath}:{offset} "
                      f"('{existing}' vs '{text}'); skipping both", file=sys.stderr)
                edits_by_file[fpath][(offset, length)] = None
            else:
                edits_by_file[fpath][(offset, length)] = text

    touched = []
    for fpath, spans in edits_by_file.items():
        with open(fpath, "r", encoding="utf-8") as f:
            content = f.read()
        ordered = sorted(
            ((offset, length, text) for (offset, length), text in spans.items() if text is not None),
            key=lambda t: t[0],
            reverse=True,
        )
        for offset, length, text in ordered:
            content = content[:offset] + text + content[offset + length:]
        with open(fpath, "w", encoding="utf-8") as f:
            f.write(content)
        touched.append(fpath)

    if touched and clang_format_bin:
        subprocess.run([clang_format_bin, "-i"] + touched)
    return touched


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("files", nargs="*",
                     help="Specific .c files to check/fix (default: every .c file "
                          "under --src-dir found in the compile database)")
    ap.add_argument("--src-dir", default=DEFAULT_SRC_DIR)
    ap.add_argument("--compile-db", default=DEFAULT_COMPILE_DB_DIR,
                     help="Directory containing compile_commands.json")
    ap.add_argument("--clang-tidy", default="clang-tidy")
    ap.add_argument("--clang-format", default="clang-format")
    ap.add_argument("--apply", action="store_true",
                     help="Write renames in place (default: report only, nothing written)")
    ap.add_argument("--report", help="Write the full JSON report to this path")
    ap.add_argument("-j", "--jobs", type=int, default=os.cpu_count() or 4)
    args = ap.parse_args()

    compile_db_path = os.path.join(args.compile_db, "compile_commands.json")
    if not os.path.isfile(compile_db_path):
        print(f"Error: {compile_db_path} not found. Configure the CMake build first "
              f"(CMAKE_EXPORT_COMPILE_COMMANDS=ON).", file=sys.stderr)
        return 1

    files = target_files(compile_db_path, args.src_dir, args.files)
    if not files:
        print(f"Error: no .c files under '{args.src_dir}' found in '{compile_db_path}'.", file=sys.stderr)
        return 1

    print(f"Checking {len(files)} file(s) with {args.jobs} worker(s)...", file=sys.stderr)
    diags, tidy_errors = collect_diagnostics(
        files, args.compile_db, build_config(), args.clang_tidy, args.jobs
    )
    for e in tidy_errors:
        print(e, file=sys.stderr)

    clean, held_back, collisions = build_report(diags)

    counts = defaultdict(int)
    for kind, _ in clean:
        counts[kind] += 1
    print(f"\n{sum(counts.values())} non-conforming identifier(s) found:")
    for kind in sorted(counts):
        print(f"  {kind:26s} {counts[kind]}")

    if held_back:
        print(f"\n{len(held_back)} identifier(s) held back due to naming collisions "
              f"(needs manual review, not auto-applied):")
        for (kind, new_name), old_names in sorted(collisions.items()):
            print(f"  {kind} '{new_name}' <- {old_names}")

    if args.report:
        report = {
            "renames": {f"{k} {n}": v["new_name"] for (k, n), v in clean.items()},
            "collisions": {f"{k} {n}": olds for (k, n), olds in collisions.items()},
        }
        with open(args.report, "w") as f:
            json.dump(report, f, indent=2, sort_keys=True)
        print(f"\nReport written to '{args.report}'")

    if not args.apply:
        print("\nDry run only -- pass --apply to write these renames.")
        return 0

    touched = apply_edits(clean, args.clang_format)
    print(f"\nApplied renames across {len(touched)} file(s). Rebuild the project to verify.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
