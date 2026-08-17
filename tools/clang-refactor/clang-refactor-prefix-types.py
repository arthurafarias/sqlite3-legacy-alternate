#!/usr/bin/env python3
"""
Enforce SRS-005 (see docs/Specifications/srs-005.md): every struct, union,
enum, typedef, and type alias declared under src/sqlite/ carries the
sqlite3_ prefix. Functions, enum constants, and variables are untouched --
this is a type/structure-only rule, orthogonal to the lower_case convention
clang-refactor-enforce-naming.py already enforces for the same set of
declaration kinds.

Same two-phase clang-tidy wrapper as clang-refactor-enforce-naming.py, for
the same reasons: readability-identifier-naming's Prefix option already
finds every declaration *and* every reference in one AST-accurate pass, is
idempotent (a symbol that already carries the prefix -- sqlite3_context,
sqlite3_vfs, and the rest of the real public API -- is left alone, not
double-prefixed), and re-running it later against new code catches drift
for free.

  1. Every target .c file is parsed with `--export-fixes` (read-only --
     nothing is written) and the proposed renames are merged into one
     report. Renames are grouped by (kind, new name); if two distinct
     symbols of the same kind would collide on the same new name, every
     edit for both is held back and reported under "collisions" for manual
     review instead of being applied.
  2. With --apply, the surviving edits are deduplicated per file and
     spliced in offset-descending order, then clang-format is run over
     every touched file.

A third, optional phase closes the loop with clangd rather than just
clang-tidy: with --verify (on by default whenever --apply runs),
`clangd --check` is run over every touched file. clang-tidy's fix-its are
themselves AST-derived and already catch every reference in the
translation units it parses, so --verify isn't a second rename pass -- it
rebuilds clangd's own type cache for each touched file post-edit and
reports any diagnostic that shows up there (e.g. a reference clang-tidy's
sharding put in a TU this tool didn't parse), so a partial or missed rename
is caught before it's mistaken for a clean run.

Run with no --apply first: it only reports. Nothing is written until you
pass --apply.
"""
import argparse
import glob
import json
import os
import re
import subprocess
import sys
import tempfile
from collections import defaultdict

import yaml

HERE = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))  # sqlite3-legacy-alternate
REPO_ROOT = os.path.normpath(os.path.join(PROJECT_ROOT, "..", ".."))
DEFAULT_SRC_DIR = os.path.join(PROJECT_ROOT, "src", "sqlite")
DEFAULT_COMPILE_DB_DIR = os.path.join(REPO_ROOT, "build")

PREFIX = "sqlite3_"

# Real, external SQLite API type names that don't happen to start with
# "sqlite3_" -- Rule 2's idempotency check only catches already-prefixed
# names, so without this explicit list clang-tidy will happily "fix" these
# into non-existent names, breaking the one thing srs-005 exists to
# preserve. `sqlite3` itself (the connection handle) is the one type in the
# entire public API that omits the trailing underscore; the fts5_* names
# are the FTS5 extension API, which upstream never put under the sqlite3_
# umbrella at all.
EXEMPT_NAMES = {
    "sqlite3",
    "fts5_api",
    "fts5_context",
    "fts5_extension_api",
    "fts5_extension_function",
    "fts5_phrase_iter",
    "fts5_tokenizer_v2",
    "fts5_tokenizer",
    "Fts5Tokenizer",
}

# Type/structure kinds only -- functions, enum constants, and variables are
# out of scope for SRS-005 and are left to clang-refactor-enforce-naming.py.
PREFIX_OPTIONS = {
    "readability-identifier-naming.StructCase": "lower_case",
    "readability-identifier-naming.StructPrefix": PREFIX,
    "readability-identifier-naming.UnionCase": "lower_case",
    "readability-identifier-naming.UnionPrefix": PREFIX,
    "readability-identifier-naming.EnumCase": "lower_case",
    "readability-identifier-naming.EnumPrefix": PREFIX,
    "readability-identifier-naming.TypedefCase": "lower_case",
    "readability-identifier-naming.TypedefPrefix": PREFIX,
    "readability-identifier-naming.TypeAliasCase": "lower_case",
    "readability-identifier-naming.TypeAliasPrefix": PREFIX,
    "readability-identifier-naming.ClassCase": "lower_case",
    "readability-identifier-naming.ClassPrefix": PREFIX,
}


def build_config():
    opts = ", ".join(f"{k}: {v}" for k, v in PREFIX_OPTIONS.items())
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
    # Read-only, same reasoning as clang-refactor-enforce-naming.py: shards
    # run concurrently and a shared header pulled in from several .c files
    # across different shards would otherwise be --fix-ed by more than one
    # clang-tidy process at once. Writing happens once, single-threaded, in
    # apply_edits() below.
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


def build_report(diags):
    """Group by (kind, old_name) -> new_name, and detect same-kind collisions
    where two distinct old names would land on the same new name (e.g. an
    existing Db and DB struct would both become sqlite3_db)."""
    by_symbol = {}  # (kind, old_name) -> {"new_name": str, "edits": {(file, offset, length): text}}
    for diag in diags:
        parsed = parse_diagnostic(diag)
        if parsed is None:
            continue
        kind, old_name, new_name, replacements = parsed
        if old_name in EXEMPT_NAMES:
            continue
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


IDENT_RE = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")


def find_existing_name_conflicts(clean, src_dir):
    """A proposed new_name that already appears verbatim as an identifier
    somewhere under src_dir conflicts with something clang-tidy didn't flag
    -- because it already conforms (Rule 2: idempotent) and so never shows
    up in the diagnostics build_report() groups. build_report()'s collision
    detection only compares proposed renames against each other; it can't
    see a target name that was never proposed because it was already
    correct. The sharpest case in this tree: the internal `Module` struct
    prefixes to `sqlite3_module`, which is already the real, distinct
    public API vtable-methods type -- applying that rename verbatim would
    silently try to redefine it. Held back for manual review, same
    disposition as an in-batch collision (see srs-005.md Rule 4)."""
    existing = set()
    for pattern in ("*.c", "*.h"):
        for fpath in glob.glob(os.path.join(src_dir, "**", pattern), recursive=True):
            with open(fpath, encoding="utf-8", errors="replace") as f:
                existing.update(IDENT_RE.findall(f.read()))

    ok, conflicts = {}, {}
    for (kind, old_name), entry in clean.items():
        new_name = entry["new_name"]
        if new_name != old_name and new_name in existing:
            conflicts[(kind, old_name)] = entry
        else:
            ok[(kind, old_name)] = entry
    return ok, conflicts


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


def verify_with_clangd(touched, compile_db_dir, clangd_bin):
    """Rebuild clangd's type cache for each touched file post-edit and
    surface anything it flags -- a second, independent check on top of
    clang-tidy's own AST-derived fix-its, using a different indexer against
    the same compile database."""
    if not touched:
        return True
    ok = True
    for fpath in touched:
        cmd = [clangd_bin, f"--compile-commands-dir={compile_db_dir}", f"--check={fpath}"]
        proc = subprocess.run(cmd, capture_output=True, text=True)
        if proc.returncode != 0:
            ok = False
            print(f"\nclangd --check found issues in {fpath}:", file=sys.stderr)
            print(proc.stderr.strip(), file=sys.stderr)
    return ok


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
    ap.add_argument("--clangd", default="clangd")
    ap.add_argument("--apply", action="store_true",
                     help="Write renames in place (default: report only, nothing written)")
    ap.add_argument("--verify", dest="verify", action="store_true", default=True,
                     help="Run `clangd --check` over touched files after --apply (default: on)")
    ap.add_argument("--no-verify", dest="verify", action="store_false")
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
    clean, name_conflicts = find_existing_name_conflicts(clean, args.src_dir)

    counts = defaultdict(int)
    for kind, _ in clean:
        counts[kind] += 1
    print(f"\n{sum(counts.values())} type(s)/structure(s) missing the '{PREFIX}' prefix:")
    for kind in sorted(counts):
        print(f"  {kind:26s} {counts[kind]}")

    if held_back:
        print(f"\n{len(held_back)} identifier(s) held back due to naming collisions "
              f"(needs manual review, not auto-applied):")
        for (kind, new_name), old_names in sorted(collisions.items()):
            print(f"  {kind} '{new_name}' <- {old_names}")

    if name_conflicts:
        print(f"\n{len(name_conflicts)} identifier(s) held back because the proposed "
              f"name already exists elsewhere in the tree (needs manual review, not "
              f"auto-applied):")
        for (kind, old_name), entry in sorted(name_conflicts.items()):
            print(f"  {kind} '{old_name}' -> '{entry['new_name']}' (walready in use)")

    if args.report:
        report = {
            "renames": {f"{k} {n}": v["new_name"] for (k, n), v in clean.items()},
            "collisions": {f"{k} {n}": olds for (k, n), olds in collisions.items()},
            "name_conflicts": {f"{k} {n}": v["new_name"] for (k, n), v in name_conflicts.items()},
        }
        with open(args.report, "w") as f:
            json.dump(report, f, indent=2, sort_keys=True)
        print(f"\nReport written to '{args.report}'")

    if not args.apply:
        print("\nDry run only -- pass --apply to write these renames.")
        return 0

    touched = apply_edits(clean, args.clang_format)
    print(f"\nApplied renames across {len(touched)} file(s). Rebuild the project to verify.")

    if args.verify:
        print("\nVerifying touched files with clangd --check...", file=sys.stderr)
        if verify_with_clangd(touched, args.compile_db, args.clangd):
            print("clangd --check: no issues in touched files.")
        else:
            print("\nclangd --check reported issues above -- inspect before trusting this pass.",
                  file=sys.stderr)
            return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
