
#pragma once

#include "sqlite/u8.h"
  struct Expr;
  struct IndexedExpr;

  struct IndexedExpr {
    Expr *pExpr;
    int iDataCur;
    int iIdxCur;
    int iIdxCol;
    u8 bMaybeNullRow;
    u8 aff;
    IndexedExpr *pIENext;
  };


