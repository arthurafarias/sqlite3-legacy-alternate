#define _GNU_SOURCE 1

#include "sqlite/NameContext.h"

#include "sqlite/Column.h"
#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/Parse.h"
#include "sqlite/Select.h"
#include "sqlite/SrcItem.h"
#include "sqlite/SrcList.h"
#include "sqlite/Subquery.h"
#include "sqlite/Table.h"
#include "sqlite/Walker.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
#include "sqlite/ynVar.h"
int resolveOrderGroupBy(NameContext *pNC, Select *pSelect, ExprList *pOrderBy, const char *zType) {
  int i, j;
  int iCol;
  struct ExprList_item *pItem;
  Parse *pParse;
  int nResult;

  nResult = pSelect->pEList->nExpr;
  pParse = pNC->pParse;
  for (i = 0, pItem = pOrderBy->a; i < pOrderBy->nExpr; i++, pItem++) {
    Expr *pE = pItem->pExpr;
    Expr *pE2 = sqlite3ExprSkipCollateAndLikely(pE);
    if ((pE2 == 0))
      continue;
    if (zType[0] != 'G') {
      iCol = resolveAsName(pParse, pSelect->pEList, pE2);
      if (iCol > 0) {

        pItem->u.x.iOrderByCol = (u16)iCol;
        continue;
      }
    }
    if (sqlite3ExprIsInteger(pE2, &iCol, 0)) {

      if (iCol < 1 || iCol > 0xffff) {
        resolveOutOfRangeError(pParse, zType, i + 1, nResult, pE2);
        return 1;
      }
      pItem->u.x.iOrderByCol = (u16)iCol;
      continue;
    }

    pItem->u.x.iOrderByCol = 0;
    if (sqlite3ResolveExprNames(pNC, pE)) {
      return 1;
    }
    for (j = 0; j < pSelect->pEList->nExpr; j++) {
      if (sqlite3ExprCompare(0, pE, pSelect->pEList->a[j].pExpr, -1) == 0) {

        windowRemoveExprFromSelect(pSelect, pE);
        pItem->u.x.iOrderByCol = j + 1;
      }
    }
  }
  return sqlite3ResolveOrderGroupBy(pParse, pSelect, pOrderBy, zType);
}

int sqlite3ResolveExprNames(NameContext *pNC, Expr *pExpr) {
  int savedHasAgg;
  Walker w;

  if (pExpr == 0)
    return 0;
  savedHasAgg = pNC->ncFlags & (0x000010 | 0x001000 | 0x008000 | 0x8000000);
  pNC->ncFlags &= ~(0x000010 | 0x001000 | 0x008000 | 0x8000000);
  w.pParse = pNC->pParse;
  w.xExprCallback = resolveExprStep;
  w.xSelectCallback = (pNC->ncFlags & 0x080000) ? 0 : resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;

  w.pParse->nHeight += pExpr->nHeight;
  if (sqlite3ExprCheckHeight(w.pParse, w.pParse->nHeight)) {
    return 1;
  }

  sqlite3WalkExprNN(&w, pExpr);

  w.pParse->nHeight -= pExpr->nHeight;

  ;
  ;
  (pExpr)->flags |= (u32)(pNC->ncFlags & (0x000010 | 0x008000));
  pNC->ncFlags |= savedHasAgg;
  return pNC->nNcErr > 0 || w.pParse->nErr > 0;
}

int sqlite3ResolveExprListNames(NameContext *pNC, ExprList *pList) {
  int i;
  int savedHasAgg = 0;
  Walker w;
  if (pList == 0)
    return 0;
  w.pParse = pNC->pParse;
  w.xExprCallback = resolveExprStep;
  w.xSelectCallback = resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
  savedHasAgg = pNC->ncFlags & (0x000010 | 0x001000 | 0x008000 | 0x8000000);
  pNC->ncFlags &= ~(0x000010 | 0x001000 | 0x008000 | 0x8000000);
  for (i = 0; i < pList->nExpr; i++) {
    Expr *pExpr = pList->a[i].pExpr;
    if (pExpr == 0)
      continue;

    w.pParse->nHeight += pExpr->nHeight;
    if (sqlite3ExprCheckHeight(w.pParse, w.pParse->nHeight)) {
      return 1;
    }

    sqlite3WalkExprNN(&w, pExpr);

    w.pParse->nHeight -= pExpr->nHeight;

    ((void)(0))

        ;

    ((void)(0))

        ;
    ;
    ;
    if (pNC->ncFlags & (0x000010 | 0x001000 | 0x008000 | 0x8000000)) {
      (pExpr)->flags |= (u32)(pNC->ncFlags & (0x000010 | 0x008000));
      savedHasAgg |= pNC->ncFlags & (0x000010 | 0x001000 | 0x008000 | 0x8000000);
      pNC->ncFlags &= ~(0x000010 | 0x001000 | 0x008000 | 0x8000000);
    }
    if (w.pParse->nErr > 0)
      return 1;
  }
  pNC->ncFlags |= savedHasAgg;
  return 0;
}

void sqlite3ExprAnalyzeAggregates(NameContext *pNC, Expr *pExpr) {
  Walker w;
  w.xExprCallback = analyzeAggregate;
  w.xSelectCallback = sqlite3WalkerDepthIncrease;
  w.xSelectCallback2 = sqlite3WalkerDepthDecrease;
  w.walkerDepth = 0;
  w.u.pNC = pNC;
  w.pParse = 0;

  sqlite3WalkExpr(&w, pExpr);
}

void sqlite3ExprAnalyzeAggList(NameContext *pNC, ExprList *pList) {
  struct ExprList_item *pItem;
  int i;
  if (pList) {
    for (pItem = pList->a, i = 0; i < pList->nExpr; i++, pItem++) {
      sqlite3ExprAnalyzeAggregates(pNC, pItem->pExpr);
    }
  }
}

int resolveAttachExpr(NameContext *pName, Expr *pExpr) {
  int rc = 0;
  if (pExpr) {
    if (pExpr->op != 60) {
      rc = sqlite3ResolveExprNames(pName, pExpr);
    } else {
      pExpr->op = 118;
    }
  }
  return rc;
}

const char *columnTypeImpl(NameContext *pNC,

                           Expr *pExpr

) {
  char const *zType = 0;
  int j;

  switch (pExpr->op) {
  case 168: {

    Table *pTab = 0;
    Select *pS = 0;
    int iCol = pExpr->iColumn;
    while (pNC && !pTab) {
      SrcList *pTabList = pNC->pSrcList;
      for (j = 0; j < pTabList->nSrc && pTabList->a[j].iCursor != pExpr->iTable; j++)
        ;
      if (j < pTabList->nSrc) {
        pTab = pTabList->a[j].pSTab;
        if (pTabList->a[j].fg.isSubquery) {
          pS = pTabList->a[j].u4.pSubq->pSelect;
        } else {
          pS = 0;
        }
      } else {
        pNC = pNC->pNext;
      }
    }

    if (pTab == 0) {

      break;
    }

    ((void)(0))

        ;
    if (pS) {

      if (iCol < pS->pEList->nExpr && (!0 || iCol >= 0)) {

        NameContext sNC;
        Expr *p = pS->pEList->a[iCol].pExpr;
        sNC.pSrcList = pS->pSrc;
        sNC.pNext = pNC;
        sNC.pParse = pNC->pParse;
        zType = columnTypeImpl(&sNC, p);
      }
    } else {

      ((void)(0))

          ;

      ((void)(0))

          ;
      if (iCol < 0) {
        zType = "INTEGER";
      } else {
        zType = sqlite3ColumnType(&pTab->aCol[iCol], 0);
      }
    }
    break;
  }

  case 139: {

    NameContext sNC;
    Select *pS;
    Expr *p;

    ((void)(0))

        ;
    pS = pExpr->x.pSelect;
    p = pS->pEList->a[0].pExpr;
    sNC.pSrcList = pS->pSrc;
    sNC.pNext = pNC;
    sNC.pParse = pNC->pParse;
    zType = columnTypeImpl(&sNC, p);
    break;
  }
  }

  return zType;
}
