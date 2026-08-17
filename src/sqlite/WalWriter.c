#define _GNU_SOURCE 1

#include "sqlite/WalWriter.h"

#include "sqlite/PgHdr.h"
#include "sqlite/Pgno.h"
#include "sqlite/Wal.h"
#include "sqlite/sqlite3_file.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
int walWriteToLog(WalWriter *p, void *pContent, int iAmt, sqlite3_int64 iOffset) {
  int rc;
  if (iOffset < p->iSyncPoint && iOffset + iAmt >= p->iSyncPoint) {
    int iFirstAmt = (int)(p->iSyncPoint - iOffset);
    rc = sqlite3OsWrite(p->pFd, pContent, iFirstAmt, iOffset);
    if (rc)
      return rc;
    iOffset += iFirstAmt;
    iAmt -= iFirstAmt;
    pContent = (void *)(iFirstAmt + (char *)pContent);


    rc = sqlite3OsSync(p->pFd, ((p->syncFlags) & 0x03));
    if (iAmt == 0 || rc)
      return rc;
  }
  rc = sqlite3OsWrite(p->pFd, pContent, iAmt, iOffset);
  return rc;
}

int walWriteOneFrame(WalWriter *p, PgHdr *pPage, int nTruncate, sqlite3_int64 iOffset) {
  int rc;
  void *pData;
  u8 aFrame[24];
  pData = pPage->pData;
  walEncodeFrame(p->pWal, pPage->pgno, nTruncate, pData, aFrame);
  rc = walWriteToLog(p, aFrame, sizeof(aFrame), iOffset);
  if (rc)
    return rc;

  rc = walWriteToLog(p, pData, p->szPage, iOffset + sizeof(aFrame));
  return rc;
}
