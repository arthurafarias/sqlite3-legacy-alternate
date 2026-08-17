#define _GNU_SOURCE 1
#include "sqlite/sqlite3_malloc.h"
#include "sqlite/sqlite3.h"
void *sqlite3_malloc(int n) {
  if (sqlite3_initialize())
    return 0;

  return n <= 0 ? 0 : sqlite3Malloc(n);
}
