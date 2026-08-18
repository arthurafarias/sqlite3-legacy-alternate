#define _GNU_SOURCE 1
#include <string.h>
#include "sqlite/AggInfo.h"
#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/NameContext.h"
#include "sqlite/Walker.h"
#include "sqlite/Window.h"
#include "sqlite/u32.h"
void analyzeAggFuncArgs(AggInfo *pAggInfo, NameContext *pNC) {
  int i;

  pNC->ncFlags |= 0x020000;
  for (i = 0; i < pAggInfo->nFunc; i++) {
    Expr *pExpr = pAggInfo->aFunc[i].pFExpr;

    sqlite3ExprAnalyzeAggList(pNC, pExpr->x.pList);
    if (pExpr->pLeft) {
      sqlite3ExprAnalyzeAggList(pNC, pExpr->pLeft->x.pList);
    }

    if ((((pExpr)->flags & (u32)(0x1000000)) != 0)) {
      sqlite3ExprAnalyzeAggregates(pNC, pExpr->y.pWin->pFilter);
    }
  }
  pNC->ncFlags &= ~0x020000;
}

void aggregateConvertIndexedExprRefToColumn(AggInfo *pAggInfo) {
  int i;
  Walker w;
  memset(&w, 0, sizeof(w));
  w.xExprCallback = aggregateIdxEprRefToColCallback;
  for (i = 0; i < pAggInfo->nFunc; i++) {
    sqlite3WalkExpr(&w, pAggInfo->aFunc[i].pFExpr);
  }
}
