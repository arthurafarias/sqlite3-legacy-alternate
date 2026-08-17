#define _GNU_SOURCE 1

#include "sqlite/sqlite3_malloc64.h"

#include "sqlite/sqlite3.h"

void *sqlite3_malloc64(sqlite3_uint64 n) {

  if (sqlite3_initialize())
    return 0;

  return sqlite3Malloc(n);
}
