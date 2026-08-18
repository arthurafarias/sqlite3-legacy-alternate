#define _GNU_SOURCE 1
#include "sqlite/RowSet.h"
#include "sqlite/RowSetChunk.h"
#include "sqlite/RowSetEntry.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/u16.h"
#include "sqlite/u64.h"
void sqlite3RowSetClear(void *pArg) {
  RowSet *p = (RowSet *)pArg;
  struct RowSetChunk *pChunk, *pNextChunk;
  for (pChunk = p->pChunk; pChunk; pChunk = pNextChunk) {
    pNextChunk = pChunk->pNextChunk;
    sqlite3DbFree(p->db, pChunk);
  }
  p->pChunk = 0;
  p->nFresh = 0;
  p->pEntry = 0;
  p->pLast = 0;
  p->pForest = 0;
  p->rsFlags = 0x01;
}

void sqlite3RowSetDelete(void *pArg) {
  sqlite3RowSetClear(pArg);
  sqlite3DbFree(((RowSet *)pArg)->db, pArg);
}

struct RowSetEntry *rowSetEntryAlloc(RowSet *p) {
  if (p->nFresh == 0) {
    struct RowSetChunk *pNew;
    pNew = sqlite3DbMallocRawNN(p->db, sizeof(*pNew));
    if (pNew == 0) {
      return 0;
    }
    pNew->pNextChunk = p->pChunk;
    p->pChunk = pNew;
    p->pFresh = pNew->aEntry;
    p->nFresh = ((1024 - 8) / sizeof(struct RowSetEntry));
  }
  p->nFresh--;
  return p->pFresh++;
}

void sqlite3RowSetInsert(RowSet *p, i64 rowid) {
  struct RowSetEntry *pEntry;
  struct RowSetEntry *pLast;

  pEntry = rowSetEntryAlloc(p);
  if (pEntry == 0)
    return;
  pEntry->v = rowid;
  pEntry->pRight = 0;
  pLast = p->pLast;
  if (pLast) {
    if (rowid <= pLast->v) {
      p->rsFlags &= ~0x01;
    }
    pLast->pRight = pEntry;
  } else {
    p->pEntry = pEntry;
  }
  p->pLast = pEntry;
}

int sqlite3RowSetNext(RowSet *p, i64 *pRowid) {
  if ((p->rsFlags & 0x02) == 0) {
    if ((p->rsFlags & 0x01) == 0) {
      p->pEntry = rowSetEntrySort(p->pEntry);
    }
    p->rsFlags |= 0x01 | 0x02;
  }

  if (p->pEntry) {
    *pRowid = p->pEntry->v;
    p->pEntry = p->pEntry->pRight;
    if (p->pEntry == 0) {
      sqlite3RowSetClear(p);
    }
    return 1;
  } else {
    return 0;
  }
}

int sqlite3RowSetTest(RowSet *pRowSet, int iBatch, sqlite3_int64 iRowid) {
  struct RowSetEntry *p, *pTree;

  if (iBatch != pRowSet->iBatch) {
    p = pRowSet->pEntry;
    if (p) {
      struct RowSetEntry **ppPrevTree = &pRowSet->pForest;
      if ((pRowSet->rsFlags & 0x01) == 0) {
        p = rowSetEntrySort(p);
      }
      for (pTree = pRowSet->pForest; pTree; pTree = pTree->pRight) {
        ppPrevTree = &pTree->pRight;
        if (pTree->pLeft == 0) {
          pTree->pLeft = rowSetListToTree(p);
          break;
        } else {
          struct RowSetEntry *pAux, *pTail;
          rowSetTreeToList(pTree->pLeft, &pAux, &pTail);
          pTree->pLeft = 0;
          p = rowSetEntryMerge(pAux, p);
        }
      }
      if (pTree == 0) {
        *ppPrevTree = pTree = rowSetEntryAlloc(pRowSet);
        if (pTree) {
          pTree->v = 0;
          pTree->pRight = 0;
          pTree->pLeft = rowSetListToTree(p);
        }
      }
      pRowSet->pEntry = 0;
      pRowSet->pLast = 0;
      pRowSet->rsFlags |= 0x01;
    }
    pRowSet->iBatch = iBatch;
  }

  for (pTree = pRowSet->pForest; pTree; pTree = pTree->pRight) {
    p = pTree->pLeft;
    while (p) {
      if (p->v < iRowid) {
        p = p->pRight;
      } else if (p->v > iRowid) {
        p = p->pLeft;
      } else {
        return 1;
      }
    }
  }
  return 0;
}
