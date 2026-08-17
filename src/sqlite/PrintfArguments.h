
#pragma once
#ifdef __cplusplus
extern C {
#endif

#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_value.h"

  typedef struct PrintfArguments PrintfArguments;

  struct PrintfArguments {
    int nArg;
    int nUsed;
    sqlite3_value **apArg;
  };

  sqlite3_int64 getIntArg(PrintfArguments * p);
  double getDoubleArg(PrintfArguments * p);
  char *getTextArg(PrintfArguments * p);

#ifdef __cplusplus
}
#endif
