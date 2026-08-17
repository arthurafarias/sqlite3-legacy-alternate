#define _GNU_SOURCE 1

#include "sqlite/sqlite3_msize.h"

#include "sqlite/Sqlite3Config.h"

sqlite3_uint64 sqlite3_msize(void *p) { return p ? sqlite3Config.m.xSize(p) : 0; }
