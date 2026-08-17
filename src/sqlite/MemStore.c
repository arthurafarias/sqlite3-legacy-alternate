#define _GNU_SOURCE 1

#include "sqlite/MemStore.h"

#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/u64.h"
void memdbEnter(MemStore *p) { sqlite3_mutex_enter(p->pMutex); }

void memdbLeave(MemStore *p) { sqlite3_mutex_leave(p->pMutex); }

int memdbEnlarge(MemStore *p, sqlite3_int64 newSz) {
  unsigned char *pNew;
  if ((p->mFlags & 2) == 0 || (p->nMmap > 0)) {
    return 13;
  }
  if (newSz > p->szMax) {
    return 13;
  }
  newSz *= 2;
  if (newSz > p->szMax)
    newSz = p->szMax;
  pNew = sqlite3Realloc(p->aData, newSz);
  if (pNew == 0)
    return (10 | (12 << 8));
  p->aData = pNew;
  p->szAlloc = newSz;
  return 0;
}
