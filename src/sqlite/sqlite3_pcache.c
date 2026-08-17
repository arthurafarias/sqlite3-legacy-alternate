#define _GNU_SOURCE 1

#include "sqlite/sqlite3_pcache.h"

#include "sqlite/PCache1.h"
#include "sqlite/PGroup.h"
#include "sqlite/PgHdr1.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_pcache_page.h"
#include "sqlite/u32.h"
void pcache1Cachesize(sqlite3_pcache *p, int nMax) {
  PCache1 *pCache = (PCache1 *)p;
  u32 n;

  if (pCache->bPurgeable) {
    PGroup *pGroup = pCache->pGroup;

    ((void)(0))

        ;
    n = (u32)nMax;
    if (n > 0x7fff0000 - pGroup->nMaxPage + pCache->nMax) {
      n = 0x7fff0000 - pGroup->nMaxPage + pCache->nMax;
    }
    pGroup->nMaxPage += (n - pCache->nMax);
    pGroup->mxPinned = pGroup->nMaxPage + 10 - pGroup->nMinPage;
    pCache->nMax = n;
    pCache->n90pct = pCache->nMax * 9 / 10;
    pcache1EnforceMaxPage(pCache);

    ((void)(0))

        ;
  }
}

void pcache1Shrink(sqlite3_pcache *p) {
  PCache1 *pCache = (PCache1 *)p;
  if (pCache->bPurgeable) {
    PGroup *pGroup = pCache->pGroup;
    unsigned int savedMaxPage;

    ((void)(0))

        ;
    savedMaxPage = pGroup->nMaxPage;
    pGroup->nMaxPage = 0;
    pcache1EnforceMaxPage(pCache);
    pGroup->nMaxPage = savedMaxPage;

    ((void)(0))

        ;
  }
}

int pcache1Pagecount(sqlite3_pcache *p) {
  int n;
  PCache1 *pCache = (PCache1 *)p;

  n = pCache->nPage;

  return n;
}

PgHdr1 *pcache1FetchNoMutex(sqlite3_pcache *p, unsigned int iKey, int createFlag) {
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *pPage = 0;

  pPage = pCache->apHash[iKey % pCache->nHash];
  while (pPage && pPage->iKey != iKey) {
    pPage = pPage->pNext;
  }

  if (pPage) {
    if (((pPage)->pLruNext != 0)) {
      return pcache1PinPage(pPage);
    } else {
      return pPage;
    }
  } else if (createFlag) {

    return pcache1FetchStage2(pCache, iKey, createFlag);
  } else {
    return 0;
  }
}

sqlite3_pcache_page *pcache1Fetch(sqlite3_pcache *p, unsigned int iKey, int createFlag) {

  {
    return (sqlite3_pcache_page *)pcache1FetchNoMutex(p, iKey, createFlag);
  }
}

void pcache1Unpin(sqlite3_pcache *p, sqlite3_pcache_page *pPg, int reuseUnlikely) {
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *pPage = (PgHdr1 *)pPg;
  PGroup *pGroup = pCache->pGroup;

  if (reuseUnlikely || pGroup->nPurgeable > pGroup->nMaxPage) {
    pcache1RemoveFromHash(pPage, 1);
  } else {

    PgHdr1 **ppFirst = &pGroup->lru.pLruNext;
    pPage->pLruPrev = &pGroup->lru;
    (pPage->pLruNext = *ppFirst)->pLruPrev = pPage;
    *ppFirst = pPage;
    pCache->nRecyclable++;
  }
}

void pcache1Rekey(sqlite3_pcache *p, sqlite3_pcache_page *pPg, unsigned int iOld, unsigned int iNew) {
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *pPage = (PgHdr1 *)pPg;
  PgHdr1 **pp;
  unsigned int hOld, hNew;

  hOld = iOld % pCache->nHash;
  pp = &pCache->apHash[hOld];
  while ((*pp) != pPage) {
    pp = &(*pp)->pNext;
  }
  *pp = pPage->pNext;

  hNew = iNew % pCache->nHash;
  pPage->iKey = iNew;
  pPage->pNext = pCache->apHash[hNew];
  pCache->apHash[hNew] = pPage;
  if (iNew > pCache->iMaxKey) {
    pCache->iMaxKey = iNew;
  }
}

void pcache1Truncate(sqlite3_pcache *p, unsigned int iLimit) {
  PCache1 *pCache = (PCache1 *)p;

  if (iLimit <= pCache->iMaxKey) {
    pcache1TruncateUnsafe(pCache, iLimit);
    pCache->iMaxKey = iLimit - 1;
  }
}

void pcache1Destroy(sqlite3_pcache *p) {
  PCache1 *pCache = (PCache1 *)p;
  PGroup *pGroup = pCache->pGroup;

  if (pCache->nPage)
    pcache1TruncateUnsafe(pCache, 0);

  pGroup->nMaxPage -= pCache->nMax;

  pGroup->nMinPage -= pCache->nMin;
  pGroup->mxPinned = pGroup->nMaxPage + 10 - pGroup->nMinPage;
  pcache1EnforceMaxPage(pCache);

  sqlite3_free(pCache->pBulk);
  sqlite3_free(pCache->apHash);
  sqlite3_free(pCache);
}
