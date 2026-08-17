# Architecture

## Starting point

The source of truth this project decomposes is the preprocessed SQLite
3.53.4 amalgamation (`sqlite3.E.c`, built from
`libraries/libsqlite3-legacy/pkg/sqlite-src-3530400` in the parent
workspace) — the single ~230K-line file the upstream SQLite project
recommends embedding directly rather than its hundreds of internal source
files. That file is the input; `src/sqlite/` here is the output of pulling
it apart.

## The container convention

Every struct, function, and primitive typedef gets its own file pair under
`src/sqlite/`:

```
src/sqlite/Btree.c       src/sqlite/Btree.h
src/sqlite/Select.c      src/sqlite/Select.h
src/sqlite/i16.c         src/sqlite/i16.h
```

The `.h` declares the symbol; the `.c` defines it. This holds uniformly —
`i16.h` declaring `typedef short i16;` gets exactly the same treatment as
`Btree.h` declaring the B-tree cursor struct — which is what makes the rest
of the tooling below possible: every script can assume "one header per
symbol, one definition file per symbol" without special-casing what kind of
symbol it is.

As of this writing that convention covers 262 `.c` files and 322 `.h` files.
`tools/clang-refactor/uncategorized-report.json` records the last sweep of
symbols that hadn't yet been placed (113 at that point: 91 used from a
single translation unit, 22 used from more than one) — see below.

## The refactor toolchain

`tools/clang-refactor/` is a set of standalone Python scripts, each doing
one narrow, mechanical transformation over the tree using `libclang` (AST-
accurate identifier and reference resolution) or `clang-tidy` directly.
None of them are run automatically as part of the build — they're applied
by hand as one-off or repeatable passes while the tree is reshaped, then
the result is committed like any other edit.

| Script | Does |
|---|---|
| `clang-refactor-functions-to-containers.py` | The seed pass: reads the preprocessed amalgamation and splits it into the one-file-per-symbol layout described above. |
| `clang-refactor-struct-to-header.py` | Pulls one named struct out of wherever it currently lives into its own header/source pair. |
| `clang-refactor-remove-all-h.py` | Replaces a blanket `#include "sqlite/_All.h"` in a `.c` file with the precise set of container headers and system headers that file actually needs, resolved via libclang rather than guessing from the canonical declaration. Already applied tree-wide — `_All.h` no longer exists. |
| `clang-refactor-categorize-uncategorized.py` + `clang-refactor-apply-uncategorized.py` | Two-phase pass over a catch-all `_Uncategorized.h` of functions that leaked in from elsewhere. The first phase classifies each by how many `.c` files reference it; the second makes single-file ones `static` (with the forward declaration moved into that file) and gives multi-file ones a proper home. Already applied — `_Uncategorized.h` no longer exists; the JSON report is the historical record of that pass. |
| `clang-refactor-scope-extern-consts.py` | Same idea as the uncategorized pass, but for top-level `extern const` declarations: pushes each into the translation unit(s) that actually need it instead of a shared dumping-ground header. |
| `clang-refactor-forward-declare-members.py` | For a struct's header, forward-declares any member field whose type is a pointer to another struct instead of `#include`-ing that struct's full header, pushing the `#include` down into the `.c` file that actually dereferences the pointer. Skips integral typedef fields (`u8`, `u16`, `Pgno`, ...) — they can't be forward-declared and don't own a definition to hide. |
| `clang-refactor-enforce-naming.py` | Wraps clang-tidy's `readability-identifier-naming` to enforce `snake_case` for functions and type names and `SCREAMING_SNAKE_CASE` for enum constants — a lint, so it's meant to be re-run as new code lands, not just once. |
| `clang-refactor-remove-void-noop.py` | Deletes dead `((void)(0));` statements left behind where `assert()`/`testcase()` macros expand to nothing under the active build config, then collapses the blank-line runs that removing them leaves behind. |
| `clang-refactor-source-reorder.py` | Reorders top-level declarations within a file. |

Most of these follow the same shape on purpose: a read-only analysis phase
that reports what it would do, and a separate apply phase — so a pass can be
inspected before it touches the tree, and re-run safely as the tree changes
under it.

## Build

A single CMake target (`sqlite3-legacy-amalgamation`) globs every `.c`/`.cpp`
under `src/` into one shared library, compiled as C23
(`target_compile_features(... c_std_23)`) and linked against `Threads` and
`libm`. There is currently no split by subsystem — unlike the sibling
`libsqlite-*` libraries in the parent workspace (see
[Relationship to the parent workspace](index.md#relationship-to-the-parent-workspace)),
this project keeps the whole amalgamation as one compilation target while it
reshapes what's *inside* that target file by file.

## Where this is headed

[`srs-002.md`](Specifications/srs-002.md) is a standing TODO: apply an
iterator pattern to every iterable construct in the library, following the
shape sketched in
[`srs-002.md.d/iterable-pattern-example/`](Specifications/srs-002.md.d/iterable-pattern-example/) —
a `list` / `list_node` / `list_iterator` split with explicit
`alloc`/`construct`/`destruct` lifecycle functions and predicate-based
search (`list_find(list*, list_find_predicate*)`), written as C that's also
valid as a C++ translation unit (`extern "C"` guards throughout). That
example is scaffolding, not yet applied to any real container in
`src/sqlite/`. `srs-001.md`, the other Specification in this directory, is
still an empty placeholder. Read the srs-002 example before starting on
this pass; it's the nearest thing this repository has to a style guide for
where the C-to-C++ conversion is going.
