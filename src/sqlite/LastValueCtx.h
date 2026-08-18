
#pragma once

#include "sqlite/sqlite3_value.h"
struct LastValueCtx;

struct LastValueCtx {
  sqlite3_value *pVal;
  int nVal;
};


