
#pragma once

#ifdef __cplusplus
extern C {
#endif

  typedef struct sqlite3_pcache sqlite3_pcache;

  typedef struct sqlite3_pcache_methods sqlite3_pcache_methods;

  struct sqlite3_pcache_methods {
    void *pArg;
    int (*xInit)(void *);
    void (*xShutdown)(void *);
    sqlite3_pcache *(*xCreate)(int szPage, int bPurgeable);
    void (*xCachesize)(sqlite3_pcache *, int nCachesize);
    int (*xPagecount)(sqlite3_pcache *);
    void *(*xFetch)(sqlite3_pcache *, unsigned key, int createFlag);
    void (*xUnpin)(sqlite3_pcache *, void *, int discard);
    void (*xRekey)(sqlite3_pcache *, void *, unsigned oldKey, unsigned newKey);
    void (*xTruncate)(sqlite3_pcache *, unsigned iLimit);
    void (*xDestroy)(sqlite3_pcache *);
  };

#ifdef __cplusplus
}
#endif
