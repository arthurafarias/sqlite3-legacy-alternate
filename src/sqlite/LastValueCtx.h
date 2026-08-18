
#pragma once

#include "sqlite/sqlite3_value.h"
typedef struct LastValueCtx LastValueCtx;

struct LastValueCtx {
  sqlite3_value *pVal;
  int nVal;
};


