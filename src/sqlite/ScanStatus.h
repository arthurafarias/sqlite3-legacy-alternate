
#pragma once

#include "sqlite/LogEst.h"
  typedef struct ScanStatus ScanStatus;
  struct ScanStatus {
    int addrExplain;
    int aAddrRange[6];
    int addrLoop;
    int addrVisit;
    int iSelectID;
    LogEst nEst;
    char *zName;
  };


