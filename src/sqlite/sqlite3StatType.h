
#pragma once
#ifdef __cplusplus
extern C {
#endif

#include "sqlite/sqlite3StatValueType.h"
  typedef struct sqlite3StatType sqlite3StatType;

  struct sqlite3StatType {
    sqlite3StatValueType nowValue[10];
    sqlite3StatValueType mxValue[10];
  };

  extern sqlite3StatType sqlite3Stat;

#ifdef __cplusplus
}
#endif
