#define _GNU_SOURCE 1

#include "sqlite/sqlite3_index_info.h"

#include "sqlite/CollSeq.h"
#include "sqlite/Expr.h"
#include "sqlite/HiddenIndexInfo.h"
#include "sqlite/Parse.h"
#include "sqlite/WhereClause.h"
#include "sqlite/WhereTerm.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_value.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
void freeIdxStr(sqlite3_index_info *pIdxInfo) {
  if (pIdxInfo->needToFreeIdxStr) {
    sqlite3_free(pIdxInfo->idxStr);
    pIdxInfo->idxStr = 0;
    pIdxInfo->needToFreeIdxStr = 0;
  }
}

const char *sqlite3_vtab_collation(sqlite3_index_info *pIdxInfo, int iCons) {
  HiddenIndexInfo *pHidden = (HiddenIndexInfo *)&pIdxInfo[1];
  const char *zRet = 0;
  if (iCons >= 0 && iCons < pIdxInfo->nConstraint) {
    CollSeq *pC = 0;
    int iTerm = pIdxInfo->aConstraint[iCons].iTermOffset;
    Expr *pX = termFromWhereClause(pHidden->pWC, iTerm)->pExpr;
    if (pX->pLeft) {
      pC = sqlite3ExprCompareCollSeq(pHidden->pParse, pX);
    }
    zRet = (pC ? pC->zName : sqlite3StrBINARY);
  }
  return zRet;
}

int sqlite3_vtab_in(sqlite3_index_info *pIdxInfo, int iCons, int bHandle) {
  HiddenIndexInfo *pHidden = (HiddenIndexInfo *)&pIdxInfo[1];
  u32 m = ((iCons) <= 31 ? ((unsigned int)1) << (iCons) : 0);
  if (m & pHidden->mIn) {
    if (bHandle == 0) {
      pHidden->mHandleIn &= ~m;
    } else if (bHandle > 0) {
      pHidden->mHandleIn |= m;
    }
    return 1;
  }
  return 0;
}

int sqlite3_vtab_rhs_value(sqlite3_index_info *pIdxInfo, int iCons, sqlite3_value **ppVal) {
  HiddenIndexInfo *pH = (HiddenIndexInfo *)&pIdxInfo[1];
  sqlite3_value *pVal = 0;
  int rc = 0;
  if (iCons < 0 || iCons >= pIdxInfo->nConstraint) {
    rc = sqlite3MisuseError(173456);
  } else {
    if (pH->aRhs[iCons] == 0) {
      WhereTerm *pTerm = termFromWhereClause(pH->pWC, pIdxInfo->aConstraint[iCons].iTermOffset);
      rc = sqlite3ValueFromExpr(pH->pParse->db, pTerm->pExpr->pRight, ((pH->pParse->db)->enc), 0x41, &pH->aRhs[iCons]);
      ;
    }
    pVal = pH->aRhs[iCons];
  }
  *ppVal = pVal;

  if (rc == 0 && pVal == 0) {
    rc = 12;
  }

  return rc;
}

int sqlite3_vtab_distinct(sqlite3_index_info *pIdxInfo) {
  HiddenIndexInfo *pHidden = (HiddenIndexInfo *)&pIdxInfo[1];

  return pHidden->eDistinct;
}
