#define _GNU_SOURCE 1

#include <string.h>

#include "sqlite/Index.h"

#include "sqlite/Column.h"
#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/LogEst.h"
#include "sqlite/Parse.h"
#include "sqlite/Pgno.h"
#include "sqlite/Table.h"
#include "sqlite/i16.h"
#include "sqlite/sqlite3.h"
#include "sqlite/u16.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
int sqlite3TableColumnToIndex(Index *pIdx, int iCol) {
  int i;
  i16 iCol16;

  iCol16 = iCol;
  for (i = 0; i < pIdx->nColumn; i++) {
    if (iCol16 == pIdx->aiColumn[i]) {
      return i;
    }
  }
  return -1;
}

void estimateIndexWidth(Index *pIdx) {
  unsigned wIndex = 0;
  int i;
  const Column *aCol = pIdx->pTable->aCol;
  for (i = 0; i < pIdx->nColumn; i++) {
    i16 x = pIdx->aiColumn[i];


    wIndex += x < 0 ? 1 : aCol[x].szEst;
  }
  pIdx->szIdxRow = sqlite3LogEst(wIndex * 4);
}

int isDupColumn(Index *pIdx, int nKey, Index *pPk, int iCol) {
  int i, j;

  ;
  j = pPk->aiColumn[iCol];

  for (i = 0; i < nKey; i++) {


    if (pIdx->aiColumn[i] == j && sqlite3StrICmp(pIdx->azColl[i], pPk->azColl[iCol]) == 0) {
      return 1;
    }
  }
  return 0;
}

void recomputeColumnsNotIndexed(Index *pIdx) {
  Bitmask m = 0;
  int j;
  Table *pTab = pIdx->pTable;
  for (j = pIdx->nColumn - 1; j >= 0; j--) {
    int x = pIdx->aiColumn[j];
    if (x >= 0 && (pTab->aCol[x].colFlags & 0x0020) == 0) {
      ;
      ;
      if (x < ((int)(sizeof(Bitmask) * 8)) - 1)
        m |= (((Bitmask)1) << (x));
    }
  }
  pIdx->colNotIdxed = ~m;
}

void sqlite3DefaultRowEst(Index *pIdx) {

  static const LogEst aVal[] = {33, 32, 30, 28, 26};
  LogEst *a = pIdx->aiRowLogEst;
  LogEst x;
  int nCopy = ((((int)(sizeof(aVal) / sizeof(aVal[0])))) < (pIdx->nKeyCol) ? (((int)(sizeof(aVal) / sizeof(aVal[0])))) : (pIdx->nKeyCol));
  int i;

  x = pIdx->pTable->nRowLogEst;

  if (x < 99) {
    pIdx->pTable->nRowLogEst = x = 99;
  }
  if (pIdx->pPartIdxWhere != 0) {
    x -= 10;


  }
  a[0] = x;

  memcpy(&a[1], aVal, nCopy * sizeof(LogEst));
  for (i = nCopy + 1; i <= pIdx->nKeyCol; i++) {
    a[i] = 23;


  }

  if (((pIdx)->onError != 0))
    a[pIdx->nKeyCol] = 0;
}

int xferCompatibleIndex(Index *pDest, Index *pSrc) {
  int i;

  if (pDest->nKeyCol != pSrc->nKeyCol || pDest->nColumn != pSrc->nColumn) {
    return 0;
  }
  if (pDest->onError != pSrc->onError) {
    return 0;
  }
  for (i = 0; i < pSrc->nKeyCol; i++) {
    if (pSrc->aiColumn[i] != pDest->aiColumn[i]) {
      return 0;
    }
    if (pSrc->aiColumn[i] == (-2)) {


      if (sqlite3ExprCompare(0, pSrc->aColExpr->a[i].pExpr, pDest->aColExpr->a[i].pExpr, -1) != 0) {
        return 0;
      }
    }
    if (pSrc->aSortOrder[i] != pDest->aSortOrder[i]) {
      return 0;
    }
    if (sqlite3_stricmp(pSrc->azColl[i], pDest->azColl[i]) != 0) {
      return 0;
    }
  }
  if (sqlite3ExprCompare(0, pSrc->pPartIdxWhere, pDest->pPartIdxWhere, -1)) {
    return 0;
  }

  return 1;
}

int sqlite3IndexHasDuplicateRootPage(Index *pIndex) {
  Index *p;
  for (p = pIndex->pTable->pIndex; p; p = p->pNext) {
    if (p->tnum == pIndex->tnum && p != pIndex)
      return 1;
  }
  return 0;
}

int indexColumnIsBeingUpdated(Index *pIdx, int iCol, int *aXRef, int chngRowid) {
  i16 iIdxCol = pIdx->aiColumn[iCol];

  if (iIdxCol >= 0) {
    return aXRef[iIdxCol] >= 0;
  }

  return sqlite3ExprReferencesUpdatedColumn(pIdx->aColExpr->a[iCol].pExpr, aXRef, chngRowid);
}

int indexWhereClauseMightChange(Index *pIdx, int *aXRef, int chngRowid) {
  if (pIdx->pPartIdxWhere == 0)
    return 0;
  return sqlite3ExprReferencesUpdatedColumn(pIdx->pPartIdxWhere, aXRef, chngRowid);
}

const char *explainIndexColumnName(Index *pIdx, int i) {
  i = pIdx->aiColumn[i];
  if (i == (-2))
    return "<expr>";
  if (i == (-1))
    return "rowid";
  return pIdx->pTable->aCol[i].zCnName;
}

int indexColumnNotNull(Index *pIdx, int iCol) {
  int j;

  j = pIdx->aiColumn[iCol];
  if (j >= 0) {
    return pIdx->pTable->aCol[j].notNull;
  } else if (j == (-1)) {
    return 1;
  } else {


    return 0;
  }
}
