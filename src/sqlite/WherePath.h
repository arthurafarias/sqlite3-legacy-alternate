
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/BitMask.h"
#include "sqlite/LogEst.h"
#include "sqlite/i8.h"
  typedef struct WhereLoop WhereLoop;

  typedef struct WherePath WherePath;
  struct WherePath {
    Bitmask maskLoop;
    Bitmask revLoop;
    LogEst nRow;
    LogEst rCost;
    LogEst rUnsort;
    i8 isOrdered;
    WhereLoop **aLoop;
  };

#ifdef __cplusplus
}
#endif
