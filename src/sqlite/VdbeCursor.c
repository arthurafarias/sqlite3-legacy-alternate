#define _GNU_SOURCE 1

#include <string.h>

#include "sqlite/VdbeCursor.h"

#include "sqlite/Bool.h"
#include "sqlite/BtCursor.h"
#include "sqlite/KeyInfo.h"
#include "sqlite/Mem.h"
#include "sqlite/RCStr.h"
#include "sqlite/SortSubtask.h"
#include "sqlite/SorterList.h"
#include "sqlite/SorterRecord.h"
#include "sqlite/UnpackedRecord.h"
#include "sqlite/Vdbe.h"
#include "sqlite/VdbeSorter.h"
#include "sqlite/VdbeTxtBlbCache.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_value.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
int __attribute__((noinline)) sqlite3VdbeFinishMoveto(VdbeCursor *p) {
  int res, rc;

  rc = sqlite3BtreeTableMoveto(p->uc.pCursor, p->movetoTarget, 0, &res);
  if (rc)
    return rc;
  if (res != 0)
    return sqlite3CorruptError(91685);

  p->deferredMoveto = 0;
  p->cacheStatus = 0;
  return 0;
}

int __attribute__((noinline)) sqlite3VdbeHandleMovedCursor(VdbeCursor *p) {
  int isDifferentRow, rc;

  rc = sqlite3BtreeCursorRestore(p->uc.pCursor, &isDifferentRow);
  p->cacheStatus = 0;
  if (isDifferentRow)
    p->nullRow = 1;
  return rc;
}

int sqlite3VdbeCursorRestore(VdbeCursor *p) {

  if (sqlite3BtreeCursorHasMoved(p->uc.pCursor)) {
    return sqlite3VdbeHandleMovedCursor(p);
  }
  return 0;
}

__attribute__((noinline)) int vdbeColumnFromOverflow(VdbeCursor *pC, int iCol, u32 t, i64 iOffset, u32 cacheStatus, u32 colCacheCtr, Mem *pDest) {
  int rc;
  sqlite3 *db = pDest->db;
  int encoding = pDest->enc;
  int len = sqlite3VdbeSerialTypeLen(t);

  if (len > db->aLimit[0])
    return 18;
  if (len > 4000 && pC->pKeyInfo == 0) {

    VdbeTxtBlbCache *pCache;
    char *pBuf;
    if (pC->colCache == 0) {
      pC->pCache = sqlite3DbMallocZero(db, sizeof(VdbeTxtBlbCache));
      if (pC->pCache == 0)
        return 7;
      pC->colCache = 1;
    }
    pCache = pC->pCache;
    if (pCache->pCValue == 0 || pCache->iCol != iCol || pCache->cacheStatus != cacheStatus || pCache->colCacheCtr != colCacheCtr || pCache->iOffset != sqlite3BtreeOffset(pC->uc.pCursor)) {
      if (pCache->pCValue)
        sqlite3RCStrUnref(pCache->pCValue);
      pBuf = pCache->pCValue = sqlite3RCStrNew(len + 3);
      if (pBuf == 0)
        return 7;
      rc = sqlite3BtreePayload(pC->uc.pCursor, iOffset, len, pBuf);
      if (rc)
        return rc;
      pBuf[len] = 0;
      pBuf[len + 1] = 0;
      pBuf[len + 2] = 0;
      pCache->iCol = iCol;
      pCache->cacheStatus = cacheStatus;
      pCache->colCacheCtr = colCacheCtr;
      pCache->iOffset = sqlite3BtreeOffset(pC->uc.pCursor);
    } else {
      pBuf = pCache->pCValue;
    }


    sqlite3RCStrRef(pBuf);
    if (t & 1) {
      rc = sqlite3VdbeMemSetStr(pDest, pBuf, len, encoding, sqlite3RCStrUnref);
      pDest->flags |= 0x0200;
    } else {
      rc = sqlite3VdbeMemSetStr(pDest, pBuf, len, 0, sqlite3RCStrUnref);
    }
  } else {
    rc = sqlite3VdbeMemFromBtree(pC->uc.pCursor, iOffset, len, pDest);
    if (rc)
      return rc;
    sqlite3VdbeSerialGet((const u8 *)pDest->z, t, pDest);
    if ((t & 1) != 0 && encoding == 1) {
      pDest->z[len] = 0;
      pDest->flags |= 0x0200;
    }
  }
  pDest->flags &= ~0x4000;
  return rc;
}

int sqlite3VdbeSorterWrite(const VdbeCursor *pCsr, Mem *pVal) {
  VdbeSorter *pSorter;
  int rc = 0;
  SorterRecord *pNew;
  int bFlush;
  i64 nReq;
  i64 nPMA;
  int t;

  pSorter = pCsr->uc.pSorter;
  t = (u32) * ((const u8 *)&pVal->z[1]);
  if (t >= 0x80)
    sqlite3GetVarint32(((const u8 *)&pVal->z[1]), (u32 *)&(t));
  if (t > 0 && t < 10 && t != 7) {
    pSorter->typeMask &= 0x01;
  } else if (t > 10 && (t & 0x01)) {
    pSorter->typeMask &= 0x02;
  } else {
    pSorter->typeMask = 0;
  }

  nReq = pVal->n + sizeof(SorterRecord);
  nPMA = pVal->n + sqlite3VarintLen(pVal->n);
  if (pSorter->mxPmaSize) {
    if (pSorter->list.aMemory) {
      bFlush = pSorter->iMemory && (pSorter->iMemory + nReq) > pSorter->mxPmaSize;
    } else {
      bFlush = ((pSorter->list.szPMA > pSorter->mxPmaSize) || (pSorter->list.szPMA > pSorter->mnPmaSize && sqlite3HeapNearlyFull()));
    }
    if (bFlush) {
      rc = vdbeSorterFlushPMA(pSorter);
      pSorter->list.szPMA = 0;
      pSorter->iMemory = 0;


    }
  }

  pSorter->list.szPMA += nPMA;
  if (nPMA > pSorter->mxKeysize) {
    pSorter->mxKeysize = nPMA;
  }

  if (pSorter->list.aMemory) {
    int nMin = pSorter->iMemory + nReq;

    if (nMin > pSorter->nMemory) {
      u8 *aNew;
      sqlite3_int64 nNew = 2 * (sqlite3_int64)pSorter->nMemory;
      int iListOff = -1;
      if (pSorter->list.pList) {
        iListOff = (u8 *)pSorter->list.pList - pSorter->list.aMemory;
      }
      while (nNew < nMin)
        nNew = nNew * 2;
      if (nNew > pSorter->mxPmaSize)
        nNew = pSorter->mxPmaSize;
      if (nNew < nMin)
        nNew = nMin;
      aNew = sqlite3Realloc(pSorter->list.aMemory, nNew);
      if (!aNew)
        return 7;
      if (iListOff >= 0) {
        pSorter->list.pList = (SorterRecord *)&aNew[iListOff];
      }
      pSorter->list.aMemory = aNew;
      pSorter->nMemory = nNew;
    }

    pNew = (SorterRecord *)&pSorter->list.aMemory[pSorter->iMemory];
    pSorter->iMemory += (((nReq) + 7) & ~7);
    if (pSorter->list.pList) {
      pNew->u.iNext = (int)((u8 *)(pSorter->list.pList) - pSorter->list.aMemory);
    }
  } else {
    pNew = (SorterRecord *)sqlite3Malloc(nReq);
    if (pNew == 0) {
      return 7;
    }
    pNew->u.pNext = pSorter->list.pList;
  }

  memcpy(((void *)((SorterRecord *)(pNew) + 1)), pVal->z, pVal->n);
  pNew->nVal = pVal->n;
  pSorter->list.pList = pNew;

  return rc;
}

int sqlite3VdbeSorterRewind(const VdbeCursor *pCsr, int *pbEof) {
  VdbeSorter *pSorter;
  int rc = 0;

  pSorter = pCsr->uc.pSorter;

  if (pSorter->bUsePMA == 0) {
    if (pSorter->list.pList) {
      *pbEof = 0;
      rc = vdbeSorterSort(&pSorter->aTask[0], &pSorter->list);
    } else {
      *pbEof = 1;
    }
    return rc;
  }

  rc = vdbeSorterFlushPMA(pSorter);

  rc = vdbeSorterJoinAll(pSorter, rc);

  ;

  if (rc == 0) {
    rc = vdbeSorterSetupMerge(pSorter);
    *pbEof = 0;
  }

  ;
  return rc;
}

int sqlite3VdbeSorterRowkey(const VdbeCursor *pCsr, Mem *pOut) {
  VdbeSorter *pSorter;
  void *pKey;
  int nKey;

  pSorter = pCsr->uc.pSorter;
  pKey = vdbeSorterRowkey(pSorter, &nKey);
  if (sqlite3VdbeMemClearAndResize(pOut, nKey)) {
    return 7;
  }
  pOut->n = nKey;
  ((pOut)->flags = ((pOut)->flags & ~(0x0dbf | 0x0400)) | 0x0010);
  memcpy(pOut->z, pKey, nKey);

  return 0;
}

int sqlite3VdbeSorterCompare(const VdbeCursor *pCsr, Mem *pVal, int nKeyCol, int *pRes) {
  VdbeSorter *pSorter;
  UnpackedRecord *r2;
  KeyInfo *pKeyInfo;
  int i;
  void *pKey;
  int nKey;

  pSorter = pCsr->uc.pSorter;
  r2 = pSorter->pUnpacked;
  pKeyInfo = pCsr->pKeyInfo;
  if (r2 == 0) {
    r2 = pSorter->pUnpacked = sqlite3VdbeAllocUnpackedRecord(pKeyInfo);
    if (r2 == 0)
      return 7;
    r2->nField = nKeyCol;
  }

  pKey = vdbeSorterRowkey(pSorter, &nKey);
  sqlite3VdbeRecordUnpack(nKey, pKey, r2);
  for (i = 0; i < nKeyCol; i++) {
    if (r2->aMem[i].flags & 0x0001) {
      *pRes = -1;
      return 0;
    }
  }

  *pRes = sqlite3VdbeRecordCompare(pVal->n, pVal->z, r2);
  return 0;
}
