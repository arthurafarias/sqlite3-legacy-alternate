
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "sqlite/ExprList.h"
#include "sqlite/Parse.h"
typedef struct Expr Expr;
typedef struct Select Select;

typedef struct SubstContext SubstContext;

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

#ifdef __cplusplus
}
#endif
