#define _GNU_SOURCE 1

#include <string.h>

#include "sqlite/ExprList.h"

#include "sqlite/Expr.h"
#include "sqlite/Parse.h"
#include "sqlite/Select.h"
#include "sqlite/Table.h"
#include "sqlite/Walker.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
__attribute__((noinline)) void resolveSetExprSubtypeArg(ExprList *pList) {
  int nn, ii;
  nn = pList ? pList->nExpr : 0;
  for (ii = 0; ii < nn; ii++) {
    Expr *pExpr = pList->a[ii].pExpr;
    while (1) {
      (pExpr)->flags |= (u32)(0x80000000);
      if (pExpr->op == 139) {




        resolveSetExprSubtypeArg(pExpr->x.pSelect->pEList);
        break;
      }
      if (pExpr->op == 173) {
        pExpr = pExpr->pLeft;


      } else {
        break;
      }
    }
  }
}

void heightOfExprList(const ExprList *p, int *pnHeight) {
  if (p) {
    int i;
    for (i = 0; i < p->nExpr; i++) {
      heightOfExpr(p->a[i].pExpr, pnHeight);
    }
  }
}

void sqlite3ExprListSetSortOrder(ExprList *p, int iSortOrder, int eNulls) {
  struct ExprList_item *pItem;
  if (p == 0)
    return;

  pItem = &p->a[p->nExpr - 1];

  if (iSortOrder == -1) {
    iSortOrder = 0;
  }
  pItem->fg.sortFlags = (u8)iSortOrder;

  if (eNulls != -1) {
    pItem->fg.bNulls = 1;
    if (iSortOrder != eNulls) {
      pItem->fg.sortFlags |= 0x02;
    }
  }
}

u32 sqlite3ExprListFlags(const ExprList *pList) {
  int i;
  u32 m = 0;

  for (i = 0; i < pList->nExpr; i++) {
    Expr *pExpr = pList->a[i].pExpr;


    m |= pExpr->flags;
  }
  return m;
}

int sqlite3ExprListCompare(const ExprList *pA, const ExprList *pB, int iTab) {
  int i;
  if (pA == 0 && pB == 0)
    return 0;
  if (pA == 0 || pB == 0)
    return 1;
  if (pA->nExpr != pB->nExpr)
    return 1;
  for (i = 0; i < pA->nExpr; i++) {
    int res;
    Expr *pExprA = pA->a[i].pExpr;
    Expr *pExprB = pB->a[i].pExpr;
    if (pA->a[i].fg.sortFlags != pB->a[i].fg.sortFlags)
      return 1;
    if ((res = sqlite3ExprCompare(0, pExprA, pExprB, iTab)))
      return res;
  }
  return 0;
}

void renameSetENames(ExprList *pEList, int val) {

  if (pEList) {
    int i;
    for (i = 0; i < pEList->nExpr; i++) {


      pEList->a[i].fg.eEName = val & 0x3;
    }
  }
}

int sqlite3CopySortOrder(ExprList *p1, ExprList *p2) {

  if (p2 && p1->nExpr == p2->nExpr) {
    int ii;
    for (ii = 0; ii < p1->nExpr; ii++) {
      u8 sortFlags;
      sortFlags = p2->a[ii].fg.sortFlags & 0x01;
      p1->a[ii].fg.sortFlags = sortFlags;
    }
    return 1;
  } else {
    return 0;
  }
}

void sqlite3ProcessReturningSubqueries(ExprList *pEList, Table *pTab) {
  Walker w;
  memset(&w, 0, sizeof(w));
  w.xExprCallback = sqlite3ExprWalkNoop;
  w.xSelectCallback = sqlite3ReturningSubqueryCorrelated;
  w.u.pTab = pTab;
  sqlite3WalkExprList(&w, pEList);
  if (w.eCode) {
    w.xExprCallback = sqlite3ReturningSubqueryVarSelect;
    w.xSelectCallback = sqlite3SelectWalkNoop;
    sqlite3WalkExprList(&w, pEList);
  }
}

void adjustOrderByCol(ExprList *pOrderBy, ExprList *pEList) {
  int i, j;
  if (pOrderBy == 0)
    return;
  for (i = 0; i < pOrderBy->nExpr; i++) {
    int t = pOrderBy->a[i].u.x.iOrderByCol;
    if (t == 0)
      continue;
    for (j = 0; j < pEList->nExpr; j++) {
      if (pEList->a[j].u.x.iOrderByCol == t) {
        pOrderBy->a[i].u.x.iOrderByCol = j + 1;
        break;
      }
    }
    if (j >= pEList->nExpr) {
      pOrderBy->a[i].u.x.iOrderByCol = 0;
    }
  }
}
