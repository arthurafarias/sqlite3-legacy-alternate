#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sqlite/CollSeq.h"
#include "sqlite/FuncDef.h"
#include "sqlite/Index.h"
#include "sqlite/KeyInfo.h"
#include "sqlite/Mem.h"
#include "sqlite/SubProgram.h"
#include "sqlite/SubrtnSig.h"
#include "sqlite/Table.h"
#include "sqlite/VTable.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3_context.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"

typedef struct VdbeOp VdbeOp;

struct VdbeOp {
  u8 opcode;
  signed char p4type;
  u16 p5;
  int p1;
  int p2;
  int p3;
  union p4union {
    int i;
    void *p;
    char *z;
    i64 *pI64;
    double *pReal;
    FuncDef *pFunc;
    sqlite3_context *pCtx;
    CollSeq *pColl;
    Mem *pMem;
    VTable *pVtab;
    KeyInfo *pKeyInfo;
    u32 *ai;
    SubProgram *pProgram;
    Table *pTab;
    SubrtnSig *pSubrtnSig;
    Index *pIdx;

  } p4;
};

#ifdef __cplusplus
}
#endif