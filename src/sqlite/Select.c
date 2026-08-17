#define _GNU_SOURCE 1

#include <string.h>

#include "sqlite/Select.h"

#include "sqlite/AggInfo.h"
#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/FuncDef.h"
#include "sqlite/Parse.h"
#include "sqlite/SrcItem.h"
#include "sqlite/SrcList.h"
#include "sqlite/Table.h"
#include "sqlite/Walker.h"
#include "sqlite/Window.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
void windowRemoveExprFromSelect(Select *pSelect, Expr *pExpr) {
  if (pSelect->pWin) {
    Walker sWalker;
    memset(&sWalker, 0, sizeof(Walker));
    sWalker.xExprCallback = resolveRemoveWindowsCb;
    sWalker.u.pSelect = pSelect;
    sqlite3WalkExpr(&sWalker, pExpr);
  }
}

void heightOfSelect(const Select *pSelect, int *pnHeight) {
  const Select *p;
  for (p = pSelect; p; p = p->pPrior) {
    heightOfExpr(p->pWhere, pnHeight);
    heightOfExpr(p->pHaving, pnHeight);
    heightOfExpr(p->pLimit, pnHeight);
    heightOfExprList(p->pEList, pnHeight);
    heightOfExprList(p->pGroupBy, pnHeight);
    heightOfExprList(p->pOrderBy, pnHeight);
  }
}

int sqlite3SelectExprHeight(const Select *p) {
  int nHeight = 0;
  heightOfSelect(p, &nHeight);
  return nHeight;
}

void gatherSelectWindows(Select *p) {
  Walker w;
  w.xExprCallback = gatherSelectWindowsCallback;
  w.xSelectCallback = gatherSelectWindowsSelectCallback;
  w.xSelectCallback2 = 0;
  w.pParse = 0;
  w.u.pSelect = p;
  sqlite3WalkSelect(&w, p);
}

Select *findRightmost(Select *p) {
  while (p->pNext)
    p = p->pNext;
  return p;
}

const char *sqlite3SelectOpName(int id) {
  char *z;
  switch (id) {
  case 136:
    z = "UNION ALL";
    break;
  case 138:
    z = "INTERSECT";
    break;
  case 137:
    z = "EXCEPT";
    break;
  default:
    z = "UNION";
    break;
  }
  return z;
}

int hasAnchor(Select *p) {
  while (p && (p->selFlags & 0x0002000) != 0) {
    p = p->pPrior;
  }
  return p != 0;
}

void recomputeColumnsUsed(Select *pSelect, SrcItem *pSrcItem) {
  Walker w;
  if ((pSrcItem->pSTab == 0))
    return;
  memset(&w, 0, sizeof(w));
  w.xExprCallback = recomputeColumnsUsedExpr;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.pSrcItem = pSrcItem;
  pSrcItem->colUsed = 0;
  sqlite3WalkSelect(&w, pSelect);
}

ExprList *findLeftmostExprlist(Select *pSel) {
  while (pSel->pPrior) {
    pSel = pSel->pPrior;
  }
  return pSel->pEList;
}

int compoundHasDifferentAffinities(Select *p) {
  int ii;
  ExprList *pList;

  pList = p->pEList;
  for (ii = 0; ii < pList->nExpr; ii++) {
    char aff;
    Select *pSub1;

    ((void)(0))

        ;
    aff = sqlite3ExprAffinity(pList->a[ii].pExpr);
    for (pSub1 = p->pPrior; pSub1; pSub1 = pSub1->pPrior) {

      ((void)(0))

          ;

      ((void)(0))

          ;

      ((void)(0))

          ;
      if (sqlite3ExprAffinity(pSub1->pEList->a[ii].pExpr) != aff) {
        return 1;
      }
    }
  }
  return 0;
}

Table *isSimpleCount(Select *p, AggInfo *pAggInfo) {
  Table *pTab;
  Expr *pExpr;

  if (p->pWhere || p->pEList->nExpr != 1 || p->pSrc->nSrc != 1 || p->pSrc->a[0].fg.isSubquery || pAggInfo->nFunc != 1 || p->pHaving) {
    return 0;
  }
  pTab = p->pSrc->a[0].pSTab;

  if (!((pTab)->eTabType == 0))
    return 0;
  pExpr = p->pEList->a[0].pExpr;

  if (pExpr->op != 169)
    return 0;
  if (pExpr->pAggInfo != pAggInfo)
    return 0;
  if ((pAggInfo->aFunc[0].pFunc->funcFlags & 0x0100) == 0)
    return 0;

  ;
  ;
  if ((((pExpr)->flags & (u32)(0x000004 | 0x1000000)) != 0))
    return 0;

  return pTab;
}

void sqlite3WindowLink(Select *pSel, Window *pWin) {
  if (pSel) {
    if (0 == pSel->pWin || 0 == sqlite3WindowCompare(0, pSel->pWin, pWin, 0)) {
      pWin->pNextWin = pSel->pWin;
      if (pSel->pWin) {
        pSel->pWin->ppThis = &pWin->pNextWin;
      }
      pSel->pWin = pWin;
      pWin->ppThis = &pSel->pWin;
    } else {
      if (sqlite3ExprListCompare(pWin->pPartition, pSel->pWin->pPartition, -1)) {
        pSel->selFlags |= 0x2000000;
      }
    }
  }
}
