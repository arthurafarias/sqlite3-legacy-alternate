#pragma once

#include "sqlite/p4union.h"
#include "sqlite/u16.h"
#include "sqlite/u8.h"
struct VdbeOp;

struct VdbeOp {
  u8 opcode;
  signed char p4type;
  u16 p5;
  int p1;
  int p2;
  int p3;
  p4union p4;
};


