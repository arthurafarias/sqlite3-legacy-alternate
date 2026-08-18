
#pragma once

#include "sqlite/u8.h"
  struct VdbeOp;
  struct SubProgram;

  struct SubProgram {
    VdbeOp *aOp;
    int nOp;
    int nMem;
    int nCsr;
    u8 *aOnce;
    void *token;
    SubProgram *pNext;
  };


