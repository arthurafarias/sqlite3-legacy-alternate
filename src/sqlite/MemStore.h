
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/sqlite3_int64.h"
  typedef struct sqlite3_mutex sqlite3_mutex;
  typedef struct MemStore MemStore;
  struct MemStore {
    sqlite3_int64 sz;
    sqlite3_int64 szAlloc;
    sqlite3_int64 szMax;
    unsigned char *aData;
    sqlite3_mutex *pMutex;
    int nMmap;
    unsigned mFlags;
    int nRdLock;
    int nWrLock;
    int nRef;
    char *zFName;
  };

  void memdbEnter(MemStore * p);
  void memdbLeave(MemStore * p);
  int memdbEnlarge(MemStore * p, sqlite3_int64 newSz);

#ifdef __cplusplus
}
#endif
