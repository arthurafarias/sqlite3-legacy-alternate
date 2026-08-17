#define _GNU_SOURCE 1

#include "sqlite/WhereTerm.h"

#include "sqlite/Column.h"
#include "sqlite/Expr.h"
#include "sqlite/LogEst.h"
#include "sqlite/SrcItem.h"
#include "sqlite/Table.h"
#include "sqlite/WhereAndInfo.h"
#include "sqlite/WhereClause.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
WhereTerm *whereNthSubterm(WhereTerm *pTerm, int N) {
  if (pTerm->eOperator != 0x0400) {
    return N == 0 ? pTerm : 0;
  }
  if (N < pTerm->u.pAndInfo->wc.nTerm) {
    return &pTerm->u.pAndInfo->wc.a[N];
  }
  return 0;
}

int constraintCompatibleWithOuterJoin(const WhereTerm *pTerm, const SrcItem *pSrc) {

  ;
  ;

  ;
  if (!(((pTerm->pExpr)->flags & (u32)(0x000001 | 0x000002)) != 0) || pTerm->pExpr->w.iJoin != pSrc->iCursor) {
    return 0;
  }
  if ((pSrc->fg.jointype & (0x08 | 0x10)) != 0 && ((((pTerm->pExpr)->flags & (u32)(0x000002)) != 0))) {
    return 0;
  }
  return 1;
}

int termCanDriveIndex(const WhereTerm *pTerm, const SrcItem *pSrc, const Bitmask notReady) {
  char aff;
  int leftCol;

  if (pTerm->leftCursor != pSrc->iCursor)
    return 0;
  if ((pTerm->eOperator & (0x0002 | 0x0080)) == 0)
    return 0;

  if ((pSrc->fg.jointype & (0x08 | 0x40 | 0x10)) != 0 && !constraintCompatibleWithOuterJoin(pTerm, pSrc)) {
    return 0;
  }
  if ((pTerm->prereqRight & notReady) != 0)
    return 0;

  leftCol = pTerm->u.x.leftColumn;
  if (leftCol < 0)
    return 0;
  aff = pSrc->pSTab->aCol[leftCol].affinity;
  if (!sqlite3IndexAffinityOk(pTerm->pExpr, aff))
    return 0;
  ;
  return columnIsGoodIndexCandidate(pSrc->pSTab, leftCol);
}

LogEst whereRangeAdjust(WhereTerm *pTerm, LogEst nNew) {
  LogEst nRet = nNew;
  if (pTerm) {
    if (pTerm->truthProb <= 0) {
      nRet += pTerm->truthProb;
    } else if ((pTerm->wtFlags & 0x0080) == 0) {
      nRet -= 20;

      ((void)(0))

          ;
    }
  }
  return nRet;
}

int isLimitTerm(WhereTerm *pTerm) { return pTerm->eMatchOp >= 73 && pTerm->eMatchOp <= 74; }
