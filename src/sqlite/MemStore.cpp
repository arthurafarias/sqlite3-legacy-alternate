#define _GNU_SOURCE 1
#include "sqlite/MemStore.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/u64.h"
#include "sqlite/SqliteDeserializeFlags.h"
#include "sqlite/SqliteResultCode.h"
void memdbEnter(MemStore *p) {
  sqlite3_mutex_enter(p->pMutex);
}

void memdbLeave(MemStore *p) {
  sqlite3_mutex_leave(p->pMutex);
}

int memdbEnlarge(MemStore *p, sqlite3_int64 newSz) {
  unsigned char *pNew;
  if ((p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE) == 0 || (p->nMmap > 0)) {
    return SQLITE_FULL;
  }
  if (newSz > p->szMax) {
    return SQLITE_FULL;
  }
  newSz *= 2;
  if (newSz > p->szMax)
    newSz = p->szMax;
  pNew = (unsigned char*)(sqlite3Realloc(p->aData, newSz));
  if (pNew == 0)
    return (10 | (12 << 8));
  p->aData = pNew;
  p->szAlloc = newSz;
  return SQLITE_OK;
}
