#define _GNU_SOURCE 1
#include "sqlite/sqlite3_realloc.h"
#include "sqlite/sqlite3.h"
void *sqlite3_realloc(void *pOld, int n) {
  if (sqlite3_initialize())
    return 0;

  if (n < 0)
    n = 0;
  return sqlite3Realloc(pOld, n);
}
