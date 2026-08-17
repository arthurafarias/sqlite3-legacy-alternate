#define _GNU_SOURCE 1
#include "sqlite/sqlite3_free.h"
#include "sqlite/Mem0Global.h"
#include "sqlite/Sqlite3Config.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/SqliteStatusParameter.h"
void sqlite3_free(void *p) {
  if (p == 0)
    return;

  if (sqlite3Config.bMemstat) {
    sqlite3_mutex_enter(mem0.mutex);
    sqlite3StatusDown(SQLITE_STATUS_MEMORY_USED, sqlite3MallocSize(p));
    sqlite3StatusDown(SQLITE_STATUS_MALLOC_COUNT, 1);
    sqlite3Config.m.xFree(p);
    sqlite3_mutex_leave(mem0.mutex);
  } else {
    sqlite3Config.m.xFree(p);
  }
}
