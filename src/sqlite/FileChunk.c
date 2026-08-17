#define _GNU_SOURCE 1

#include "sqlite/FileChunk.h"

#include "sqlite/sqlite3.h"
void memjrnlFreeChunks(FileChunk *pFirst) {
  FileChunk *pIter;
  FileChunk *pNext;
  for (pIter = pFirst; pIter; pIter = pNext) {
    pNext = pIter->pNext;
    sqlite3_free(pIter);
  }
}
