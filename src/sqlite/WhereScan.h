
#pragma once

#include "sqlite/i16.h"
#include "sqlite/u32.h"
  struct Expr;
  struct Index;
  struct WhereClause;
  struct WhereTerm;
  struct WhereScan;
  struct WhereScan {
    WhereClause *pOrigWC;
    WhereClause *pWC;
    const char *zCollName;
    Expr *pIdxExpr;
    int k;
    u32 opMask;
    char idxaff;
    unsigned char iEquiv;
    unsigned char nEquiv;
    int aiCur[11];
    i16 aiColumn[11];
  };

  WhereTerm *whereScanNext(WhereScan * pScan);
  __attribute__((noinline)) WhereTerm *whereScanInitIndexExpr(WhereScan * pScan);
  WhereTerm *whereScanInit(WhereScan * pScan, WhereClause * pWC, int iCur, int iColumn, u32 opMask, Index *pIdx);


