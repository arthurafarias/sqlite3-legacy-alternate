
#pragma once

#include "sqlite/i64.h"
#include "sqlite/sqlite3_value.h"
typedef struct NthValueCtx NthValueCtx;

struct NthValueCtx {
  i64 nStep;
  sqlite3_value *pValue;
};


