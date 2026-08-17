
#pragma once
#ifdef __cplusplus
extern C {
#endif
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

#ifdef __cplusplus
}
#endif
