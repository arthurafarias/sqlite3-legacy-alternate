
#pragma once

#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_mem_methods.h"
#include "sqlite/sqlite3_mutex_methods.h"
#include "sqlite/sqlite3_pcache_methods2.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
typedef struct sqlite3_mutex sqlite3_mutex;

struct Sqlite3Config {
  int bMemstat;
  u8 bCoreMutex;
  u8 bFullMutex;
  u8 bOpenUri;
  u8 bUseCis;
  u8 bSmallMalloc;
  u8 bExtraSchemaChecks;

  int mxStrlen;
  int neverCorrupt;
  int szLookaside;
  int nLookaside;
  int nStmtSpill;
  sqlite3_mem_methods m;
  sqlite3_mutex_methods mutex;
  sqlite3_pcache_methods2 pcache2;
  void *pHeap;
  int nHeap;
  int mnReq, mxReq;
  sqlite3_int64 szMmap;
  sqlite3_int64 mxMmap;
  void *pPage;
  int szPage;
  int nPage;
  int mxParserStack;
  int sharedCacheEnabled;
  u32 szPma;

  int isInit;
  int inProgress;
  int isMutexInit;
  int isMallocInit;
  int isPCacheInit;
  int nRefInitMutex;
  sqlite3_mutex *pInitMutex;
  void (*xLog)(void *, int, const char *);
  void *pLogArg;
  sqlite3_int64 mxMemdbSize;

  int (*xTestCallback)(int);

  int bLocaltimeFault;
  int (*xAltLocaltime)(const void *, void *);
  int iOnceResetThreshold;
  u32 szSorterRef;
  unsigned int iPrngSeed;
};

extern struct Sqlite3Config sqlite3Config;

