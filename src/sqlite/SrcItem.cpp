#define _GNU_SOURCE 1
#include "sqlite/SrcItem.h"
#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/Select.h"
#include "sqlite/SrcList.h"
#include "sqlite/Subquery.h"
#include "sqlite/Table.h"
#include "sqlite/Window.h"
#include "sqlite/i16.h"
#include "sqlite/sqlite3.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
void sqlite3SrcItemColumnUsed(SrcItem *pItem, int iCol) {
  if (pItem->fg.isNestedFrom) {
    ExprList *pResults;

    pResults = pItem->u4.pSubq->pSelect->pEList;

    pResults->a[iCol].fg.bUsed = 1;
  }
}

int disableUnusedSubqueryResultColumns(SrcItem *pItem) {
  int nCol;
  Select *pSub;
  Select *pX;
  Table *pTab;
  int j;
  int nChng = 0;
  Bitmask colUsed;

  if (pItem->fg.isCorrelated || pItem->fg.isCte) {
    return 0;
  }

  pTab = pItem->pSTab;

  pSub = pItem->u4.pSubq->pSelect;

  for (pX = pSub; pX; pX = pX->pPrior) {
    if ((pX->selFlags & (0x0000001 | 0x0000008)) != 0) {
      return 0;
    }
    if (pX->pPrior && pX->op != 136) {
      return 0;
    }

    if (pX->pWin) {
      return 0;
    }
  }
  colUsed = pItem->colUsed;
  if (pSub->pOrderBy) {
    ExprList *pList = pSub->pOrderBy;
    for (j = 0; j < pList->nExpr; j++) {
      u16 iCol = pList->a[j].u.x.iOrderByCol;
      if (iCol > 0) {
        iCol--;
        colUsed |= ((Bitmask)1) << (iCol >= ((int)(sizeof(Bitmask) * 8)) ? ((int)(sizeof(Bitmask) * 8)) - 1 : iCol);
      }
    }
  }
  nCol = pTab->nCol;
  for (j = 0; j < nCol; j++) {
    Bitmask m = j < ((int)(sizeof(Bitmask) * 8)) - 1 ? (((Bitmask)1) << (j))
                                                     : (((Bitmask)1) << (((int)(sizeof(Bitmask) * 8)) - 1));
    if ((m & colUsed) != 0)
      continue;
    for (pX = pSub; pX; pX = pX->pPrior) {
      Expr *pY = pX->pEList->a[j].pExpr;
      if (pY->op == 122)
        continue;
      pY->op = 122;
      (pY)->flags &= ~(u32)(0x002000 | 0x080000);
      pX->selFlags |= 0x1000000;
      nChng++;
    }
  }
  return nChng;
}

int sameSrcAlias(SrcItem *p0, SrcList *pSrc) {
  int i;
  for (i = 0; i < pSrc->nSrc; i++) {
    SrcItem *p1 = &pSrc->a[i];
    if (p1 == p0)
      continue;
    if (p0->pSTab == p1->pSTab && 0 == sqlite3_stricmp(p0->zAlias, p1->zAlias)) {
      return 1;
    }
    if (p1->fg.isSubquery && (p1->u4.pSubq->pSelect->selFlags & 0x0000800) != 0 &&
        sameSrcAlias(p0, p1->u4.pSubq->pSelect->pSrc)) {
      return 1;
    }
  }
  return 0;
}
