
#pragma once

#include "sqlite/ExprList.h"
#include "sqlite/Parse.h"

struct SubstContext;

struct SubstContext {
  Parse *pParse;
  int iTable;
  int iNewTable;
  int isOuterJoin;
  int nSelDepth;
  ExprList *pEList;
  ExprList *pCList;
};

void substExprList(SubstContext *, ExprList *);
void substSelect(SubstContext *, Select *, int);
Expr *substExpr(SubstContext *pSubst, Expr *pExpr);


