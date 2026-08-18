#define _GNU_SOURCE 1
#include "sqlite/WhereScan.h"
#include "sqlite/CollSeq.h"
#include "sqlite/Column.h"
#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/Index.h"
#include "sqlite/Parse.h"
#include "sqlite/Table.h"
#include "sqlite/WhereClause.h"
#include "sqlite/WhereInfo.h"
#include "sqlite/WhereTerm.h"
#include "sqlite/i16.h"
#include "sqlite/sqlite3.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
#include "sqlite/ynVar.h"
WhereTerm *whereScanNext(WhereScan *pScan) {
  int iCur;
  i16 iColumn;
  Expr *pX;
  WhereClause *pWC;
  WhereTerm *pTerm;
  int k = pScan->k;

  pWC = pScan->pWC;
  while (1) {
    iColumn = pScan->aiColumn[pScan->iEquiv - 1];
    iCur = pScan->aiCur[pScan->iEquiv - 1];

    do {
      for (pTerm = pWC->a + k; k < pWC->nTerm; k++, pTerm++) {
        if (pTerm->leftCursor == iCur && pTerm->u.x.leftColumn == iColumn &&
            (iColumn != (-2) || sqlite3ExprCompareSkip(pTerm->pExpr->pLeft, pScan->pIdxExpr, iCur) == 0) &&
            (pScan->iEquiv <= 1 || !(((pTerm->pExpr)->flags & (u32)(0x000001)) != 0))) {
          if ((pTerm->eOperator & 0x0800) != 0 &&
              pScan->nEquiv < ((int)(sizeof(pScan->aiCur) / sizeof(pScan->aiCur[0]))) &&
              (pX = whereRightSubexprIsColumn(pTerm->pExpr)) != 0) {
            int j;
            for (j = 0; j < pScan->nEquiv; j++) {
              if (pScan->aiCur[j] == pX->iTable && pScan->aiColumn[j] == pX->iColumn) {
                break;
              }
            }
            if (j == pScan->nEquiv) {
              pScan->aiCur[j] = pX->iTable;
              pScan->aiColumn[j] = pX->iColumn;
              pScan->nEquiv++;
            }
          }
          if ((pTerm->eOperator & pScan->opMask) != 0) {
            if (pScan->zCollName && (pTerm->eOperator & 0x0100) == 0) {
              const char *zCollName;
              Parse *pParse = pWC->pWInfo->pParse;
              pX = pTerm->pExpr;

              if ((pTerm->eOperator & 0x0001)) {
                zCollName = indexInAffinityOk(pParse, pTerm, pScan->idxaff);
                if (!zCollName)
                  continue;
              } else {
                CollSeq *pColl;
                if (!sqlite3IndexAffinityOk(pX, pScan->idxaff)) {
                  continue;
                }

                pColl = sqlite3ExprCompareCollSeq(pParse, pX);
                zCollName = pColl ? pColl->zName : sqlite3StrBINARY;
              }

              if (sqlite3StrICmp(zCollName, pScan->zCollName)) {
                continue;
              }
            }
            if ((pTerm->eOperator & (0x0002 | 0x0080)) != 0 && (pX = pTerm->pExpr->pRight, (pX != 0)) &&
                pX->op == 168 && pX->iTable == pScan->aiCur[0] && pX->iColumn == pScan->aiColumn[0]) {
              continue;
            }
            pScan->pWC = pWC;
            pScan->k = k + 1;

            return pTerm;
          }
        }
      }
      pWC = pWC->pOuter;
      k = 0;
    } while (pWC != 0);
    if (pScan->iEquiv >= pScan->nEquiv)
      break;
    pWC = pScan->pOrigWC;
    k = 0;
    pScan->iEquiv++;
  }
  return 0;
}

__attribute__((noinline)) WhereTerm *whereScanInitIndexExpr(WhereScan *pScan) {
  pScan->idxaff = sqlite3ExprAffinity(pScan->pIdxExpr);
  return whereScanNext(pScan);
}

WhereTerm *whereScanInit(WhereScan *pScan, WhereClause *pWC, int iCur, int iColumn, u32 opMask, Index *pIdx) {
  pScan->pOrigWC = pWC;
  pScan->pWC = pWC;
  pScan->pIdxExpr = 0;
  pScan->idxaff = 0;
  pScan->zCollName = 0;
  pScan->opMask = opMask;
  pScan->k = 0;
  pScan->aiCur[0] = iCur;
  pScan->nEquiv = 1;
  pScan->iEquiv = 1;
  if (pIdx) {
    int j = iColumn;
    iColumn = pIdx->aiColumn[j];
    if (iColumn == pIdx->pTable->iPKey) {
      iColumn = (-1);
    } else if (iColumn >= 0) {
      pScan->idxaff = pIdx->pTable->aCol[iColumn].affinity;
      pScan->zCollName = pIdx->azColl[j];
    } else if (iColumn == (-2)) {
      pScan->pIdxExpr = pIdx->aColExpr->a[j].pExpr;
      pScan->zCollName = pIdx->azColl[j];
      pScan->aiColumn[0] = (-2);
      return whereScanInitIndexExpr(pScan);
    }
  } else if (iColumn == (-2)) {
    return 0;
  }
  pScan->aiColumn[0] = iColumn;
  return whereScanNext(pScan);
}
