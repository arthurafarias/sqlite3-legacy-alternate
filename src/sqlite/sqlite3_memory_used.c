#define _GNU_SOURCE 1

#include "sqlite/sqlite3_memory_used.h"

#include "sqlite/sqlite3.h"

sqlite3_int64 sqlite3_memory_used(void) {
  sqlite3_int64 res, mx;
  sqlite3_status64(0, &res, &mx, 0);
  return res;
}
