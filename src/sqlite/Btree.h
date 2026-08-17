
#pragma once
#ifdef __cplusplus
extern C {
#endif

#include "sqlite/Pgno.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"

#include "sqlite/BtLock.h"
#include "sqlite/i64.h"
  typedef struct Pager Pager;

  typedef struct BtCursor BtCursor;
  typedef struct Btree Btree;
  typedef struct sqlite3 sqlite3;
  typedef struct BtShared BtShared;
  typedef struct KeyInfo KeyInfo;

  struct Btree {
    sqlite3 *db;
    BtShared *pBt;
    u8 inTrans;
    u8 sharable;
    u8 locked;
    u8 hasIncrblobCur;
    int wantToLock;
    int nBackup;
    u32 iBDataVersion;
    Btree *pNext;
    Btree *pPrev;
    BtLock lock;
  };

  int sqlite3BtreeClose(Btree *);
  int sqlite3BtreeSetCacheSize(Btree *, int);
  int sqlite3BtreeSetSpillSize(Btree *, int);
  int sqlite3BtreeSetMmapLimit(Btree *, sqlite3_int64);
  int sqlite3BtreeSetPagerFlags(Btree *, unsigned);
  int sqlite3BtreeSetPageSize(Btree * p, int nPagesize, int nReserve, int eFix);
  int sqlite3BtreeGetPageSize(Btree *);
  Pgno sqlite3BtreeMaxPageCount(Btree *, Pgno);
  Pgno sqlite3BtreeLastPage(Btree *);
  int sqlite3BtreeSecureDelete(Btree *, int);
  int sqlite3BtreeGetRequestedReserve(Btree *);
  int sqlite3BtreeGetReserveNoMutex(Btree * p);
  int sqlite3BtreeSetAutoVacuum(Btree *, int);
  int sqlite3BtreeGetAutoVacuum(Btree *);
  int sqlite3BtreeBeginTrans(Btree *, int, int *);
  int sqlite3BtreeCommitPhaseOne(Btree *, const char *);
  int sqlite3BtreeCommitPhaseTwo(Btree *, int);
  int sqlite3BtreeCommit(Btree *);
  int sqlite3BtreeRollback(Btree *, int, int);
  int sqlite3BtreeBeginStmt(Btree *, int);
  int sqlite3BtreeCreateTable(Btree *, Pgno *, int flags);
  int sqlite3BtreeTxnState(Btree *);
  int sqlite3BtreeIsInBackup(Btree *);
  void *sqlite3BtreeSchema(Btree *, int, void (*)(void *));
  int sqlite3BtreeSchemaLocked(Btree * pBtree);
  int sqlite3BtreeLockTable(Btree * pBtree, int iTab, u8 isWriteLock);
  int sqlite3BtreeSavepoint(Btree *, int, int);
  int sqlite3BtreeCheckpoint(Btree *, int, int *, int *);
  const char *sqlite3BtreeGetFilename(Btree *);
  const char *sqlite3BtreeGetJournalname(Btree *);
  int sqlite3BtreeCopyFile(Btree *, Btree *);
  int sqlite3BtreeIncrVacuum(Btree *);
  int sqlite3BtreeDropTable(Btree *, int, int *);
  int sqlite3BtreeClearTable(Btree *, int, i64 *);
  int sqlite3BtreeTripAllCursors(Btree *, int, int);
  void sqlite3BtreeGetMeta(Btree * pBtree, int idx, u32 *pValue);
  int sqlite3BtreeUpdateMeta(Btree *, int idx, u32 value);
  int sqlite3BtreeNewDb(Btree * p);
  int sqlite3BtreeCursor(Btree *, Pgno iTable, int wrFlag, struct KeyInfo *, BtCursor *pCursor);

  struct Pager *sqlite3BtreePager(Btree *);
  int sqlite3BtreeSetVersion(Btree * pBt, int iVersion);
  int sqlite3BtreeIsReadonly(Btree * pBt);
  int sqlite3BtreeCheckpoint(Btree *, int, int *, int *);
  void sqlite3BtreeClearCache(Btree *);
  void sqlite3BtreeEnter(Btree *);
  int sqlite3BtreeSharable(Btree *);
  int sqlite3BtreeConnectionCount(Btree *);
  void sqlite3BtreeLeave(Btree *);

  int sqlite3BtreeCursorSize(void);
  void lockBtreeMutex(Btree * p);
  void __attribute__((noinline)) unlockBtreeMutex(Btree * p);
  void __attribute__((noinline)) btreeLockCarefully(Btree * p);
  int querySharedCacheTableLock(Btree * p, Pgno iTab, u8 eLock);
  int setSharedCacheTableLock(Btree * p, Pgno iTable, u8 eLock);
  void clearAllSharedCacheTableLocks(Btree * p);
  void downgradeAllSharedCacheTableLocks(Btree * p);
  void invalidateIncrblobCursors(Btree * pBtree, Pgno pgnoRoot, i64 iRow, int isClearTable);
  int btreeInvokeBusyHandler(void *pArg);
  __attribute__((noinline)) int btreeBeginTrans(Btree * p, int wrflag, int *pSchemaVersion);
  int autoVacuumCommit(Btree * p);
  void btreeEndTransaction(Btree * p);
  int btreeCursor(Btree * p, Pgno iTable, int wrFlag, struct KeyInfo *pKeyInfo, BtCursor *pCur);
  int btreeCursorWithLock(Btree * p, Pgno iTable, int wrFlag, struct KeyInfo *pKeyInfo, BtCursor *pCur);
  int btreeCreateTable(Btree * p, Pgno * piTable, int createTabFlags);
  int btreeDropTable(Btree * p, Pgno iTable, int *piMoved);
  void btreeHeapInsert(u32 * aHeap, u32 x);
  int btreeHeapPull(u32 * aHeap, u32 * pOut);
  int setDestPgsz(Btree * pDest, Btree * pSrc);

#ifdef __cplusplus
}
#endif
