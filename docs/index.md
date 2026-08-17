# Documentation

Docs for `sqlite3-legacy-amalgamation`: a decomposition of the SQLite 3.53.4
amalgamation into one C source/header pair per struct, function, and
typedef, kept building as a single shared library throughout.

- [Architecture](architecture.md) — the one-file-per-symbol container
  convention, what each script in `tools/clang-refactor/` does to the tree,
  and the build.
- [Specifications/](Specifications/) — the SRS documents driving the next
  stage of the refactor (currently: converting iterable constructs to an
  iterator pattern).
- [Logo](logo/) — the project mark, its rationale, and usage.
- [../README.md](../README.md) — project overview and build instructions.

## Relationship to the parent workspace

This directory lives inside the larger `sqlite-cpp` workspace (see that
repository's own `docs/index.md` and `srs/index.md`), which runs a
different, subsystem-by-subsystem split of SQLite
(`libsqlite-utils`, `libsqlite-backend-*`, `libsqlite-core-*`,
`libsqlite-compiler-*`) through a four-stage SRS pipeline (legacy restructure
→ C++ conversion → namespace-mirrored file layout → STL-based internals).
This project is not one of those libraries and is not part of that
numbered pipeline — it's an independent, more granular decomposition of the
*whole* amalgamation (one file per symbol, rather than one library per
subsystem), tracked with its own local `srs-001`/`srs-002` documents under
[`Specifications/`](Specifications/). It has its own git history, separate
from the parent workspace's.
