
#pragma once

#include "sqlite/LogEst.h"
#include "sqlite/u8.h"
  typedef struct CteUse CteUse;

  struct CteUse {
    int nUse;
    int addrM9e;
    int regRtn;
    int iCur;
    LogEst nRowEst;
    u8 eM10d;
  };


