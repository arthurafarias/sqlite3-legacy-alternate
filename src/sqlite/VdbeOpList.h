
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/u8.h"
  typedef struct VdbeOpList VdbeOpList;

  struct VdbeOpList {
    u8 opcode;
    signed char p1;
    signed char p2;
    signed char p3;
  };

#ifdef __cplusplus
}
#endif
