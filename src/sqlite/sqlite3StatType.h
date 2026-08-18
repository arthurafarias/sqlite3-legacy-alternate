
#pragma once

#include "sqlite/sqlite3StatValueType.h"
  struct sqlite3StatType;

  struct sqlite3StatType {
    sqlite3StatValueType nowValue[10];
    sqlite3StatValueType mxValue[10];
  };

  extern sqlite3StatType sqlite3Stat;


