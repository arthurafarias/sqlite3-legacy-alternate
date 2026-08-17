#pragma once
#ifdef __cplusplus
extern "C" {
#endif

/* Toy stand-in for the real open/insert-or-bind/retrieve/close shape of the
 * sqlite3 public API (srs-009.md's worked example), not a real database:
 * a fixed-capacity open-addressing hash table keyed by a long integer. */
typedef struct record_store record_store;

record_store *record_store_open(int capacity);
void record_store_close(record_store *self);
int record_store_insert(record_store *self, long key, const char *value);
const char *record_store_get(record_store *self, long key);

#ifdef __cplusplus
}
#endif
