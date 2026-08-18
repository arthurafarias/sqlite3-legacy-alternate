
#pragma once

#include "sqlite/BitMask.h"
#include "sqlite/LogEst.h"
  struct WhereOrCost;
  struct WhereOrCost {
    Bitmask prereq;
    LogEst rRun;
    LogEst nOut;
  };


