#define _GNU_SOURCE 1
#include "sqlite/PCache1.h"
#include "sqlite/PCacheGlobal.h"
#include "sqlite/PGroup.h"
#include "sqlite/PgFreeslot.h"
#include "sqlite/PgHdr1.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/sqlite3_pcache_page.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/SqliteStatusParameter.h"
void *pcache1Alloc(int nByte) {
  void *p = 0;

  if (nByte <= (pcache1_g).szSlot) {
    sqlite3_mutex_enter((pcache1_g).mutex);
    p = (PgHdr1 *)(pcache1_g).pFree;
    if (p) {
      (pcache1_g).pFree = (pcache1_g).pFree->pNext;
      (pcache1_g).nFreeSlot--;
      __atomic_store_n((&(pcache1_g).bUnderPressure), ((pcache1_g).nFreeSlot < (pcache1_g).nReserve), 0);

      sqlite3StatusHighwater(SQLITE_STATUS_PAGECACHE_SIZE, nByte);
      sqlite3StatusUp(SQLITE_STATUS_PAGECACHE_USED, 1);
    }
    sqlite3_mutex_leave((pcache1_g).mutex);
  }
  if (p == 0) {
    p = sqlite3Malloc(nByte);

    if (p) {
      int sz = sqlite3MallocSize(p);
      sqlite3_mutex_enter((pcache1_g).mutex);
      sqlite3StatusHighwater(SQLITE_STATUS_PAGECACHE_SIZE, nByte);
      sqlite3StatusUp(SQLITE_STATUS_PAGECACHE_OVERFLOW, sz);
      sqlite3_mutex_leave((pcache1_g).mutex);
    }
  }
  return p;
}

int pcache1InitBulk(PCache1 *pCache) {
  i64 szBulk;
  char *zBulk;
  if ((pcache1_g).nInitPage == 0)
    return 0;

  if (pCache->nMax < 3)
    return 0;
  sqlite3BeginBenignMalloc();
  if ((pcache1_g).nInitPage > 0) {
    szBulk = pCache->szAlloc * (i64)(pcache1_g).nInitPage;
  } else {
    szBulk = -1024 * (i64)(pcache1_g).nInitPage;
  }
  if (szBulk > pCache->szAlloc * (i64)pCache->nMax) {
    szBulk = pCache->szAlloc * (i64)pCache->nMax;
  }
  if (szBulk >= pCache->szAlloc) {
    zBulk = pCache->pBulk = sqlite3Malloc(szBulk);
    sqlite3EndBenignMalloc();
    if (zBulk) {
      int nBulk = sqlite3MallocSize(zBulk) / pCache->szAlloc;
      do {
        PgHdr1 *pX = (PgHdr1 *)&zBulk[pCache->szPage];
        pX->page.pBuf = zBulk;
        pX->page.pExtra = (u8 *)pX + (((sizeof(*pX)) + 7) & ~7);

        pX->isBulkLocal = 1;
        pX->isAnchor = 0;
        pX->pNext = pCache->pFree;
        pX->pLruPrev = 0;
        pCache->pFree = pX;
        zBulk += pCache->szAlloc;
      } while (--nBulk);
    }
  }
  return pCache->pFree != 0;
}

PgHdr1 *pcache1AllocPage(PCache1 *pCache, int benignMalloc) {
  PgHdr1 *p = 0;
  void *pPg;

  if (pCache->pFree || (pCache->nPage == 0 && pcache1InitBulk(pCache))) {
    p = pCache->pFree;
    pCache->pFree = p->pNext;
    p->pNext = 0;
  } else {
    if (benignMalloc) {
      sqlite3BeginBenignMalloc();
    }
    pPg = pcache1Alloc(pCache->szAlloc);
    if (benignMalloc) {
      sqlite3EndBenignMalloc();
    }

    if (pPg == 0)
      return 0;
    p = (PgHdr1 *)&((u8 *)pPg)[pCache->szPage];
    p->page.pBuf = pPg;
    p->page.pExtra = (u8 *)p + (((sizeof(*p)) + 7) & ~7);

    p->isBulkLocal = 0;
    p->isAnchor = 0;
    p->pLruPrev = 0;
  }
  (*pCache->pnPurgeable)++;
  return p;
}

int pcache1UnderMemoryPressure(PCache1 *pCache) {
  if ((pcache1_g).nSlot && (pCache->szPage + pCache->szExtra) <= (pcache1_g).szSlot) {
    return __atomic_load_n((&(pcache1_g).bUnderPressure), 0);
  } else {
    return sqlite3HeapNearlyFull();
  }
}

void pcache1ResizeHash(PCache1 *p) {
  PgHdr1 **apNew;
  u64 nNew;
  u32 i;

  nNew = 2 * (u64)p->nHash;
  if (nNew < 256) {
    nNew = 256;
  }

  if (p->nHash) {
    sqlite3BeginBenignMalloc();
  }
  apNew = (PgHdr1 **)sqlite3MallocZero(sizeof(PgHdr1 *) * nNew);
  if (p->nHash) {
    sqlite3EndBenignMalloc();
  }

  if (apNew) {
    for (i = 0; i < p->nHash; i++) {
      PgHdr1 *pPage;
      PgHdr1 *pNext = p->apHash[i];
      while ((pPage = pNext) != 0) {
        unsigned int h = pPage->iKey % nNew;
        pNext = pPage->pNext;
        pPage->pNext = apNew[h];
        apNew[h] = pPage;
      }
    }
    sqlite3_free(p->apHash);
    p->apHash = apNew;
    p->nHash = nNew;
  }
}

void pcache1EnforceMaxPage(PCache1 *pCache) {
  PGroup *pGroup = pCache->pGroup;
  PgHdr1 *p;

  while (pGroup->nPurgeable > pGroup->nMaxPage && (p = pGroup->lru.pLruPrev)->isAnchor == 0) {
    pcache1PinPage(p);
    pcache1RemoveFromHash(p, 1);
  }
  if (pCache->nPage == 0 && pCache->pBulk) {
    sqlite3_free(pCache->pBulk);
    pCache->pBulk = pCache->pFree = 0;
  }
}

void pcache1TruncateUnsafe(PCache1 *pCache, unsigned int iLimit) {
  unsigned int h, iStop;

  if (pCache->iMaxKey - iLimit < pCache->nHash) {
    h = iLimit % pCache->nHash;
    iStop = pCache->iMaxKey % pCache->nHash;

  } else {
    h = pCache->nHash / 2;
    iStop = h - 1;
  }
  for (;;) {
    PgHdr1 **pp;
    PgHdr1 *pPage;

    pp = &pCache->apHash[h];
    while ((pPage = *pp) != 0) {
      if (pPage->iKey >= iLimit) {
        pCache->nPage--;
        *pp = pPage->pNext;
        if (((pPage)->pLruNext != 0))
          pcache1PinPage(pPage);
        pcache1FreePage(pPage);
      } else {
        pp = &pPage->pNext;
      }
    }
    if (h == iStop)
      break;
    h = (h + 1) % pCache->nHash;
  }
}

__attribute__((noinline)) PgHdr1 *pcache1FetchStage2(PCache1 *pCache, unsigned int iKey, int createFlag) {
  unsigned int nPinned;
  PGroup *pGroup = pCache->pGroup;
  PgHdr1 *pPage = 0;

  nPinned = pCache->nPage - pCache->nRecyclable;

  if (createFlag == 1 && (nPinned >= pGroup->mxPinned || nPinned >= pCache->n90pct ||
                          (pcache1UnderMemoryPressure(pCache) && pCache->nRecyclable < nPinned))) {
    return 0;
  }

  if (pCache->nPage >= pCache->nHash)
    pcache1ResizeHash(pCache);

  if (pCache->bPurgeable && !pGroup->lru.pLruPrev->isAnchor &&
      ((pCache->nPage + 1 >= pCache->nMax) || pcache1UnderMemoryPressure(pCache))) {
    PCache1 *pOther;
    pPage = pGroup->lru.pLruPrev;

    pcache1RemoveFromHash(pPage, 0);
    pcache1PinPage(pPage);
    pOther = pPage->pCache;
    if (pOther->szAlloc != pCache->szAlloc) {
      pcache1FreePage(pPage);
      pPage = 0;
    } else {
      pGroup->nPurgeable -= (pOther->bPurgeable - pCache->bPurgeable);
    }
  }

  if (!pPage) {
    pPage = pcache1AllocPage(pCache, createFlag == 1);
  }

  if (pPage) {
    unsigned int h = iKey % pCache->nHash;
    pCache->nPage++;
    pPage->iKey = iKey;
    pPage->pNext = pCache->apHash[h];
    pPage->pCache = pCache;
    pPage->pLruNext = 0;

    *(void **)pPage->page.pExtra = 0;
    pCache->apHash[h] = pPage;
    if (iKey > pCache->iMaxKey) {
      pCache->iMaxKey = iKey;
    }
  }
  return pPage;
}