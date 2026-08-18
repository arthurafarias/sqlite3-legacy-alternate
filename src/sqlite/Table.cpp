#define _GNU_SOURCE 1
#include "sqlite/Table.h"
#include "sqlite/Column.h"
#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/FKey.h"
#include "sqlite/Hash.h"
#include "sqlite/Index.h"
#include "sqlite/LogEst.h"
#include "sqlite/Schema.h"
#include "sqlite/i16.h"
#include "sqlite/sqlite3.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
char sqlite3TableColumnAffinity(const Table *pTab, int iCol) {
  if (iCol < 0 || (iCol >= pTab->nCol))
    return 0x44;
  return pTab->aCol[iCol].affinity;
}

const char *sqlite3RowidAlias(Table *pTab) {
  const char *azOpt[] = {"_ROWID_", "ROWID", "OID"};
  int ii;

  for (ii = 0; ii < ((int)(sizeof(azOpt) / sizeof(azOpt[0]))); ii++) {
    if (sqlite3ColumnIndex(pTab, azOpt[ii]) < 0)
      return azOpt[ii];
  }
  return 0;
}

Expr *sqlite3ColumnExpr(Table *pTab, Column *pCol) {
  if (pCol->iDflt == 0)
    return 0;
  if (!((pTab)->eTabType == 0))
    return 0;
  if (pTab->u.tab.pDfltList == 0)
    return 0;
  if ((pTab->u.tab.pDfltList->nExpr < pCol->iDflt))
    return 0;
  return pTab->u.tab.pDfltList->a[pCol->iDflt - 1].pExpr;
}

Index *sqlite3PrimaryKeyIndex(Table *pTab) {
  Index *p;
  for (p = pTab->pIndex; p && !((p)->idxType == 2); p = p->pNext) {
  }
  return p;
}

i16 sqlite3StorageColumnToTable(Table *pTab, i16 iCol) {
  if (pTab->tabFlags & 0x00000020) {
    int i;
    for (i = 0; i <= iCol; i++) {
      if (pTab->aCol[i].colFlags & 0x0020)
        iCol++;
    }
  }
  return iCol;
}

i16 sqlite3TableColumnToStorage(Table *pTab, i16 iCol) {
  int i;
  i16 n;

  if ((pTab->tabFlags & 0x00000020) == 0 || iCol < 0)
    return iCol;
  for (i = 0, n = 0; i < iCol; i++) {
    if ((pTab->aCol[i].colFlags & 0x0020) == 0)
      n++;
  }
  if (pTab->aCol[i].colFlags & 0x0020) {
    return pTab->nNVCol + i - n;
  } else {
    return n;
  }
}

void estimateTableWidth(Table *pTab) {
  unsigned wTable = 0;
  const Column *pTabCol;
  int i;
  for (i = pTab->nCol, pTabCol = pTab->aCol; i > 0; i--, pTabCol++) {
    wTable += pTabCol->szEst;
  }
  if (pTab->iPKey < 0)
    wTable++;
  pTab->szTabRow = sqlite3LogEst(wTable * 4);
}

FKey *sqlite3FkReferences(Table *pTab) {
  return (FKey *)sqlite3HashFind(&pTab->pSchema->fkeyHash, pTab->zName);
}

int fkChildIsModified(Table *pTab, FKey *p, int *aChange, int bChngRowid) {
  int i;
  for (i = 0; i < p->nCol; i++) {
    int iChildKey = p->aCol[i].iFrom;
    if (aChange[iChildKey] >= 0)
      return 1;
    if (iChildKey == pTab->iPKey && bChngRowid)
      return 1;
  }
  return 0;
}

int fkParentIsModified(Table *pTab, FKey *p, int *aChange, int bChngRowid) {
  int i;
  for (i = 0; i < p->nCol; i++) {
    char *zKey = p->aCol[i].zCol;
    int iKey;
    for (iKey = 0; iKey < pTab->nCol; iKey++) {
      if (aChange[iKey] >= 0 || (iKey == pTab->iPKey && bChngRowid)) {
        Column *pCol = &pTab->aCol[iKey];
        if (zKey) {
          if (0 == sqlite3StrICmp(pCol->zCnName, zKey))
            return 1;
        } else if (pCol->colFlags & 0x0001) {
          return 1;
        }
      }
    }
  }
  return 0;
}

int tableSkipIntegrityCheck(const Table *pTab, const Table *pObjTab) {
  if (pObjTab) {
    return pTab != pObjTab;
  } else {
    return (pTab->tabFlags & 0x00020000) != 0;
  }
}

int sqlite3ColumnIndex(Table *pTab, const char *zCol) {
  int i;
  u8 h;
  const Column *aCol;
  int nCol;

  h = sqlite3StrIHash(zCol);
  aCol = pTab->aCol;
  nCol = pTab->nCol;

  i = pTab->aHx[h % sizeof(pTab->aHx)];

  if (aCol[i].hName == h && sqlite3StrICmp(aCol[i].zCnName, zCol) == 0) {
    return i;
  }

  i = 0;
  while (1) {
    if (aCol[i].hName == h && sqlite3StrICmp(aCol[i].zCnName, zCol) == 0) {
      return i;
    }
    i++;
    if (i >= nCol)
      break;
  }
  return -1;
}

__attribute__((noinline)) int columnIsGoodIndexCandidate(const Table *pTab, int iCol) {
  const Index *pIdx;
  for (pIdx = pTab->pIndex; pIdx != 0; pIdx = pIdx->pNext) {
    int j;
    for (j = 0; j < pIdx->nKeyCol; j++) {
      if (pIdx->aiColumn[j] == iCol) {
        if (j == 0)
          return 0;
        if (pIdx->hasStat1 && pIdx->aiRowLogEst[j + 1] > 20)
          return 0;
        break;
      }
    }
  }
  return 1;
}
