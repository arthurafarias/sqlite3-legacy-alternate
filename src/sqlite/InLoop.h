#pragma once

#include "sqlite/u8.h"
struct InLoop;
struct InLoop {
  int iCur;
  int addrInTop;
  int iBase;
  int nPrefix;
  u8 eEndLoopOp;
};


