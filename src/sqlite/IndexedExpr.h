
#pragma once

#include "sqlite/u8.h"
  typedef struct Expr Expr;
  typedef struct IndexedExpr IndexedExpr;

  struct IndexedExpr {
    Expr *pExpr;
    int iDataCur;
    int iIdxCur;
    int iIdxCol;
    u8 bMaybeNullRow;
    u8 aff;
    IndexedExpr *pIENext;
  };


