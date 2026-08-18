
#pragma once

#include "sqlite/i64.h"
  typedef struct Savepoint Savepoint;

  struct Savepoint {
    char *zName;
    i64 nDeferredCons;
    i64 nDeferredImmCons;
    Savepoint *pNext;
  };


