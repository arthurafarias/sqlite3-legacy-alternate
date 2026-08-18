
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/Mem.h"
#include "sqlite/Op.h"
#include "sqlite/i64.h"
#include "sqlite/u8.h"
  typedef struct AuxData AuxData;
  typedef struct Vdbe Vdbe;
  typedef struct VdbeCursor VdbeCursor;

  typedef struct VdbeFrame VdbeFrame;
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

#ifdef __cplusplus
}
#endif
