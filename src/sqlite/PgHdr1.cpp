#define _GNU_SOURCE 1
#include "sqlite/PgHdr1.h"
#include "sqlite/PCache1.h"
#include "sqlite/PCacheGlobal.h"
#include "sqlite/PgFreeslot.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/sqlite3_pcache_page.h"
#include "sqlite/u16.h"
#include "sqlite/uptr.h"
#include "sqlite/SqliteStatusParameter.h"
void pcache1Free(void *p) {
  if (p == 0)
    return;
  if ((((uptr)(p) >= (uptr)((pcache1_g).pStart)) && ((uptr)(p) < (uptr)((pcache1_g).pEnd)))) {
    PgFreeslot *pSlot;
    sqlite3_mutex_enter((pcache1_g).mutex);
    sqlite3StatusDown(SQLITE_STATUS_PAGECACHE_USED, 1);
    pSlot = (PgFreeslot *)p;
    pSlot->pNext = (pcache1_g).pFree;
    (pcache1_g).pFree = pSlot;
    (pcache1_g).nFreeSlot++;
    __atomic_store_n((&(pcache1_g).bUnderPressure), ((pcache1_g).nFreeSlot < (pcache1_g).nReserve), 0);

    sqlite3_mutex_leave((pcache1_g).mutex);
  } else {
    {
      int nFreed = 0;
      nFreed = sqlite3MallocSize(p);
      sqlite3_mutex_enter((pcache1_g).mutex);
      sqlite3StatusDown(SQLITE_STATUS_PAGECACHE_OVERFLOW, nFreed);
      sqlite3_mutex_leave((pcache1_g).mutex);
    }

    sqlite3_free(p);
  }
}

void pcache1FreePage(PgHdr1 *p) {
  PCache1 *pCache;

  pCache = p->pCache;

  if (p->isBulkLocal) {
    p->pNext = pCache->pFree;
    pCache->pFree = p;
  } else {
    pcache1Free(p->page.pBuf);
  }
  (*pCache->pnPurgeable)--;
}

PgHdr1 *pcache1PinPage(PgHdr1 *pPage) {
  pPage->pLruPrev->pLruNext = pPage->pLruNext;
  pPage->pLruNext->pLruPrev = pPage->pLruPrev;
  pPage->pLruNext = 0;

  pPage->pCache->nRecyclable--;
  return pPage;
}

void pcache1RemoveFromHash(PgHdr1 *pPage, int freeFlag) {
  unsigned int h;
  PCache1 *pCache = pPage->pCache;
  PgHdr1 **pp;

  h = pPage->iKey % pCache->nHash;
  for (pp = &pCache->apHash[h]; (*pp) != pPage; pp = &(*pp)->pNext)
    ;
  *pp = (*pp)->pNext;

  pCache->nPage--;
  if (freeFlag)
    pcache1FreePage(pPage);
}