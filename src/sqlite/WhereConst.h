
#pragma once

#include "sqlite/u32.h"
#include "sqlite/u8.h"
  typedef struct Expr Expr;
  typedef struct Parse Parse;
  typedef struct WhereConst WhereConst;
  struct WhereConst {
    Parse *pParse;
    u8 *pOomFault;
    int nConst;
    int nChng;
    int bHasAffBlob;
    u32 mExcludeOn;

    Expr **apExpr;
  };

  void constInsert(WhereConst * pConst, Expr * pColumn, Expr * pValue, Expr * pExpr);
  void findConstInWhere(WhereConst * pConst, Expr * pExpr);
  int propagateConstantExprRewriteOne(WhereConst * pConst, Expr * pExpr, int bIgnoreAffBlob);


