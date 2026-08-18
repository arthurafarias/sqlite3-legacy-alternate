
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/Pgno.h"
#include "sqlite/i64.h"
#include "sqlite/u16.h"
#include "sqlite/u8.h"
  typedef struct PgHdr PgHdr;

  typedef struct sqlite3_pcache_page sqlite3_pcache_page;
  typedef struct PCache PCache;
  typedef struct Pager Pager;

  struct PgHdr {
    sqlite3_pcache_page *pPage;
    void *pData;
    void *pExtra;
    PCache *pCache;
    PgHdr *pDirty;
    Pager *pPager;

    Pgno pgno;
    u16 flags;

    i64 nRef;
    PgHdr *pDirtyNext;
    PgHdr *pDirtyPrev;
  };

  void sqlite3PcacheRelease(PgHdr *);
  void sqlite3PcacheDrop(PgHdr *);
  void sqlite3PcacheMakeDirty(PgHdr *);
  void sqlite3PcacheMakeClean(PgHdr *);
  void sqlite3PcacheMove(PgHdr *, Pgno);
  void sqlite3PcacheRef(PgHdr *);
  i64 sqlite3PcachePageRefcount(PgHdr *);

  void pcacheManageDirtyList(PgHdr * pPage, u8 addRemove);
  void pcacheUnpin(PgHdr * p);
  PgHdr *pcacheMergeDirtyList(PgHdr * pA, PgHdr * pB);
  PgHdr *pcacheSortDirtyList(PgHdr * pIn);
  int subjRequiresPage(PgHdr * pPg);
  int readDbPage(PgHdr * pPg);
  void pager_write_changecounter(PgHdr * pPg);
  void pagerReleaseMapPage(PgHdr * pPg);
  int subjournalPage(PgHdr * pPg);
  int subjournalPageIfRequired(PgHdr * pPg);
  __attribute__((noinline)) int pagerAddPageToRollbackJournal(PgHdr * pPg);
  int pager_write(PgHdr * pPg);
  __attribute__((noinline)) int pagerWriteLargeSector(PgHdr * pPg);

#ifdef __cplusplus
}
#endif
