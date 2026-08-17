
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/u8.h"
  typedef struct VdbeOp VdbeOp;
  typedef struct SubProgram SubProgram;

  struct SubProgram {
    VdbeOp *aOp;
    int nOp;
    int nMem;
    int nCsr;
    u8 *aOnce;
    void *token;
    SubProgram *pNext;
  };

#ifdef __cplusplus
}
#endif
