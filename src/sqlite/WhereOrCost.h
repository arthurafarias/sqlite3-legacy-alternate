
#pragma once

#include "sqlite/BitMask.h"
#include "sqlite/LogEst.h"
  typedef struct WhereOrCost WhereOrCost;
  struct WhereOrCost {
    Bitmask prereq;
    LogEst rRun;
    LogEst nOut;
  };


