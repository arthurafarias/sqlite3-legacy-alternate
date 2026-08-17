# Coverage ledger (srs-009.md test-strategy rule 5)

One row per function in the coverage surface, cross-referenced against the
test id(s) and benchmark id(s) that exercise it. A row with an empty test-id
column is an uncovered function — the ledger makes that a visible gap
instead of something tracked by memory. Filled in here for the worked
example's four-function toy API; the real pass extends this table to all
290 rows of [srs-009.md's coverage-surface table](../../srs-009.md#coverage-surface).

| Function | Header | Test id(s) | Benchmark id(s) | Status |
|---|---|---|---|---|
| `record_store_open` | `record_store.h` | `test_insert_then_get_roundtrips`, `test_open_rejects_nonpositive_capacity` | `record_store_open+close (paired)` | covered |
| `record_store_close` | `record_store.h` | all five cases (teardown) | `record_store_open+close (paired)` | covered |
| `record_store_insert` | `record_store.h` | `test_insert_then_get_roundtrips`, `test_insert_overwrites_existing_key`, `test_insert_fails_when_full` | `record_store_insert (steady-state overwrite)`, `record insert (bulk load, unique keys)` | covered |
| `record_store_get` | `record_store.h` | `test_insert_then_get_roundtrips`, `test_get_missing_key_returns_null` | `record_store_get (hit)`, `record retrieve (point lookup, cache-warm)` | covered |
