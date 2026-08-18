
#pragma once

#include "sqlite/Mem.h"
#include "sqlite/Op.h"
#include "sqlite/i64.h"
#include "sqlite/u8.h"
  struct AuxData;
  struct Vdbe;
  struct VdbeCursor;

  struct VdbeFrame;
  struct VdbeFrame {
    Vdbe *v;
    VdbeFrame *pParent;
    Op *aOp;
    Mem *aMem;
    VdbeCursor **apCsr;
    u8 *aOnce;
    void *token;
    i64 lastRowid;
    AuxData *pAuxData;

    int nCursor;
    int pc;
    int nOp;
    int nMem;
    int nChildMem;
    int nChildCsr;
    i64 nChange;
    i64 nDbChange;
  };

  void sqlite3VdbeFrameMemDel(void *);
  void sqlite3VdbeFrameDelete(VdbeFrame *);
  int sqlite3VdbeFrameRestore(VdbeFrame *);


