
#pragma once

#include "sqlite/sqlite3_vtab.h"
#include "sqlite/u8.h"
  typedef struct sqlite3 sqlite3;

  typedef struct JsonEachConnection JsonEachConnection;
  struct JsonEachConnection {
    sqlite3_vtab base;
    sqlite3 *db;
    u8 eMode;
    u8 bRecursive;
  };


