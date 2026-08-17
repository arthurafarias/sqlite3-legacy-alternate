#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/u8.h"
typedef struct InLoop InLoop;
struct InLoop {
  int iCur;
  int addrInTop;
  int iBase;
  int nPrefix;
  u8 eEndLoopOp;
};

#ifdef __cplusplus
}
#endif
