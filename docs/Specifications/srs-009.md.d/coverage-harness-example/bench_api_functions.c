/* One benchmark entry per record_store function (srs-009.md
 * benchmark-strategy rule 1) — the pattern a real pass repeats for all 290
 * public sqlite3_* functions. */
#include "bench_harness.h"
#include "record_store.h"

int main(void) {
  record_store *s = record_store_open(200000);
  long key = 0;

  BENCH("record_store_insert (steady-state overwrite)", 100000, 1000, {
    record_store_insert(s, key % 1000, "benchmark-value");
    key++;
  });

  BENCH("record_store_get (hit)", 100000, 1000, {
    record_store_get(s, key % 1000);
    key++;
  });

  record_store_close(s);

  /* open/close have no meaningful teardown-free measurement on their own —
   * timed paired, matching each open with its close inside the loop body. */
  BENCH("record_store_open+close (paired)", 10000, 100, {
    record_store *tmp = record_store_open(64);
    record_store_close(tmp);
  });

  return 0;
}
