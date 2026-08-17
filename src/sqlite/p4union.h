#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/i64.h"
#include "sqlite/u32.h"
typedef struct CollSeq CollSeq;
typedef struct FuncDef FuncDef;
typedef struct Index Index;
typedef struct KeyInfo KeyInfo;
typedef struct sqlite3_value Mem;
typedef struct SubProgram SubProgram;
typedef struct SubrtnSig SubrtnSig;
typedef struct Table Table;
typedef struct VTable VTable;
typedef struct sqlite3_context sqlite3_context;

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

#ifdef __cplusplus
}
#endif
