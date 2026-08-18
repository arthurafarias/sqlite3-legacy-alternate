
#pragma once

#include "sqlite/Wal.h"
#include "sqlite/sqlite3_file.h"
#include "sqlite/sqlite3_int64.h"

struct WalWriter;

struct WalWriter {
  Wal *pWal;
  sqlite3_file *pFd;
  sqlite3_int64 iSyncPoint;
  int syncFlags;
  int szPage;
};

int walWriteToLog(WalWriter *p, void *pContent, int iAmt, sqlite3_int64 iOffset);
int walWriteOneFrame(WalWriter *p, PgHdr *pPage, int nTruncate, sqlite3_int64 iOffset);


