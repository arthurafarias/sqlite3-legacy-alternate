# SRS Index

Software Requirement Specifications driving `sqlite3-legacy-alternate`'s
refactor of the SQLite 3.53.4 amalgamation, tracked as one document per
convention or pass. See [architecture.md](../architecture.md) for the
container convention these build on, and [docs/index.md](../index.md) for
how this local numbering relates to the parent workspace's own SRS
pipeline (unrelated — see the note there).

| # | Title | Status | Depends on |
|---|-------|--------|------------|
| [srs-001](srs-001.md) | *(untitled — empty placeholder)* | ⬜ Not written | — |
| [srs-002](srs-002.md) | Iterable → Iterator Pattern Conversion | ⬜ Not applied (TODO stub; worked example exists) | — |
| [srs-003](srs-003.md) | Façade Visibility Convention | ✅ Applied tree-wide | — |
| [srs-004](srs-004.md) | Identifier Naming Convention (`snake_case`) | ⬜ Not applied (tooling ready) | — |
| [srs-005](srs-005.md) | `sqlite3_` Type Prefix Convention | ⬜ Not applied tree-wide (tooling ready; 8 identifiers held back for manual resolution) | — |
| [srs-006](srs-006.md) | Subsystem Namespace Prefix Convention | ⬜ Not applied (blocked on srs-004) | srs-004 |
| [srs-007](srs-007.md) | Method-Table Indirection Removal | ⬜ Draft, not applied | should land before srs-004 |
| [srs-008](srs-008.md) | Warning-Clean Compilation (`-Wall -pedantic -Werror`) | ⬜ Not applied (3 empty TUs need a dead-vs-restore decision first) | — |
| [srs-009](srs-009.md) | Amalgamation Macro-Literal Restoration | ⬜ Tooling built, classify run, apply not yet run | — |

## Reading order / dependency notes

- **srs-001** is an empty placeholder — no content to track yet.
- **srs-002** and **srs-003** are independent of each other and of
  everything below; srs-002 is a standing TODO with only a worked example
  (`srs-002.md.d/iterable-pattern-example/`) checked in, srs-003 is the one
  fully applied pass in the set.
- **srs-004** (naming) is a prerequisite for **srs-006** (namespace
  prefixing builds on the flat `sqlite3_<snake>` shape srs-004 produces).
- **srs-007** (method-table collapse) should land *before* srs-004's
  tree-wide rename, since it rewrites call sites that are cheaper to touch
  once against today's `sqlite3OsRead`-style names than once now and again
  after renaming.
- **srs-009** (macro-literal restoration) is independent of everything
  else in this list — it rewrites integer literals inside function bodies
  that are already in their correct file, so it doesn't depend on srs-003's
  container convention, srs-004/srs-005/srs-006's naming/prefixing passes,
  or srs-008's warning cleanup landing first.
- **srs-005** (type prefixing) is orthogonal to srs-003/srs-004/srs-006 —
  it only changes how a type's own name is spelled, not where it lives or
  what naming case it uses.

Only **srs-003** is currently reflected in `src/sqlite/`; everything else
above is planning/tooling checked in ahead of the pass it describes.
