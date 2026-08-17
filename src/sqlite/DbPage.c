#define _GNU_SOURCE 1
#include "sqlite/DbPage.h"
#include "sqlite/BtShared.h"
#include "sqlite/MemPage.h"
#include "sqlite/Pager.h"
#include "sqlite/PgHdr.h"
#include "sqlite/Pgno.h"
#include "sqlite/i64.h"
#include "sqlite/u16.h"
#include "sqlite/u8.h"
void sqlite3PagerRef(DbPage *pPg) {
  sqlite3PcacheRef(pPg);
}

void sqlite3PagerUnrefNotNull(DbPage *pPg) {
  if (pPg->flags & 0x020) {
    pagerReleaseMapPage(pPg);
  } else {
    sqlite3PcacheRelease(pPg);
  }
}

void sqlite3PagerUnref(DbPage *pPg) {
  if (pPg)
    sqlite3PagerUnrefNotNull(pPg);
}

void sqlite3PagerUnrefPageOne(DbPage *pPg) {
  Pager *pPager;

  pPager = pPg->pPager;
  sqlite3PcacheRelease(pPg);
  pagerUnlockIfUnused(pPager);
}

int sqlite3PagerPageRefcount(DbPage *pPage) {
  return sqlite3PcachePageRefcount(pPage);
}

void sqlite3PagerRekey(DbPage *pPg, Pgno iNew, u16 flags) {
  pPg->flags = flags;
  sqlite3PcacheMove(pPg, iNew);
}

void *sqlite3PagerGetData(DbPage *pPg) {
  return pPg->pData;
}

void *sqlite3PagerGetExtra(DbPage *pPg) {
  return pPg->pExtra;
}

MemPage *btreePageFromDbPage(DbPage *pDbPage, Pgno pgno, BtShared *pBt) {
  MemPage *pPage = (MemPage *)sqlite3PagerGetExtra(pDbPage);
  if (pgno != pPage->pgno) {
    pPage->aData = sqlite3PagerGetData(pDbPage);
    pPage->pDbPage = pDbPage;
    pPage->pBt = pBt;
    pPage->pgno = pgno;
    pPage->hdrOffset = pgno == 1 ? 100 : 0;
  }

  return pPage;
}

void pageReinit(DbPage *pData) {
  MemPage *pPage;
  pPage = (MemPage *)sqlite3PagerGetExtra(pData);

  if (pPage->isInit) {
    pPage->isInit = 0;
    if (sqlite3PagerPageRefcount(pData) > 1) {
      btreeInitPage(pPage);
    }
  }
}
