
#pragma once

#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_value.h"
  struct PrintfArguments;

  struct PrintfArguments {
    int nArg;
    int nUsed;
    sqlite3_value **apArg;
  };

  sqlite3_int64 getIntArg(PrintfArguments * p);
  double getDoubleArg(PrintfArguments * p);
  char *getTextArg(PrintfArguments * p);


