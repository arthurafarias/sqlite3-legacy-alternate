#define _GNU_SOURCE 1

#include <string.h>

#include "sqlite/sqlite3_vtab.h"

#include "sqlite/JsonEachConnection.h"
#include "sqlite/JsonEachCursor.h"
#include "sqlite/JsonString.h"
#include "sqlite/PragmaVtab.h"
#include "sqlite/PragmaVtabCursor.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_index_info.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_vtab_cursor.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
int pragmaVtabDisconnect(sqlite3_vtab *pVtab) {
  PragmaVtab *pTab = (PragmaVtab *)pVtab;
  sqlite3_free(pTab);
  return 0;
}

int pragmaVtabBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo) {
  PragmaVtab *pTab = (PragmaVtab *)tab;
  const struct sqlite3_index_constraint *pConstraint;
  int i, j;
  int seen[2];

  pIdxInfo->estimatedCost = (double)1;
  if (pTab->nHidden == 0) {
    return 0;
  }
  pConstraint = pIdxInfo->aConstraint;
  seen[0] = 0;
  seen[1] = 0;
  for (i = 0; i < pIdxInfo->nConstraint; i++, pConstraint++) {
    if (pConstraint->iColumn < pTab->iHidden)
      continue;
    if (pConstraint->op != 2)
      continue;
    if (pConstraint->usable == 0)
      return 19;
    j = pConstraint->iColumn - pTab->iHidden;


    seen[j] = i + 1;
  }
  if (seen[0] == 0) {
    pIdxInfo->estimatedCost = (double)2147483647;
    pIdxInfo->estimatedRows = 2147483647;
    return 0;
  }
  j = seen[0] - 1;
  pIdxInfo->aConstraintUsage[j].argvIndex = 1;
  pIdxInfo->aConstraintUsage[j].omit = 1;
  pIdxInfo->estimatedCost = (double)20;
  pIdxInfo->estimatedRows = 20;
  if (seen[1]) {
    j = seen[1] - 1;
    pIdxInfo->aConstraintUsage[j].argvIndex = 2;
    pIdxInfo->aConstraintUsage[j].omit = 1;
  }
  return 0;
}

int pragmaVtabOpen(sqlite3_vtab *pVtab, sqlite3_vtab_cursor **ppCursor) {
  PragmaVtabCursor *pCsr;
  pCsr = (PragmaVtabCursor *)sqlite3_malloc(sizeof(*pCsr));
  if (pCsr == 0)
    return 7;
  memset(pCsr, 0, sizeof(PragmaVtabCursor));
  pCsr->base.pVtab = pVtab;
  *ppCursor = &pCsr->base;
  return 0;
}

int jsonEachDisconnect(sqlite3_vtab *pVtab) {
  JsonEachConnection *p = (JsonEachConnection *)pVtab;
  sqlite3DbFree(p->db, pVtab);
  return 0;
}

int jsonEachOpen(sqlite3_vtab *p, sqlite3_vtab_cursor **ppCursor) {
  JsonEachConnection *pVtab = (JsonEachConnection *)p;
  JsonEachCursor *pCur;

  (void)(p);
  pCur = sqlite3DbMallocZero(pVtab->db, sizeof(*pCur));
  if (pCur == 0)
    return 7;
  pCur->db = pVtab->db;
  pCur->eMode = pVtab->eMode;
  pCur->bRecursive = pVtab->bRecursive;
  jsonStringZero(&pCur->path);
  *ppCursor = &pCur->base;
  return 0;
}

int jsonEachBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo) {
  int i;
  int aIdx[2];
  int unusableMask = 0;
  int idxMask = 0;
  const struct sqlite3_index_constraint *pConstraint;

  (void)(tab);
  aIdx[0] = aIdx[1] = -1;
  pConstraint = pIdxInfo->aConstraint;
  for (i = 0; i < pIdxInfo->nConstraint; i++, pConstraint++) {
    int iCol;
    int iMask;
    if (pConstraint->iColumn < 8)
      continue;
    iCol = pConstraint->iColumn - 8;


    ;
    iMask = 1 << iCol;
    if (pConstraint->usable == 0) {
      unusableMask |= iMask;
    } else if (pConstraint->op == 2) {
      aIdx[iCol] = i;
      idxMask |= iMask;
    }
  }
  if (pIdxInfo->nOrderBy > 0 && pIdxInfo->aOrderBy[0].iColumn < 0 && pIdxInfo->aOrderBy[0].desc == 0) {
    pIdxInfo->orderByConsumed = 1;
  }

  if ((unusableMask & ~idxMask) != 0) {

    return 19;
  }
  if (aIdx[0] < 0) {

    pIdxInfo->idxNum = 0;
  } else {
    pIdxInfo->estimatedCost = 1.0;
    i = aIdx[0];
    pIdxInfo->aConstraintUsage[i].argvIndex = 1;
    pIdxInfo->aConstraintUsage[i].omit = 1;
    if (aIdx[1] < 0) {
      pIdxInfo->idxNum = 1;
    } else {
      i = aIdx[1];
      pIdxInfo->aConstraintUsage[i].argvIndex = 2;
      pIdxInfo->aConstraintUsage[i].omit = 1;
      pIdxInfo->idxNum = 3;
    }
  }
  return 0;
}
