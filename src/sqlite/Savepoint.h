
#pragma once
#ifdef __cplusplus
extern C {
#endif

#include "sqlite/i64.h"
  typedef struct Savepoint Savepoint;

  struct Savepoint {
    char *zName;
    i64 nDeferredCons;
    i64 nDeferredImmCons;
    Savepoint *pNext;
  };

#ifdef __cplusplus
}
#endif
