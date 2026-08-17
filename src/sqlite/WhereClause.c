#define _GNU_SOURCE 1

#include <string.h>

#include "sqlite/WhereClause.h"

#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/Index.h"
#include "sqlite/LogEst.h"
#include "sqlite/Parse.h"
#include "sqlite/Select.h"
#include "sqlite/SrcItem.h"
#include "sqlite/SrcList.h"
#include "sqlite/Table.h"
#include "sqlite/Token.h"
#include "sqlite/WhereAndInfo.h"
#include "sqlite/WhereInfo.h"
#include "sqlite/WhereLoop.h"
#include "sqlite/WhereOrInfo.h"
#include "sqlite/WhereScan.h"
#include "sqlite/WhereTerm.h"
#include "sqlite/sqlite3.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
int whereClauseInsert(WhereClause *pWC, Expr *p, u16 wtFlags) {
  WhereTerm *pTerm;
  int idx;
  ;
  if (pWC->nTerm >= pWC->nSlot) {
    WhereTerm *pOld = pWC->a;
    sqlite3 *db = pWC->pWInfo->pParse->db;
    pWC->a = sqlite3WhereMalloc(pWC->pWInfo, sizeof(pWC->a[0]) * pWC->nSlot * 2);
    if (pWC->a == 0) {
      if (wtFlags & 0x0001) {
        sqlite3ExprDelete(db, p);
      }
      pWC->a = pOld;
      return 0;
    }
    memcpy(pWC->a, pOld, sizeof(pWC->a[0]) * pWC->nTerm);
    pWC->nSlot = pWC->nSlot * 2;
  }
  pTerm = &pWC->a[idx = pWC->nTerm++];
  if ((wtFlags & 0x0002) == 0)
    pWC->nBase = pWC->nTerm;
  if (p && (((p)->flags & (u32)(0x080000)) != 0)) {
    pTerm->truthProb = sqlite3LogEst(p->iTable) - 270;
  } else {
    pTerm->truthProb = 1;
  }
  pTerm->pExpr = sqlite3ExprSkipCollateAndLikely(p);
  pTerm->wtFlags = wtFlags;
  pTerm->pWC = pWC;
  pTerm->iParent = -1;
  memset(&pTerm->eOperator, 0,
         sizeof(WhereTerm) -

             __builtin_offsetof(

                 WhereTerm

                 ,

                 eOperator

                 )

  );
  return idx;
}

void markTermAsChild(WhereClause *pWC, int iChild, int iParent) {
  pWC->a[iChild].iParent = iParent;
  pWC->a[iChild].truthProb = pWC->a[iParent].truthProb;

  pWC->a[iParent].nChild++;
  ;
}

void sqlite3WhereSplit(WhereClause *pWC, Expr *pExpr, u8 op) {
  Expr *pE2 = sqlite3ExprSkipCollateAndLikely(pExpr);
  pWC->op = op;

  if (pE2 == 0)
    return;
  if (pE2->op != op) {
    whereClauseInsert(pWC, pExpr, 0);
  } else {
    sqlite3WhereSplit(pWC, pE2->pLeft, op);
    sqlite3WhereSplit(pWC, pE2->pRight, op);
  }
}

void whereAddLimitExpr(WhereClause *pWC, int iReg, Expr *pExpr, int iCsr, int eMatchOp) {
  Parse *pParse = pWC->pWInfo->pParse;
  sqlite3 *db = pParse->db;
  Expr *pNew;
  int iVal = 0;

  if (sqlite3ExprIsInteger(pExpr, &iVal, pParse) && iVal >= 0) {
    Expr *pVal = sqlite3ExprInt32(db, iVal);
    if (pVal == 0)
      return;
    pNew = sqlite3PExpr(pParse, 47, 0, pVal);
  } else {
    Expr *pVal = sqlite3ExprAlloc(db, 176, 0, 0);
    if (pVal == 0)
      return;
    pVal->iTable = iReg;
    pNew = sqlite3PExpr(pParse, 47, 0, pVal);
  }
  if (pNew) {
    WhereTerm *pTerm;
    int idx;
    idx = whereClauseInsert(pWC, pNew, 0x0001 | 0x0002);
    pTerm = &pWC->a[idx];
    pTerm->leftCursor = iCsr;
    pTerm->eOperator = 0x0040;
    pTerm->eMatchOp = eMatchOp;
  }
}

void __attribute__((noinline)) sqlite3WhereAddLimit(WhereClause *pWC, Select *p) {

  if (p->pGroupBy == 0 && (p->selFlags & (0x0000001 | 0x0000008)) == 0 && (p->pSrc->nSrc == 1 && ((p->pSrc->a[0].pSTab)->eTabType == 1))) {
    ExprList *pOrderBy = p->pOrderBy;
    int iCsr = p->pSrc->a[0].iCursor;
    int ii;

    for (ii = 0; ii < pWC->nTerm; ii++) {
      if (pWC->a[ii].wtFlags & 0x0004) {




        continue;
      }
      if (pWC->a[ii].nChild) {

        continue;
      }
      if (pWC->a[ii].leftCursor == iCsr && pWC->a[ii].prereqRight == 0)
        continue;

      if (pWC->a[ii].iParent >= 0) {
        WhereTerm *pParent = &pWC->a[pWC->a[ii].iParent];
        if (pParent->leftCursor == iCsr && pParent->prereqRight == 0 && pParent->nChild == 1) {
          continue;
        }
      }

      return;
    }

    if (pOrderBy) {
      for (ii = 0; ii < pOrderBy->nExpr; ii++) {
        Expr *pExpr = pOrderBy->a[ii].pExpr;
        if (pExpr->op != 168)
          return;
        if (pExpr->iTable != iCsr)
          return;
        if (pOrderBy->a[ii].fg.sortFlags & 0x02)
          return;
      }
    }


    if (p->iOffset != 0 && (p->selFlags & 0x0000100) == 0) {
      whereAddLimitExpr(pWC, p->iOffset, p->pLimit->pRight, iCsr, 74);
    }
    if (p->iOffset == 0 || (p->selFlags & 0x0000100) == 0) {
      whereAddLimitExpr(pWC, p->iLimit, p->pLimit->pLeft, iCsr, 73);
    }
  }
}

void sqlite3WhereClauseInit(WhereClause *pWC, WhereInfo *pWInfo) {
  pWC->pWInfo = pWInfo;
  pWC->hasOr = 0;
  pWC->pOuter = 0;
  pWC->nTerm = 0;
  pWC->nBase = 0;
  pWC->nSlot = ((int)(sizeof(pWC->aStatic) / sizeof(pWC->aStatic[0])));
  pWC->a = pWC->aStatic;
}

void sqlite3WhereClauseClear(WhereClause *pWC) {
  sqlite3 *db = pWC->pWInfo->pParse->db;

  if (pWC->nTerm > 0) {
    WhereTerm *a = pWC->a;
    WhereTerm *aLast = &pWC->a[pWC->nTerm - 1];

    while (1) {


      if (a->wtFlags & 0x0001) {
        sqlite3ExprDelete(db, a->pExpr);
      }
      if (a->wtFlags & (0x0010 | 0x0020)) {
        if (a->wtFlags & 0x0010) {


          whereOrInfoDelete(db, a->u.pOrInfo);
        } else {


          whereAndInfoDelete(db, a->u.pAndInfo);
        }
      }
      if (a == aLast)
        break;
      a++;
    }
  }
}

WhereTerm *sqlite3WhereFindTerm(WhereClause *pWC, int iCur, int iColumn, Bitmask notReady, u32 op, Index *pIdx) {
  WhereTerm *pResult = 0;
  WhereTerm *p;
  WhereScan scan;

  p = whereScanInit(&scan, pWC, iCur, iColumn, op, pIdx);
  op &= 0x0002 | 0x0080;
  while (p) {
    if ((p->prereqRight & notReady) == 0) {
      if (p->prereqRight == 0 && (p->eOperator & op) != 0) {
        ;
        return p;
      }
      if (pResult == 0)
        pResult = p;
    }
    p = whereScanNext(&scan);
  }
  return pResult;
}

WhereTerm *termFromWhereClause(WhereClause *pWC, int iTerm) {
  WhereClause *p;
  for (p = pWC; p; p = p->pOuter) {
    if (iTerm < p->nTerm)
      return &p->a[iTerm];
    iTerm -= p->nTerm;
  }
  return 0;
}

void whereLoopOutputAdjust(WhereClause *pWC, WhereLoop *pLoop, LogEst nRow) {
  WhereTerm *pTerm, *pX;
  Bitmask notAllowed = ~(pLoop->prereq | pLoop->maskSelf);
  int i, j;
  LogEst iReduce = 0;

  for (i = pWC->nBase, pTerm = pWC->a; i > 0; i--, pTerm++) {


    if ((pTerm->prereqAll & notAllowed) != 0)
      continue;
    if ((pTerm->prereqAll & pLoop->maskSelf) == 0)
      continue;
    if ((pTerm->wtFlags & 0x0002) != 0)
      continue;
    for (j = pLoop->nLTerm - 1; j >= 0; j--) {
      pX = pLoop->aLTerm[j];
      if (pX == 0)
        continue;
      if (pX == pTerm)
        break;
      if (pX->iParent >= 0 && (&pWC->a[pX->iParent]) == pTerm)
        break;
    }
    if (j < 0) {
      sqlite3ProgressCheck(pWC->pWInfo->pParse);
      if (pLoop->maskSelf == pTerm->prereqAll) {

        if ((pTerm->eOperator & 0x3f) != 0 || (pWC->pWInfo->pTabList->a[pLoop->iTab].fg.jointype & (0x08 | 0x40)) == 0) {
          pLoop->wsFlags |= 0x00800000;
        }
      }
      if (pTerm->truthProb <= 0) {

        pLoop->nOut += pTerm->truthProb;
      } else {

        Expr *pOpExpr = pTerm->pExpr;
        pLoop->nOut--;
        if ((pTerm->eOperator & (0x0002 | 0x0080)) != 0 && (pTerm->wtFlags & 0) == 0) {
          Expr *pRight = pOpExpr->pRight;
          int k = 0;
          ;
          if (sqlite3ExprIsInteger(pRight, &k, 0) && k >= (-1) && k <= 1) {
            k = 10;
          } else {
            k = 20;
          }
          if (iReduce < k) {
            pTerm->wtFlags |= 0x2000;
            iReduce = k;
          }
        } else if ((((pOpExpr)->flags & (u32)(0x000100)) != 0) && pOpExpr->op == 172) {
          int eOp;




          eOp = sqlite3ExprIsLikeOperator(pOpExpr);
          if ((eOp > 0)) {
            int szPattern;
            Expr *pRHS = pOpExpr->x.pList->a[0].pExpr;
            eOp = eOp == 65;
            szPattern = estLikePatternLength(pRHS, eOp);
            if (szPattern > 0) {
              pLoop->nOut -= szPattern * 2;
            }
          }
        }
      }
    }
  }
  if (pLoop->nOut > nRow - iReduce) {
    pLoop->nOut = nRow - iReduce;
  }
}
