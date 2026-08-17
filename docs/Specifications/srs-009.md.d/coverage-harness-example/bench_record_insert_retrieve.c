/* Dedicated end-to-end record insert/retrieve benchmark (srs-009.md
 * benchmark-strategy rule 2), separate from the per-function
 * micro-benchmarks in bench_api_functions.c. */
#include <stdio.h>
#include "bench_harness.h"
#include "record_store.h"

#define RECORD_COUNT 100000

int main(void) {
  record_store *s = record_store_open(RECORD_COUNT * 2);
  char value[32];
  long key;

  /* Bulk load: zero warm-up, since warming up would already insert every
   * unique key before the timed region (benchmark-strategy rule 3). */
  key = 0;
  BENCH("record insert (bulk load, unique keys)", RECORD_COUNT, 0, {
    snprintf(value, sizeof value, "value-%ld", key);
    record_store_insert(s, key, value);
    key++;
  });

  /* Point lookup: warm-up is the correct steady-state measurement here. */
  key = 0;
  BENCH("record retrieve (point lookup, cache-warm)", RECORD_COUNT, 1000, {
    const char *v = record_store_get(s, key % RECORD_COUNT);
    (void)v;
    key++;
  });

  record_store_close(s);
  return 0;
}
