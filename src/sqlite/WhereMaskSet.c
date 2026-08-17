#define _GNU_SOURCE 1

#include "sqlite/WhereMaskSet.h"

#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/Select.h"
#include "sqlite/SrcItem.h"
#include "sqlite/SrcList.h"
#include "sqlite/Subquery.h"
#include "sqlite/Window.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
Bitmask exprSelectUsage(WhereMaskSet *pMaskSet, Select *pS) {
  Bitmask mask = 0;
  while (pS) {
    SrcList *pSrc = pS->pSrc;
    mask |= sqlite3WhereExprListUsage(pMaskSet, pS->pEList);
    mask |= sqlite3WhereExprListUsage(pMaskSet, pS->pGroupBy);
    mask |= sqlite3WhereExprListUsage(pMaskSet, pS->pOrderBy);
    mask |= sqlite3WhereExprUsage(pMaskSet, pS->pWhere);
    mask |= sqlite3WhereExprUsage(pMaskSet, pS->pHaving);
    if ((pSrc != 0)) {
      int i;
      for (i = 0; i < pSrc->nSrc; i++) {
        if (pSrc->a[i].fg.isSubquery) {
          mask |= exprSelectUsage(pMaskSet, pSrc->a[i].u4.pSubq->pSelect);
        }
        if (pSrc->a[i].fg.isUsing == 0) {
          mask |= sqlite3WhereExprUsage(pMaskSet, pSrc->a[i].u3.pOn);
        }
        if (pSrc->a[i].fg.isTabFunc) {
          mask |= sqlite3WhereExprListUsage(pMaskSet, pSrc->a[i].u1.pFuncArg);
        }
      }
    }
    pS = pS->pPrior;
  }
  return mask;
}

__attribute__((noinline)) Bitmask sqlite3WhereExprUsageFull(WhereMaskSet *pMaskSet, Expr *p) {
  Bitmask mask;
  mask = (p->op == 179) ? sqlite3WhereGetMask(pMaskSet, p->iTable) : 0;
  if (p->pLeft)
    mask |= sqlite3WhereExprUsageNN(pMaskSet, p->pLeft);
  if (p->pRight) {
    mask |= sqlite3WhereExprUsageNN(pMaskSet, p->pRight);


  } else if ((((p)->flags & 0x001000) != 0)) {
    if ((((p)->flags & (u32)(0x000040)) != 0))
      pMaskSet->bVarSelect = 1;
    mask |= exprSelectUsage(pMaskSet, p->x.pSelect);
  } else if (p->x.pList) {
    mask |= sqlite3WhereExprListUsage(pMaskSet, p->x.pList);
  }

  if ((p->op == 172 || p->op == 169) && (((p)->flags & 0x1000000) != 0)) {


    mask |= sqlite3WhereExprListUsage(pMaskSet, p->y.pWin->pPartition);
    mask |= sqlite3WhereExprListUsage(pMaskSet, p->y.pWin->pOrderBy);
    mask |= sqlite3WhereExprUsage(pMaskSet, p->y.pWin->pFilter);
  }

  return mask;
}

Bitmask sqlite3WhereExprUsageNN(WhereMaskSet *pMaskSet, Expr *p) {
  if (p->op == 168 && !(((p)->flags & (u32)(0x000020)) != 0)) {
    return sqlite3WhereGetMask(pMaskSet, p->iTable);
  } else if ((((p)->flags & (u32)(0x010000 | 0x800000)) != 0)) {


    return 0;
  }
  return sqlite3WhereExprUsageFull(pMaskSet, p);
}

Bitmask sqlite3WhereExprUsage(WhereMaskSet *pMaskSet, Expr *p) { return p ? sqlite3WhereExprUsageNN(pMaskSet, p) : 0; }

Bitmask sqlite3WhereExprListUsage(WhereMaskSet *pMaskSet, ExprList *pList) {
  int i;
  Bitmask mask = 0;
  if (pList) {
    for (i = 0; i < pList->nExpr; i++) {
      mask |= sqlite3WhereExprUsage(pMaskSet, pList->a[i].pExpr);
    }
  }
  return mask;
}

Bitmask sqlite3WhereGetMask(WhereMaskSet *pMaskSet, int iCursor) {
  int i;

  if (pMaskSet->ix[0] == iCursor) {
    return 1;
  }
  for (i = 1; i < pMaskSet->n; i++) {
    if (pMaskSet->ix[i] == iCursor) {
      return (((Bitmask)1) << (i));
    }
  }
  return 0;
}

void createMask(WhereMaskSet *pMaskSet, int iCursor) { pMaskSet->ix[pMaskSet->n++] = iCursor; }
