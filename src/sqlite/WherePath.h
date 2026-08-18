
#pragma once

#include "sqlite/BitMask.h"
#include "sqlite/LogEst.h"
#include "sqlite/i8.h"
  struct WhereLoop;

  struct WherePath;
  struct WherePath {
    Bitmask maskLoop;
    Bitmask revLoop;
    LogEst nRow;
    LogEst rCost;
    LogEst rUnsort;
    i8 isOrdered;
    WhereLoop **aLoop;
  };


