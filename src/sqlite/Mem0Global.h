
#pragma once

#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_mutex.h"
typedef struct Mem0Global Mem0Global;

struct Mem0Global {
  sqlite3_mutex *mutex;
  sqlite3_int64 alarmThreshold;
  sqlite3_int64 hardLimit;

  int nearlyFull;
};

extern struct Mem0Global mem0;


