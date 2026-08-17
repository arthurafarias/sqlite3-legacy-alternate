
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/sqlite3_int64.h"
#include "sqlite/u8.h"
typedef struct ReusableSpace ReusableSpace;

struct ReusableSpace {
  u8 *pSpace;
  sqlite3_int64 nFree;
  sqlite3_int64 nNeeded;
};

void *allocSpace(struct ReusableSpace *p, void *pBuf, sqlite3_int64 nByte);

#ifdef __cplusplus
}
#endif
