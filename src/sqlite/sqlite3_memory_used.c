#define _GNU_SOURCE 1
#include "sqlite/sqlite3_memory_used.h"
#include "sqlite/sqlite3.h"
#include "sqlite/SqliteStatusParameter.h"
sqlite3_int64 sqlite3_memory_used(void) {
  sqlite3_int64 res, mx;
  sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &res, &mx, 0);
  return res;
}
