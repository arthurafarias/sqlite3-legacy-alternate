#define _GNU_SOURCE 1
#include <string.h>
#include <stddef.h>
#include "sqlite/PCache.h"
#include "sqlite/PCache1.h"
#include "sqlite/PCacheGlobal.h"
#include "sqlite/PGroup.h"
#include "sqlite/PgFreeslot.h"
#include "sqlite/PgHdr.h"
#include "sqlite/PgHdr1.h"
#include "sqlite/Pgno.h"
#include "sqlite/Sqlite3Config.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/sqlite3_pcache.h"
#include "sqlite/sqlite3_pcache_methods2.h"
#include "sqlite/sqlite3_pcache_page.h"
#include "sqlite/u16.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/SqliteConfigOption.h"
#include "sqlite/SqliteMutexType.h"
#include "sqlite/SqliteResultCode.h"
/* Private helpers, formerly declared in _Uncategorized.h. */
static sqlite3_pcache *pcache1Create(int szPage, int szExtra, int bPurgeable);
static int pcache1Init(void *NotUsed);
static void pcache1Shutdown(void *NotUsed);

static int pcache1Init(void *NotUsed) {
  (void)(NotUsed);

  memset(&(pcache1_g), 0, sizeof((pcache1_g)));

  (pcache1_g).separateCache = sqlite3Config.pPage == 0 || sqlite3Config.bCoreMutex > 0;

  if (sqlite3Config.bCoreMutex) {
    (pcache1_g).grp.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_LRU);
    (pcache1_g).mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_PMEM);
  }

  if ((pcache1_g).separateCache && sqlite3Config.nPage != 0 && sqlite3Config.pPage == 0) {
    (pcache1_g).nInitPage = sqlite3Config.nPage;
  } else {
    (pcache1_g).nInitPage = 0;
  }
  (pcache1_g).grp.mxPinned = 10;
  (pcache1_g).isInit = 1;
  return SQLITE_OK;
}

static void pcache1Shutdown(void *NotUsed) {
  (void)(NotUsed);

  memset(&(pcache1_g), 0, sizeof((pcache1_g)));
}

static sqlite3_pcache *pcache1Create(int szPage, int szExtra, int bPurgeable) {
  PCache1 *pCache;
  PGroup *pGroup;
  i64 sz;

  sz = sizeof(PCache1) + sizeof(PGroup) * (pcache1_g).separateCache;
  pCache = (PCache1 *)sqlite3MallocZero(sz);
  if (pCache) {
    if ((pcache1_g).separateCache) {
      pGroup = (PGroup *)&pCache[1];
      pGroup->mxPinned = 10;
    } else {
      pGroup = &(pcache1_g).grp;
    }

    if (pGroup->lru.isAnchor == 0) {
      pGroup->lru.isAnchor = 1;
      pGroup->lru.pLruPrev = pGroup->lru.pLruNext = &pGroup->lru;
    }
    pCache->pGroup = pGroup;
    pCache->szPage = szPage;
    pCache->szExtra = szExtra;
    pCache->szAlloc = szPage + szExtra + (((sizeof(PgHdr1)) + 7) & ~7);
    pCache->bPurgeable = (bPurgeable ? 1 : 0);
    pcache1ResizeHash(pCache);
    if (bPurgeable) {
      pCache->nMin = 10;
      pGroup->nMinPage += pCache->nMin;
      pGroup->mxPinned = pGroup->nMaxPage + 10 - pGroup->nMinPage;
      pCache->pnPurgeable = &pGroup->nPurgeable;
    } else {
      pCache->pnPurgeable = &pCache->nPurgeableDummy;
    }

    if (pCache->nHash == 0) {
      pcache1Destroy((sqlite3_pcache *)pCache);
      pCache = 0;
    }
  }
  return (sqlite3_pcache *)pCache;
}

int numberOfCachePages(PCache *p) {
  if (p->szCache >= 0) {
    return p->szCache;
  } else {
    i64 n;

    n = ((-1024 * (i64)p->szCache) / (p->szPage + p->szExtra));
    if (n > 1000000000)
      n = 1000000000;
    return (int)n;
  }
}

int sqlite3PcacheSetPageSize(PCache *pCache, int szPage) {
  if (pCache->szPage) {
    sqlite3_pcache *pNew;
    pNew = sqlite3Config.pcache2.xCreate(szPage, pCache->szExtra + (((sizeof(PgHdr)) + 7) & ~7), pCache->bPurgeable);
    if (pNew == 0)
      return 7;
    sqlite3Config.pcache2.xCachesize(pNew, numberOfCachePages(pCache));
    if (pCache->pCache) {
      sqlite3Config.pcache2.xDestroy(pCache->pCache);
    }
    pCache->pCache = pNew;
    pCache->szPage = szPage;
  }
  return 0;
}

sqlite3_pcache_page *sqlite3PcacheFetch(PCache *pCache, Pgno pgno, int createFlag) {
  int eCreate;
  sqlite3_pcache_page *pRes;

  eCreate = createFlag & pCache->eCreate;

  pRes = sqlite3Config.pcache2.xFetch(pCache->pCache, pgno, eCreate);

  return pRes;
}

int sqlite3PcacheFetchStress(PCache *pCache, Pgno pgno, sqlite3_pcache_page **ppPage) {
  PgHdr *pPg;
  if (pCache->eCreate == 2)
    return 0;

  if (sqlite3PcachePagecount(pCache) > pCache->szSpill) {
    for (pPg = pCache->pSynced; pPg && (pPg->nRef || (pPg->flags & 0x008)); pPg = pPg->pDirtyPrev)
      ;
    pCache->pSynced = pPg;
    if (!pPg) {
      for (pPg = pCache->pDirtyTail; pPg && pPg->nRef; pPg = pPg->pDirtyPrev)
        ;
    }
    if (pPg) {
      int rc;

      rc = pCache->xStress(pCache->pStress, pPg);
      if (rc != SQLITE_OK && rc != SQLITE_BUSY) {
        return rc;
      }
    }
  }
  *ppPage = sqlite3Config.pcache2.xFetch(pCache->pCache, pgno, 2);
  return *ppPage == 0 ? 7 : SQLITE_OK;
}

__attribute__((noinline)) PgHdr *pcacheFetchFinishWithInit(PCache *pCache, Pgno pgno, sqlite3_pcache_page *pPage) {
  PgHdr *pPgHdr;

  pPgHdr = (PgHdr *)pPage->pExtra;

  memset(&pPgHdr->pDirty, 0, sizeof(PgHdr) - offsetof(PgHdr, pDirty));
  pPgHdr->pPage = pPage;
  pPgHdr->pData = pPage->pBuf;
  pPgHdr->pExtra = (void *)&pPgHdr[1];
  memset(pPgHdr->pExtra, 0, 8);

  pPgHdr->pCache = pCache;
  pPgHdr->pgno = pgno;
  pPgHdr->flags = 0x001;
  return sqlite3PcacheFetchFinish(pCache, pgno, pPage);
}

PgHdr *sqlite3PcacheFetchFinish(PCache *pCache, Pgno pgno, sqlite3_pcache_page *pPage) {
  PgHdr *pPgHdr;

  pPgHdr = (PgHdr *)pPage->pExtra;

  if (!pPgHdr->pPage) {
    return pcacheFetchFinishWithInit(pCache, pgno, pPage);
  }
  pCache->nRefSum++;
  pPgHdr->nRef++;

  return pPgHdr;
}

void sqlite3PcacheCleanAll(PCache *pCache) {
  PgHdr *p;
  while ((p = pCache->pDirty) != 0) {
    sqlite3PcacheMakeClean(p);
  }
}

void sqlite3PcacheClearWritable(PCache *pCache) {
  PgHdr *p;
  for (p = pCache->pDirty; p; p = p->pDirtyNext) {
    p->flags &= ~(0x008 | 0x004);
  }
  pCache->pSynced = pCache->pDirtyTail;
}

void sqlite3PcacheClearSyncFlags(PCache *pCache) {
  PgHdr *p;
  for (p = pCache->pDirty; p; p = p->pDirtyNext) {
    p->flags &= ~0x008;
  }
  pCache->pSynced = pCache->pDirtyTail;
}

void sqlite3PcacheTruncate(PCache *pCache, Pgno pgno) {
  if (pCache->pCache) {
    PgHdr *p;
    PgHdr *pNext;
    for (p = pCache->pDirty; p; p = pNext) {
      pNext = p->pDirtyNext;

      if (p->pgno > pgno) {
        sqlite3PcacheMakeClean(p);
      }
    }
    if (pgno == 0 && pCache->nRefSum) {
      sqlite3_pcache_page *pPage1;
      pPage1 = sqlite3Config.pcache2.xFetch(pCache->pCache, 1, 0);
      if ((pPage1)) {
        memset(pPage1->pBuf, 0, pCache->szPage);
        pgno = 1;
      }
    }
    sqlite3Config.pcache2.xTruncate(pCache->pCache, pgno + 1);
  }
}

void sqlite3PcacheClose(PCache *pCache) {
  sqlite3Config.pcache2.xDestroy(pCache->pCache);
}

void sqlite3PcacheClear(PCache *pCache) {
  sqlite3PcacheTruncate(pCache, 0);
}

PgHdr *sqlite3PcacheDirtyList(PCache *pCache) {
  PgHdr *p;
  for (p = pCache->pDirty; p; p = p->pDirtyNext) {
    p->pDirty = p->pDirtyNext;
  }
  return pcacheSortDirtyList(pCache->pDirty);
}

i64 sqlite3PcacheRefCount(PCache *pCache) {
  return pCache->nRefSum;
}

int sqlite3PcachePagecount(PCache *pCache) {
  return sqlite3Config.pcache2.xPagecount(pCache->pCache);
}

void sqlite3PcacheSetCachesize(PCache *pCache, int mxPage) {
  pCache->szCache = mxPage;
  sqlite3Config.pcache2.xCachesize(pCache->pCache, numberOfCachePages(pCache));
}

int sqlite3PcacheSetSpillsize(PCache *p, int mxPage) {
  int res;

  if (mxPage) {
    if (mxPage < 0) {
      mxPage = (int)((-1024 * (i64)mxPage) / (p->szPage + p->szExtra));
    }
    p->szSpill = mxPage;
  }
  res = numberOfCachePages(p);
  if (res < p->szSpill)
    res = p->szSpill;
  return res;
}

void sqlite3PcacheShrink(PCache *pCache) {
  sqlite3Config.pcache2.xShrink(pCache->pCache);
}

int sqlite3PCachePercentDirty(PCache *pCache) {
  PgHdr *pDirty;
  int nDirty = 0;
  int nCache = numberOfCachePages(pCache);
  for (pDirty = pCache->pDirty; pDirty; pDirty = pDirty->pDirtyNext)
    nDirty++;
  return nCache ? (int)(((i64)nDirty * 100) / nCache) : 0;
}

int sqlite3PCacheIsDirty(PCache *pCache) {
  return (pCache->pDirty != 0);
}

void sqlite3PCacheBufferSetup(void *pBuf, int sz, int n) {
  if ((pcache1_g).isInit) {
    PgFreeslot *p;
    if (pBuf == 0)
      sz = n = 0;
    if (n == 0)
      sz = 0;
    sz = ((sz) & ~7);
    (pcache1_g).szSlot = sz;
    (pcache1_g).nSlot = (pcache1_g).nFreeSlot = n;
    (pcache1_g).nReserve = n > 90 ? 10 : (n / 10 + 1);
    (pcache1_g).pStart = pBuf;
    (pcache1_g).pFree = 0;
    __atomic_store_n((&(pcache1_g).bUnderPressure), (0), 0);
    while (n--) {
      p = (PgFreeslot *)pBuf;
      p->pNext = (pcache1_g).pFree;
      (pcache1_g).pFree = p;
      pBuf = (void *)&((char *)pBuf)[sz];
    }
    (pcache1_g).pEnd = pBuf;
  }
}

void sqlite3PCacheSetDefault(void) {
  static const sqlite3_pcache_methods2 defaultMethods = {1,
                                                         0,
                                                         pcache1Init,
                                                         pcache1Shutdown,
                                                         pcache1Create,
                                                         pcache1Cachesize,
                                                         pcache1Pagecount,
                                                         pcache1Fetch,
                                                         pcache1Unpin,
                                                         pcache1Rekey,
                                                         pcache1Truncate,
                                                         pcache1Destroy,
                                                         pcache1Shrink};
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &defaultMethods);
}