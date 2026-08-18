
#pragma once

#include "sqlite/BitMask.h"
  struct Expr;
  struct ExprList;
  struct Select;

  struct WhereMaskSet;
  struct WhereMaskSet {
    int bVarSelect;
    int n;
    int ix[((int)(sizeof(Bitmask) * 8))];
  };

  Bitmask sqlite3WhereGetMask(WhereMaskSet *, int);
  Bitmask sqlite3WhereExprUsage(WhereMaskSet *, Expr *);
  Bitmask sqlite3WhereExprUsageNN(WhereMaskSet *, Expr *);
  Bitmask sqlite3WhereExprListUsage(WhereMaskSet *, ExprList *);
  Bitmask exprSelectUsage(WhereMaskSet * pMaskSet, Select * pS);
  __attribute__((noinline)) Bitmask sqlite3WhereExprUsageFull(WhereMaskSet * pMaskSet, Expr * p);
  void createMask(WhereMaskSet * pMaskSet, int iCursor);


