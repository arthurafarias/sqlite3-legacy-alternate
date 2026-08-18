
#pragma once

#include "sqlite/sqlite3_pcache_page.h"
#include "sqlite/u16.h"
  typedef struct PCache1 PCache1;

  typedef struct PgHdr1 PgHdr1;
  struct PgHdr1 {
    sqlite3_pcache_page page;
    unsigned int iKey;
    u16 isBulkLocal;
    u16 isAnchor;
    PgHdr1 *pNext;
    PCache1 *pCache;
    PgHdr1 *pLruNext;
    PgHdr1 *pLruPrev;
  };

  void pcache1FreePage(PgHdr1 * p);
  PgHdr1 *pcache1PinPage(PgHdr1 * pPage);
  void pcache1RemoveFromHash(PgHdr1 * pPage, int freeFlag);

  void pcache1Free(void *p);


