#define _GNU_SOURCE 1
#include <string.h>
#include "sqlite/PmaWriter.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_file.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
void vdbePmaWriteBlob(PmaWriter *p, u8 *pData, int nData) {
  int nRem = nData;
  while (nRem > 0 && p->eFWErr == 0) {
    int nCopy = nRem;
    if (nCopy > (p->nBuffer - p->iBufEnd)) {
      nCopy = p->nBuffer - p->iBufEnd;
    }

    memcpy(&p->aBuffer[p->iBufEnd], &pData[nData - nRem], nCopy);
    p->iBufEnd += nCopy;
    if (p->iBufEnd == p->nBuffer) {
      p->eFWErr =
          sqlite3OsWrite(p->pFd, &p->aBuffer[p->iBufStart], p->iBufEnd - p->iBufStart, p->iWriteOff + p->iBufStart);
      p->nPmaSpill += (p->iBufEnd - p->iBufStart);
      p->iBufStart = p->iBufEnd = 0;
      p->iWriteOff += p->nBuffer;
    }

    nRem -= nCopy;
  }
}

int vdbePmaWriterFinish(PmaWriter *p, i64 *piEof, u64 *pnSpill) {
  int rc;
  if (p->eFWErr == 0 && (p->aBuffer) && p->iBufEnd > p->iBufStart) {
    p->eFWErr =
        sqlite3OsWrite(p->pFd, &p->aBuffer[p->iBufStart], p->iBufEnd - p->iBufStart, p->iWriteOff + p->iBufStart);
    p->nPmaSpill += (p->iBufEnd - p->iBufStart);
  }
  *piEof = (p->iWriteOff + p->iBufEnd);
  *pnSpill += p->nPmaSpill;
  sqlite3_free(p->aBuffer);
  rc = p->eFWErr;
  memset(p, 0, sizeof(PmaWriter));
  return rc;
}

void vdbePmaWriteVarint(PmaWriter *p, u64 iVal) {
  int nByte;
  u8 aByte[10];
  nByte = sqlite3PutVarint(aByte, iVal);
  vdbePmaWriteBlob(p, aByte, nByte);
}
