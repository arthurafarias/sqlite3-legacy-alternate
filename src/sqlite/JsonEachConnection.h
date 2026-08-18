
#pragma once

#include "sqlite/sqlite3_vtab.h"
#include "sqlite/u8.h"
  struct sqlite3;

  struct JsonEachConnection;
  struct JsonEachConnection {
    sqlite3_vtab base;
    sqlite3 *db;
    u8 eMode;
    u8 bRecursive;
  };


