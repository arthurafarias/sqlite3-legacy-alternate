#define _GNU_SOURCE 1

#include "sqlite/ReusableSpace.h"

#include "sqlite/sqlite3_int64.h"
#include "sqlite/u8.h"
void *allocSpace(struct ReusableSpace *p, void *pBuf, sqlite3_int64 nByte) {

  if (pBuf == 0) {
    nByte = (nByte);
    if (nByte <= p->nFree) {
      p->nFree -= nByte;
      pBuf = &p->pSpace[p->nFree];
    } else {
      p->nNeeded += nByte;
    }
  }

  return pBuf;
}
