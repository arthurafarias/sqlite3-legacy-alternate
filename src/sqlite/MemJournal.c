#define _GNU_SOURCE 1

#include <string.h>

#include "sqlite/MemJournal.h"

#include "sqlite/FileChunk.h"
#include "sqlite/FilePoint.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3_file.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_vfs.h"
#include "sqlite/u8.h"
int memjrnlCreateFile(MemJournal *p) {
  int rc;
  sqlite3_file *pReal = (sqlite3_file *)p;
  MemJournal copy = *p;

  memset(p, 0, sizeof(MemJournal));
  rc = sqlite3OsOpen(copy.pVfs, copy.zJournal, pReal, copy.flags, 0);
  if (rc == 0) {
    int nChunk = copy.nChunkSize;
    i64 iOff = 0;
    FileChunk *pIter;
    for (pIter = copy.pFirst; pIter; pIter = pIter->pNext) {
      if (iOff + nChunk > copy.endpoint.iOffset) {
        nChunk = copy.endpoint.iOffset - iOff;
      }
      rc = sqlite3OsWrite(pReal, (u8 *)pIter->zChunk, nChunk, iOff);
      if (rc)
        break;
      iOff += nChunk;
    }
    if (rc == 0) {

      memjrnlFreeChunks(copy.pFirst);
    }
  }
  if (rc != 0) {

    sqlite3OsClose(pReal);
    *p = copy;
  }
  return rc;
}
