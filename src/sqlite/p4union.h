#pragma once

#include "sqlite/i64.h"
#include "sqlite/u32.h"
struct CollSeq;
struct FuncDef;
struct Index;
struct KeyInfo;
struct sqlite3_value;

struct SubProgram;
struct SubrtnSig;
struct Table;
struct VTable;
struct sqlite3_context;
typedef struct sqlite3_value Mem;

typedef union p4union p4union;
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
};
