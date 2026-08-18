
#pragma once

#include "sqlite/Pgno.h"
#include "sqlite/WalIndexHdr.h"
#include "sqlite/i16.h"
#include "sqlite/i64.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
  struct PgHdr;
  struct WalCkptInfo;
  struct WalHashLoc;
  struct WalIterator;
  struct sqlite3;
  struct sqlite3_vfs;

  struct Wal;

  struct sqlite3_file;

  struct Wal {
    sqlite3_vfs *pVfs;
    sqlite3_file *pDbFd;
    sqlite3_file *pWalFd;
    u32 iCallback;
    i64 mxWalSize;
    int nWiData;
    int szFirstBlock;
    volatile u32 **apWiData;
    u32 szPage;
    i16 readLock;
    u8 syncFlags;
    u8 exclusiveMode;
    u8 writeLock;
    u8 ckptLock;
    u8 readOnly;
    u8 truncateOnCommit;
    u8 syncHeader;
    u8 padToSectorBoundary;
    u8 bShmUnreliable;
    WalIndexHdr hdr;
    u32 minFrame;
    u32 iReCksum;
    const char *zWalName;
    u32 nCkpt;
  };

  int sqlite3WalDefaultHook(void *, sqlite3 *, const char *, int);
  int sqlite3WalClose(Wal * pWal, sqlite3 *, int sync_flags, int, u8 *);
  void sqlite3WalLimit(Wal *, i64);
  int sqlite3WalBeginReadTransaction(Wal * pWal, int *);
  void sqlite3WalEndReadTransaction(Wal * pWal);
  int sqlite3WalFindFrame(Wal *, Pgno, u32 *);
  int sqlite3WalReadFrame(Wal *, u32, int, u8 *);
  Pgno sqlite3WalDbsize(Wal * pWal);
  int sqlite3WalBeginWriteTransaction(Wal * pWal);
  int sqlite3WalEndWriteTransaction(Wal * pWal);
  int sqlite3WalUndo(Wal * pWal, int (*xUndo)(void *, Pgno), void *pUndoCtx);
  void sqlite3WalSavepoint(Wal * pWal, u32 * aWalData);
  int sqlite3WalSavepointUndo(Wal * pWal, u32 * aWalData);
  int sqlite3WalFrames(Wal * pWal, int, PgHdr *, Pgno, int, int);
  int sqlite3WalCheckpoint(Wal * pWal, sqlite3 * db, int eMode, int (*xBusy)(void *), void *pBusyArg, int sync_flags,
                           int nBuf, u8 *zBuf, int *pnLog, int *pnCkpt);
  int sqlite3WalCallback(Wal * pWal);
  int sqlite3WalExclusiveMode(Wal * pWal, int op);
  int sqlite3WalHeapMemory(Wal * pWal);
  sqlite3_file *sqlite3WalFile(Wal * pWal);
  __attribute__((noinline)) int walIndexPageRealloc(Wal * pWal, int iPage, volatile u32 **ppPage);
  int walIndexPage(Wal * pWal, int iPage, volatile u32 **ppPage);
  volatile WalCkptInfo *walCkptInfo(Wal * pWal);
  volatile WalIndexHdr *walIndexHdr(Wal * pWal);
  void walShmBarrier(Wal * pWal);
  void walIndexWriteHdr(Wal * pWal);
  void walEncodeFrame(Wal * pWal, u32 iPage, u32 nTruncate, u8 * aData, u8 * aFrame);
  int walDecodeFrame(Wal * pWal, u32 * piPage, u32 * pnTruncate, u8 * aData, u8 * aFrame);
  int walLockShared(Wal * pWal, int lockIdx);
  void walUnlockShared(Wal * pWal, int lockIdx);
  int walLockExclusive(Wal * pWal, int lockIdx, int n);
  void walUnlockExclusive(Wal * pWal, int lockIdx, int n);
  int walHashGet(Wal * pWal, int iHash, WalHashLoc *pLoc);
  u32 walFramePgno(Wal * pWal, u32 iFrame);
  void walCleanupHash(Wal * pWal);
  int walIndexAppend(Wal * pWal, u32 iFrame, u32 iPage);
  int walIndexRecover(Wal * pWal);
  void walIndexClose(Wal * pWal, int isDelete);
  int walIteratorInit(Wal * pWal, u32 nBackfill, WalIterator * *pp);
  int walBusyLock(Wal * pWal, int (*xBusy)(void *), void *pBusyArg, int lockIdx, int n);
  int walPagesize(Wal * pWal);
  void walRestartHdr(Wal * pWal, u32 salt1);
  int walCheckpoint(Wal * pWal, sqlite3 * db, int eMode, int (*xBusy)(void *), void *pBusyArg, int sync_flags,
                    u8 *zBuf);
  void walLimitSize(Wal * pWal, i64 nMax);
  int walIndexTryHdr(Wal * pWal, int *pChanged);
  int walIndexReadHdr(Wal * pWal, int *pChanged);
  int walBeginShmUnreliable(Wal * pWal, int *pChanged);
  int walTryBeginRead(Wal * pWal, int *pChanged, int useWal, int *pCnt);
  int walBeginReadTransaction(Wal * pWal, int *pChanged);
  int walFindFrame(Wal * pWal, Pgno pgno, u32 * piRead);
  int walRestartLog(Wal * pWal);
  int walRewriteChecksums(Wal * pWal, u32 iLast);
  int walFrames(Wal * pWal, int szPage, PgHdr *pList, Pgno nTruncate, int isCommit, int sync_flags);


