
#pragma once

#include "sqlite/i64.h"
#include "sqlite/u32.h"
  typedef struct VdbeTxtBlbCache VdbeTxtBlbCache;
  struct VdbeTxtBlbCache {
    char *pCValue;
    i64 iOffset;
    int iCol;
    u32 cacheStatus;
    u32 colCacheCtr;
  };


