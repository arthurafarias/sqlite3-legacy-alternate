
#pragma once
#ifdef __cplusplus
extern C {
#endif
  typedef struct PgHdr1 PgHdr1;
  typedef struct sqlite3_pcache_page sqlite3_pcache_page;

  typedef struct sqlite3_pcache sqlite3_pcache;

  void pcache1Destroy(sqlite3_pcache * p);
  void pcache1Cachesize(sqlite3_pcache * p, int nMax);
  void pcache1Shrink(sqlite3_pcache * p);
  int pcache1Pagecount(sqlite3_pcache * p);
  PgHdr1 *pcache1FetchNoMutex(sqlite3_pcache * p, unsigned int iKey, int createFlag);
  sqlite3_pcache_page *pcache1Fetch(sqlite3_pcache * p, unsigned int iKey, int createFlag);
  void pcache1Unpin(sqlite3_pcache * p, sqlite3_pcache_page * pPg, int reuseUnlikely);
  void pcache1Rekey(sqlite3_pcache * p, sqlite3_pcache_page * pPg, unsigned int iOld, unsigned int iNew);
  void pcache1Truncate(sqlite3_pcache * p, unsigned int iLimit);

#ifdef __cplusplus
}
#endif
