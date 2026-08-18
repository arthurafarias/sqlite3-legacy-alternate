#define _GNU_SOURCE 1
#include "sqlite/WhereConst.h"
#include "sqlite/CollSeq.h"
#include "sqlite/Expr.h"
#include "sqlite/Parse.h"
#include "sqlite/sqlite3.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/ynVar.h"
void constInsert(WhereConst *pConst, Expr *pColumn, Expr *pValue, Expr *pExpr) {
  int i;

  if ((((pColumn)->flags & (u32)(0x000020)) != 0))
    return;
  if (sqlite3ExprAffinity(pValue) != 0)
    return;
  if (!sqlite3IsBinary(sqlite3ExprCompareCollSeq(pConst->pParse, pExpr))) {
    return;
  }

  for (i = 0; i < pConst->nConst; i++) {
    const Expr *pE2 = pConst->apExpr[i * 2];

    if (pE2->iTable == pColumn->iTable && pE2->iColumn == pColumn->iColumn) {
      return;
    }
  }

  if (sqlite3ExprAffinity(pColumn) <= 0x41) {
    pConst->bHasAffBlob = 1;
  }

  pConst->nConst++;
  pConst->apExpr = (Expr**)(sqlite3DbReallocOrFree(pConst->pParse->db, pConst->apExpr, pConst->nConst * 2 * sizeof(Expr *)));
  if (pConst->apExpr == 0) {
    pConst->nConst = 0;
  } else {
    pConst->apExpr[pConst->nConst * 2 - 2] = pColumn;
    pConst->apExpr[pConst->nConst * 2 - 1] = pValue;
  }
}

void findConstInWhere(WhereConst *pConst, Expr *pExpr) {
  Expr *pRight, *pLeft;
  if (pExpr == 0)
    return;
  if ((((pExpr)->flags & (u32)(pConst->mExcludeOn)) != 0)) {
    return;
  }
  if (pExpr->op == 44) {
    findConstInWhere(pConst, pExpr->pRight);
    findConstInWhere(pConst, pExpr->pLeft);
    return;
  }
  if (pExpr->op != 54)
    return;
  pRight = pExpr->pRight;
  pLeft = pExpr->pLeft;

  if (pRight->op == 168 && sqlite3ExprIsConstant(pConst->pParse, pLeft)) {
    constInsert(pConst, pRight, pLeft, pExpr);
  }
  if (pLeft->op == 168 && sqlite3ExprIsConstant(pConst->pParse, pRight)) {
    constInsert(pConst, pLeft, pRight, pExpr);
  }
}

int propagateConstantExprRewriteOne(WhereConst *pConst, Expr *pExpr, int bIgnoreAffBlob) {
  int i;
  if (pConst->pOomFault[0])
    return 1;
  if (pExpr->op != 168)
    return 0;
  if ((((pExpr)->flags & (u32)(0x000020 | pConst->mExcludeOn)) != 0)) {
    return 0;
  }
  for (i = 0; i < pConst->nConst; i++) {
    Expr *pColumn = pConst->apExpr[i * 2];
    if (pColumn == pExpr)
      continue;
    if (pColumn->iTable != pExpr->iTable)
      continue;
    if (pColumn->iColumn != pExpr->iColumn)
      continue;

    if (bIgnoreAffBlob && sqlite3ExprAffinity(pColumn) <= 0x41) {
      break;
    }

    pConst->nChng++;
    (pExpr)->flags &= ~(u32)(0x800000);
    (pExpr)->flags |= (u32)(0x000020);

    pExpr->pLeft = sqlite3ExprDup(pConst->pParse->db, pConst->apExpr[i * 2 + 1], 0);
    if (pConst->pParse->db->mallocFailed)
      return 1;
    break;
  }
  return 1;
}
