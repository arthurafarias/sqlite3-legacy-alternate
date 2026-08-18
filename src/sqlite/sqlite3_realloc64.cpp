#define _GNU_SOURCE 1
#include "sqlite/sqlite3_realloc64.h"
#include "sqlite/sqlite3.h"
void *sqlite3_realloc64(void *pOld, sqlite3_uint64 n) {
  if (sqlite3_initialize())
    return 0;

  return sqlite3Realloc(pOld, n);
}
