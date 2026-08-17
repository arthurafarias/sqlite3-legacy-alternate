#define _GNU_SOURCE 1

#include <string.h>

#include "sqlite/SubstContext.h"

#include "sqlite/CollSeq.h"
#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/Parse.h"
#include "sqlite/Select.h"
#include "sqlite/SrcItem.h"
#include "sqlite/SrcList.h"
#include "sqlite/Subquery.h"
#include "sqlite/Window.h"
#include "sqlite/sqlite3.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
#include "sqlite/ynVar.h"
Expr *substExpr(SubstContext *pSubst, Expr *pExpr) {
  if (pExpr == 0)
    return 0;
  if ((((pExpr)->flags & (u32)(0x000001 | 0x000002)) != 0) && pExpr->w.iJoin == pSubst->iTable) {
    ;
    pExpr->w.iJoin = pSubst->iNewTable;
  }
  if (pExpr->op == 168 && pExpr->iTable == pSubst->iTable && !(((pExpr)->flags & (u32)(0x000020)) != 0)) {

    {
      Expr *pNew;
      int iColumn;
      Expr *pCopy;
      Expr ifNullRow;
      iColumn = pExpr->iColumn;






      pCopy = pSubst->pEList->a[iColumn].pExpr;
      if (sqlite3ExprIsVector(pCopy)) {
        sqlite3VectorErrorMsg(pSubst->pParse, pCopy);
      } else {
        sqlite3 *db = pSubst->pParse->db;
        if (pSubst->isOuterJoin && (pCopy->op != 168 || pCopy->iTable != pSubst->iNewTable)) {
          memset(&ifNullRow, 0, sizeof(ifNullRow));
          ifNullRow.op = 179;
          ifNullRow.pLeft = pCopy;
          ifNullRow.iTable = pSubst->iNewTable;
          ifNullRow.iColumn = -99;
          ifNullRow.flags = 0x040000;
          pCopy = &ifNullRow;
        };
        pNew = sqlite3ExprDup(db, pCopy, 0);
        if (db->mallocFailed) {
          sqlite3ExprDelete(db, pNew);
          return pExpr;
        }
        if (pSubst->isOuterJoin) {
          (pNew)->flags |= (u32)(0x200000);
        }
        if (pNew->op == 171) {
          pNew->u.iValue = sqlite3ExprTruthValue(pNew);
          pNew->op = 156;
          (pNew)->flags |= (u32)(0x000800);
        }

        {
          CollSeq *pNat = sqlite3ExprCollSeq(pSubst->pParse, pNew);
          CollSeq *pColl = sqlite3ExprCollSeq(pSubst->pParse, pSubst->pCList->a[iColumn].pExpr);
          if (pNat != pColl || (pNew->op != 168 && pNew->op != 114)) {
            pNew = sqlite3ExprAddCollateString(pSubst->pParse, pNew, (pColl ? pColl->zName : "BINARY"));
          }
        }
        (pNew)->flags &= ~(u32)(0x000200);
        if ((((pExpr)->flags & (u32)(0x000001 | 0x000002)) != 0)) {
          sqlite3SetJoinExpr(pNew, pExpr->w.iJoin, pExpr->flags & (0x000001 | 0x000002));
        }
        sqlite3ExprDelete(db, pExpr);
        pExpr = pNew;
      }
    }
  } else {
    if (pExpr->op == 179 && pExpr->iTable == pSubst->iTable) {
      pExpr->iTable = pSubst->iNewTable;
    }
    if (pExpr->op == 169 && pExpr->op2 >= pSubst->nSelDepth) {
      pExpr->op2--;
    }
    pExpr->pLeft = substExpr(pSubst, pExpr->pLeft);
    pExpr->pRight = substExpr(pSubst, pExpr->pRight);
    if ((((pExpr)->flags & 0x001000) != 0)) {
      substSelect(pSubst, pExpr->x.pSelect, 1);
    } else {
      substExprList(pSubst, pExpr->x.pList);
    }

    if ((((pExpr)->flags & (u32)(0x1000000)) != 0)) {
      Window *pWin = pExpr->y.pWin;
      pWin->pFilter = substExpr(pSubst, pWin->pFilter);
      substExprList(pSubst, pWin->pPartition);
      substExprList(pSubst, pWin->pOrderBy);
    }
  }
  return pExpr;
}

void substExprList(SubstContext *pSubst, ExprList *pList) {
  int i;
  if (pList == 0)
    return;
  for (i = 0; i < pList->nExpr; i++) {
    pList->a[i].pExpr = substExpr(pSubst, pList->a[i].pExpr);
  }
}

void substSelect(SubstContext *pSubst, Select *p, int doPrior) {
  SrcList *pSrc;
  SrcItem *pItem;
  int i;
  if (!p)
    return;
  pSubst->nSelDepth++;
  do {
    substExprList(pSubst, p->pEList);
    substExprList(pSubst, p->pGroupBy);
    substExprList(pSubst, p->pOrderBy);
    p->pHaving = substExpr(pSubst, p->pHaving);
    p->pWhere = substExpr(pSubst, p->pWhere);
    pSrc = p->pSrc;


    for (i = pSrc->nSrc, pItem = pSrc->a; i > 0; i--, pItem++) {
      if (pItem->fg.isSubquery) {
        substSelect(pSubst, pItem->u4.pSubq->pSelect, 1);
      }
      if (pItem->fg.isTabFunc) {
        substExprList(pSubst, pItem->u1.pFuncArg);
      }
    }
  } while (doPrior && (p = p->pPrior) != 0);
  pSubst->nSelDepth--;
}
