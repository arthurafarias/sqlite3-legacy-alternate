
#pragma once

#include "sqlite/u8.h"
  typedef struct VdbeOpList VdbeOpList;

  struct VdbeOpList {
    u8 opcode;
    signed char p1;
    signed char p2;
    signed char p3;
  };


