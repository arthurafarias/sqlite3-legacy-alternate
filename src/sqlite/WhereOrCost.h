
#pragma once
#ifdef __cplusplus
extern C {
#endif

#include "sqlite/BitMask.h"
#include "sqlite/LogEst.h"

  typedef struct WhereOrCost WhereOrCost;
  struct WhereOrCost {
    Bitmask prereq;
    LogEst rRun;
    LogEst nOut;
  };

#ifdef __cplusplus
}
#endif
