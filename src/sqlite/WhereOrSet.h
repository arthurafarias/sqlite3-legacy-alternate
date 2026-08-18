
#pragma once

#include "sqlite/BitMask.h"
#include "sqlite/LogEst.h"
#include "sqlite/WhereOrCost.h"
#include "sqlite/u16.h"
  struct WhereOrSet;
  struct WhereOrSet {
    u16 n;
    WhereOrCost a[3];
  };

  void whereOrMove(WhereOrSet * pDest, WhereOrSet * pSrc);
  int whereOrInsert(WhereOrSet * pSet, Bitmask prereq, LogEst rRun, LogEst nOut);


