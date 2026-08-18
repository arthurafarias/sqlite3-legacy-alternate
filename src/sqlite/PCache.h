
#pragma once
#include "sqlite/PgHdr.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/Pgno.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3_pcache.h"
#include "sqlite/u8.h"
  typedef struct PCache PCache;

  typedef struct sqlite3_pcache_page sqlite3_pcache_page;
  typedef struct PgHdr PgHdr;

  struct PCache {
    PgHdr *pDirty, *pDirtyTail;      /* List of dirty pages in LRU order */
    PgHdr *pSynced;                  /* Last synced page in dirty page list */
    i64 nRefSum;                     /* Sum of ref counts over all pages */
    int szCache;                     /* Configured cache size */
    int szSpill;                     /* Size before spilling occurs */
    int szPage;                      /* Size of every page in this cache */
    int szExtra;                     /* Size of extra space for each page */
    u8 bPurgeable;                   /* True if pages are on backing store */
    u8 eCreate;                      /* eCreate value for for xFetch() */
    int (*xStress)(void *, PgHdr *); /* Call to try make a page clean */
    void *pStress;                   /* Argument to xStress */
    sqlite3_pcache *pCache;          /* Pluggable cache module */
  };

  sqlite3_pcache_page *sqlite3PcacheFetch(PCache *, Pgno, int createFlag);
  int sqlite3PcacheFetchStress(PCache *, Pgno, sqlite3_pcache_page **);
  PgHdr *sqlite3PcacheFetchFinish(PCache *, Pgno, sqlite3_pcache_page * pPage);
  void sqlite3PcacheCleanAll(PCache *);
  void sqlite3PcacheClearWritable(PCache *);
  void sqlite3PcacheTruncate(PCache *, Pgno x);
  PgHdr *sqlite3PcacheDirtyList(PCache *);
  void sqlite3PcacheClose(PCache *);
  void sqlite3PcacheClearSyncFlags(PCache *);
  void sqlite3PcacheClear(PCache *);
  i64 sqlite3PcacheRefCount(PCache *);
  int sqlite3PcachePagecount(PCache *);
  void sqlite3PcacheSetCachesize(PCache *, int);
  int sqlite3PcacheSetSpillsize(PCache *, int);
  void sqlite3PcacheShrink(PCache *);
  int sqlite3PCachePercentDirty(PCache *);
  int sqlite3PCacheIsDirty(PCache * pCache);
  int sqlite3PcacheSetPageSize(PCache *, int);

  void sqlite3PCacheBufferSetup(void *, int sz, int n);
  void sqlite3PCacheSetDefault(void);
  int numberOfCachePages(PCache * p);
  __attribute__((noinline)) PgHdr *pcacheFetchFinishWithInit(PCache * pCache, Pgno pgno, sqlite3_pcache_page * pPage);

#ifdef __cplusplus
}
#endif
