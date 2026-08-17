#define _GNU_SOURCE 1
#include <string.h>
#include "sqlite/Btree.h"
#include "sqlite/BtCursor.h"
#include "sqlite/BtLock.h"
#include "sqlite/BtShared.h"
#include "sqlite/BusyHandler.h"
#include "sqlite/CellInfo.h"
#include "sqlite/Db.h"
#include "sqlite/DbPage.h"
#include "sqlite/KeyInfo.h"
#include "sqlite/MemPage.h"
#include "sqlite/Pager.h"
#include "sqlite/Pgno.h"
#include "sqlite/i64.h"
#include "sqlite/i8.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_backup.h"
#include "sqlite/sqlite3_file.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_io_methods.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/SqliteFileControlOpcode.h"
#include "sqlite/SqliteResultCode.h"
void lockBtreeMutex(Btree *p) {
  sqlite3_mutex_enter(p->pBt->mutex);
  p->pBt->db = p->db;
  p->locked = 1;
}

void __attribute__((noinline)) unlockBtreeMutex(Btree *p) {
  BtShared *pBt = p->pBt;

  sqlite3_mutex_leave(pBt->mutex);
  p->locked = 0;
}

void sqlite3BtreeEnter(Btree *p) {
  if (!p->sharable)
    return;
  p->wantToLock++;
  if (p->locked)
    return;
  btreeLockCarefully(p);
}

void __attribute__((noinline)) btreeLockCarefully(Btree *p) {
  Btree *pLater;

  if (sqlite3_mutex_try(p->pBt->mutex) == SQLITE_OK) {
    p->pBt->db = p->db;
    p->locked = 1;
    return;
  }

  for (pLater = p->pNext; pLater; pLater = pLater->pNext) {
    if (pLater->locked) {
      unlockBtreeMutex(pLater);
    }
  }
  lockBtreeMutex(p);
  for (pLater = p->pNext; pLater; pLater = pLater->pNext) {
    if (pLater->wantToLock) {
      lockBtreeMutex(pLater);
    }
  }
}

void sqlite3BtreeLeave(Btree *p) {
  if (p->sharable) {
    p->wantToLock--;
    if (p->wantToLock == 0) {
      unlockBtreeMutex(p);
    }
  }
}

int querySharedCacheTableLock(Btree *p, Pgno iTab, u8 eLock) {
  BtShared *pBt = p->pBt;
  BtLock *pIter;

  if (!p->sharable) {
    return SQLITE_OK;
  }

  if (pBt->pWriter != p && (pBt->btsFlags & 0x0040) != 0) {
    return (6 | (1 << 8));
  }

  for (pIter = pBt->pLock; pIter; pIter = pIter->pNext) {
    if (pIter->pBtree != p && pIter->iTable == iTab && pIter->eLock != eLock) {
      if (eLock == 2) {
        pBt->btsFlags |= 0x0080;
      }
      return (6 | (1 << 8));
    }
  }
  return SQLITE_OK;
}

int setSharedCacheTableLock(Btree *p, Pgno iTable, u8 eLock) {
  BtShared *pBt = p->pBt;
  BtLock *pLock = 0;
  BtLock *pIter;

  for (pIter = pBt->pLock; pIter; pIter = pIter->pNext) {
    if (pIter->iTable == iTable && pIter->pBtree == p) {
      pLock = pIter;
      break;
    }
  }

  if (!pLock) {
    pLock = (BtLock *)sqlite3MallocZero(sizeof(BtLock));
    if (!pLock) {
      return 7;
    }
    pLock->iTable = iTable;
    pLock->pBtree = p;
    pLock->pNext = pBt->pLock;
    pBt->pLock = pLock;
  }

  if (eLock > pLock->eLock) {
    pLock->eLock = eLock;
  }

  return SQLITE_OK;
}

void clearAllSharedCacheTableLocks(Btree *p) {
  BtShared *pBt = p->pBt;
  BtLock **ppIter = &pBt->pLock;

  while (*ppIter) {
    BtLock *pLock = *ppIter;

    if (pLock->pBtree == p) {
      *ppIter = pLock->pNext;

      if (pLock->iTable != 1) {
        sqlite3_free(pLock);
      }
    } else {
      ppIter = &pLock->pNext;
    }
  }

  if (pBt->pWriter == p) {
    pBt->pWriter = 0;
    pBt->btsFlags &= ~(0x0040 | 0x0080);
  } else if (pBt->nTransaction == 2) {
    pBt->btsFlags &= ~0x0080;
  }
}

void downgradeAllSharedCacheTableLocks(Btree *p) {
  BtShared *pBt = p->pBt;

  if (pBt->pWriter == p) {
    BtLock *pLock;
    pBt->pWriter = 0;
    pBt->btsFlags &= ~(0x0040 | 0x0080);
    for (pLock = pBt->pLock; pLock; pLock = pLock->pNext) {
      pLock->eLock = 1;
    }
  }
}

void invalidateIncrblobCursors(Btree *pBtree, Pgno pgnoRoot, i64 iRow, int isClearTable) {
  BtCursor *p;

  pBtree->hasIncrblobCur = 0;
  for (p = pBtree->pBt->pCursor; p; p = p->pNext) {
    if ((p->curFlags & 0x10) != 0) {
      pBtree->hasIncrblobCur = 1;
      if (p->pgnoRoot == pgnoRoot && (isClearTable || p->info.nKey == iRow)) {
        p->eState = 1;
      }
    }
  }
}

BtCursor *sqlite3BtreeFakeValidCursor(void) {
  static u8 fakeCursor = 0;

  return (BtCursor *)&fakeCursor;
}

Pgno sqlite3BtreeLastPage(Btree *p) {
  return btreePagecount(p->pBt);
}

int btreeInvokeBusyHandler(void *pArg) {
  BtShared *pBt = (BtShared *)pArg;

  return sqlite3InvokeBusyHandler(&pBt->db->busyHandler);
}

int sqlite3BtreeClose(Btree *p) {
  BtShared *pBt = p->pBt;

  sqlite3BtreeEnter(p);

  sqlite3BtreeRollback(p, SQLITE_OK, 0);
  sqlite3BtreeLeave(p);

  if (!p->sharable || removeFromSharingList(pBt)) {
    sqlite3PagerClose(pBt->pPager, p->db);
    if (pBt->xFreeSchema && pBt->pSchema) {
      pBt->xFreeSchema(pBt->pSchema);
    }
    sqlite3DbFree(0, pBt->pSchema);
    freeTempSpace(pBt);
    sqlite3_free(pBt);
  }

  if (p->pPrev)
    p->pPrev->pNext = p->pNext;
  if (p->pNext)
    p->pNext->pPrev = p->pPrev;

  sqlite3_free(p);
  return SQLITE_OK;
}

int sqlite3BtreeSetCacheSize(Btree *p, int mxPage) {
  BtShared *pBt = p->pBt;

  sqlite3BtreeEnter(p);
  sqlite3PagerSetCachesize(pBt->pPager, mxPage);
  sqlite3BtreeLeave(p);
  return SQLITE_OK;
}

int sqlite3BtreeSetSpillSize(Btree *p, int mxPage) {
  BtShared *pBt = p->pBt;
  int res;

  sqlite3BtreeEnter(p);
  res = sqlite3PagerSetSpillsize(pBt->pPager, mxPage);
  sqlite3BtreeLeave(p);
  return res;
}

int sqlite3BtreeSetMmapLimit(Btree *p, sqlite3_int64 szMmap) {
  BtShared *pBt = p->pBt;

  sqlite3BtreeEnter(p);
  sqlite3PagerSetMmapLimit(pBt->pPager, szMmap);
  sqlite3BtreeLeave(p);
  return SQLITE_OK;
}

int sqlite3BtreeSetPagerFlags(Btree *p, unsigned pgFlags) {
  BtShared *pBt = p->pBt;

  sqlite3BtreeEnter(p);
  sqlite3PagerSetFlags(pBt->pPager, pgFlags);
  sqlite3BtreeLeave(p);
  return SQLITE_OK;
}

int sqlite3BtreeSetPageSize(Btree *p, int pageSize, int nReserve, int iFix) {
  int rc = SQLITE_OK;
  int x;
  BtShared *pBt = p->pBt;

  sqlite3BtreeEnter(p);
  pBt->nReserveWanted = (u8)nReserve;
  x = pBt->pageSize - pBt->usableSize;
  if (x == nReserve && (pageSize == 0 || (u32)pageSize == pBt->pageSize)) {
    sqlite3BtreeLeave(p);
    return SQLITE_OK;
  }
  if (nReserve < x)
    nReserve = x;
  if (pBt->btsFlags & 0x0002) {
    sqlite3BtreeLeave(p);
    return SQLITE_READONLY;
  }

  if (pageSize >= 512 && pageSize <= 65536 && ((pageSize - 1) & pageSize) == 0) {
    if (nReserve > 32 && pageSize == 512)
      pageSize = 1024;
    pBt->pageSize = (u32)pageSize;
    freeTempSpace(pBt);
  }
  rc = sqlite3PagerSetPagesize(pBt->pPager, &pBt->pageSize, nReserve);
  pBt->usableSize = pBt->pageSize - (u16)nReserve;
  if (iFix)
    pBt->btsFlags |= 0x0002;
  sqlite3BtreeLeave(p);
  return rc;
}

int sqlite3BtreeGetPageSize(Btree *p) {
  return p->pBt->pageSize;
}

int sqlite3BtreeGetReserveNoMutex(Btree *p) {
  int n;

  n = p->pBt->pageSize - p->pBt->usableSize;
  return n;
}

int sqlite3BtreeGetRequestedReserve(Btree *p) {
  int n1, n2;
  sqlite3BtreeEnter(p);
  n1 = (int)p->pBt->nReserveWanted;
  n2 = sqlite3BtreeGetReserveNoMutex(p);
  sqlite3BtreeLeave(p);
  return n1 > n2 ? n1 : n2;
}

Pgno sqlite3BtreeMaxPageCount(Btree *p, Pgno mxPage) {
  Pgno n;
  sqlite3BtreeEnter(p);
  n = sqlite3PagerMaxPageCount(p->pBt->pPager, mxPage);
  sqlite3BtreeLeave(p);
  return n;
}

int sqlite3BtreeSecureDelete(Btree *p, int newFlag) {
  int b;
  if (p == 0)
    return 0;
  sqlite3BtreeEnter(p);

  if (newFlag >= 0) {
    p->pBt->btsFlags &= ~0x000c;
    p->pBt->btsFlags |= (u16)(0x0004 * newFlag);
  }
  b = (p->pBt->btsFlags & 0x000c) / 0x0004;
  sqlite3BtreeLeave(p);
  return b;
}

int sqlite3BtreeSetAutoVacuum(Btree *p, int autoVacuum) {
  BtShared *pBt = p->pBt;
  int rc = SQLITE_OK;
  u8 av = (u8)autoVacuum;

  sqlite3BtreeEnter(p);
  if ((pBt->btsFlags & 0x0002) != 0 && (av ? 1 : 0) != pBt->autoVacuum) {
    rc = SQLITE_READONLY;
  } else {
    pBt->autoVacuum = av ? 1 : 0;
    pBt->incrVacuum = av == 2 ? 1 : 0;
  }
  sqlite3BtreeLeave(p);
  return rc;
}

int sqlite3BtreeGetAutoVacuum(Btree *p) {
  int rc;
  sqlite3BtreeEnter(p);
  rc = ((!p->pBt->autoVacuum) ? 0 : (!p->pBt->incrVacuum) ? 1 : 2);
  sqlite3BtreeLeave(p);
  return rc;
}

int sqlite3BtreeNewDb(Btree *p) {
  int rc;
  sqlite3BtreeEnter(p);
  p->pBt->nPage = 0;
  rc = newDatabase(p->pBt);
  sqlite3BtreeLeave(p);
  return rc;
}

__attribute__((noinline)) int btreeBeginTrans(Btree *p, int wrflag, int *pSchemaVersion) {
  BtShared *pBt = p->pBt;
  Pager *pPager = pBt->pPager;
  int rc = SQLITE_OK;

  sqlite3BtreeEnter(p);

  if (p->inTrans == 2 || (p->inTrans == 1 && !wrflag)) {
    goto trans_begun;
  }

  if ((p->db->flags & 0x02000000) && sqlite3PagerIsreadonly(pPager) == 0) {
    pBt->btsFlags &= ~0x0001;
  }

  if ((pBt->btsFlags & 0x0001) != 0 && wrflag) {
    rc = SQLITE_READONLY;
    goto trans_begun;
  }

  {
    sqlite3 *pBlock = 0;

    if ((wrflag && pBt->inTransaction == 2) || (pBt->btsFlags & 0x0080) != 0) {
      pBlock = pBt->pWriter->db;
    } else if (wrflag > 1) {
      BtLock *pIter;
      for (pIter = pBt->pLock; pIter; pIter = pIter->pNext) {
        if (pIter->pBtree != p) {
          pBlock = pIter->pBtree->db;
          break;
        }
      }
    }
    if (pBlock) {
      rc = (6 | (1 << 8));
      goto trans_begun;
    }
  }

  rc = querySharedCacheTableLock(p, 1, 1);
  if (SQLITE_OK != rc)
    goto trans_begun;

  pBt->btsFlags &= ~0x0010;
  if (pBt->nPage == 0)
    pBt->btsFlags |= 0x0010;
  do {
    while (pBt->pPage1 == 0 && SQLITE_OK == (rc = lockBtree(pBt)))
      ;

    if (rc == SQLITE_OK && wrflag) {
      if ((pBt->btsFlags & 0x0001) != 0) {
        rc = SQLITE_READONLY;
      } else {
        rc = sqlite3PagerBegin(pPager, wrflag > 1, sqlite3TempInMemory(p->db));
        if (rc == SQLITE_OK) {
          rc = newDatabase(pBt);
        } else if (rc == (5 | (2 << 8)) && pBt->inTransaction == 0) {
          rc = SQLITE_BUSY;
        }
      }
    }

    if (rc != SQLITE_OK) {
      (void)0;
      unlockBtreeIfUnused(pBt);
    }

  } while ((rc & 0xFF) == SQLITE_BUSY && pBt->inTransaction == 0 && btreeInvokeBusyHandler(pBt));

  if (rc == 0) {
    if (p->inTrans == 0) {
      pBt->nTransaction++;

      if (p->sharable) {
        p->lock.eLock = 1;
        p->lock.pNext = pBt->pLock;
        pBt->pLock = &p->lock;
      }
    }
    p->inTrans = (wrflag ? 2 : 1);
    if (p->inTrans > pBt->inTransaction) {
      pBt->inTransaction = p->inTrans;
    }
    if (wrflag) {
      MemPage *pPage1 = pBt->pPage1;

      pBt->pWriter = p;
      pBt->btsFlags &= ~0x0040;
      if (wrflag > 1)
        pBt->btsFlags |= 0x0040;

      if (pBt->nPage != sqlite3Get4byte(&pPage1->aData[28])) {
        rc = sqlite3PagerWrite(pPage1->pDbPage);
        if (rc == SQLITE_OK) {
          sqlite3Put4byte(&pPage1->aData[28], pBt->nPage);
        }
      }
    }
  }

trans_begun:
  if (rc == SQLITE_OK) {
    if (pSchemaVersion) {
      *pSchemaVersion = sqlite3Get4byte(&pBt->pPage1->aData[40]);
    }
    if (wrflag) {
      rc = sqlite3PagerOpenSavepoint(pPager, p->db->nSavepoint);
    }
  }

  sqlite3BtreeLeave(p);
  return rc;
}

int sqlite3BtreeBeginTrans(Btree *p, int wrflag, int *pSchemaVersion) {
  BtShared *pBt;
  if (p->sharable || p->inTrans == 0 || (p->inTrans == 1 && wrflag != 0)) {
    return btreeBeginTrans(p, wrflag, pSchemaVersion);
  }
  pBt = p->pBt;
  if (pSchemaVersion) {
    *pSchemaVersion = sqlite3Get4byte(&pBt->pPage1->aData[40]);
  }
  if (wrflag) {
    return sqlite3PagerOpenSavepoint(pBt->pPager, p->db->nSavepoint);
  } else {
    return SQLITE_OK;
  }
}

int sqlite3BtreeIncrVacuum(Btree *p) {
  int rc;
  BtShared *pBt = p->pBt;

  sqlite3BtreeEnter(p);

  if (!pBt->autoVacuum) {
    rc = SQLITE_DONE;
  } else {
    Pgno nOrig = btreePagecount(pBt);
    Pgno nFree = sqlite3Get4byte(&pBt->pPage1->aData[36]);
    Pgno nFin = finalDbSize(pBt, nOrig, nFree);

    if (nOrig < nFin || nFree >= nOrig) {
      rc = sqlite3CorruptError(77404);
    } else if (nFree > 0) {
      rc = saveAllCursors(pBt, 0, 0);
      if (rc == SQLITE_OK) {
        invalidateAllOverflowCache(pBt);
        rc = incrVacuumStep(pBt, nFin, nOrig, 0);
      }
      if (rc == SQLITE_OK) {
        rc = sqlite3PagerWrite(pBt->pPage1->pDbPage);
        sqlite3Put4byte(&pBt->pPage1->aData[28], pBt->nPage);
      }
    } else {
      rc = SQLITE_DONE;
    }
  }
  sqlite3BtreeLeave(p);
  return rc;
}

int autoVacuumCommit(Btree *p) {
  int rc = SQLITE_OK;
  Pager *pPager;
  BtShared *pBt;
  sqlite3 *db;

  pBt = p->pBt;
  pPager = pBt->pPager;

  invalidateAllOverflowCache(pBt);

  if (!pBt->incrVacuum) {
    Pgno nFin;
    Pgno nFree;
    Pgno nVac;
    Pgno iFree;
    Pgno nOrig;

    nOrig = btreePagecount(pBt);
    if ((ptrmapPageno((pBt), (nOrig)) == (nOrig)) || nOrig == ((Pgno)((sqlite3PendingByte / ((pBt)->pageSize)) + 1))) {
      return sqlite3CorruptError(77455);
    }

    nFree = sqlite3Get4byte(&pBt->pPage1->aData[36]);
    db = p->db;
    if (db->xAutovacPages) {
      int iDb;
      for (iDb = 0; (iDb < db->nDb); iDb++) {
        if (db->aDb[iDb].pBt == p)
          break;
      }
      nVac = db->xAutovacPages(db->pAutovacPagesArg, db->aDb[iDb].zDbSName, nOrig, nFree, pBt->pageSize);
      if (nVac > nFree) {
        nVac = nFree;
      }
      if (nVac == 0) {
        return SQLITE_OK;
      }
    } else {
      nVac = nFree;
    }
    nFin = finalDbSize(pBt, nOrig, nVac);
    if (nFin > nOrig)
      return sqlite3CorruptError(77482);
    if (nFin < nOrig) {
      rc = saveAllCursors(pBt, 0, 0);
    }
    for (iFree = nOrig; iFree > nFin && rc == SQLITE_OK; iFree--) {
      rc = incrVacuumStep(pBt, nFin, iFree, nVac == nFree);
    }
    if ((rc == SQLITE_DONE || rc == SQLITE_OK) && nFree > 0) {
      rc = sqlite3PagerWrite(pBt->pPage1->pDbPage);
      if (nVac == nFree) {
        sqlite3Put4byte(&pBt->pPage1->aData[32], 0);
        sqlite3Put4byte(&pBt->pPage1->aData[36], 0);
      }
      sqlite3Put4byte(&pBt->pPage1->aData[28], nFin);
      pBt->bDoTruncate = 1;
      pBt->nPage = nFin;
    }
    if (rc != SQLITE_OK) {
      sqlite3PagerRollback(pPager);
    }
  }

  return rc;
}

int sqlite3BtreeCommitPhaseOne(Btree *p, const char *zSuperJrnl) {
  int rc = SQLITE_OK;
  if (p->inTrans == 2) {
    BtShared *pBt = p->pBt;
    sqlite3BtreeEnter(p);

    if (pBt->autoVacuum) {
      rc = autoVacuumCommit(p);
      if (rc != SQLITE_OK) {
        sqlite3BtreeLeave(p);
        return rc;
      }
    }
    if (pBt->bDoTruncate) {
      sqlite3PagerTruncateImage(pBt->pPager, pBt->nPage);
    }

    rc = sqlite3PagerCommitPhaseOne(pBt->pPager, zSuperJrnl, 0);
    sqlite3BtreeLeave(p);
  }
  return rc;
}

void btreeEndTransaction(Btree *p) {
  BtShared *pBt = p->pBt;
  sqlite3 *db = p->db;

  pBt->bDoTruncate = 0;

  if (p->inTrans > 0 && db->nVdbeRead > 1) {
    downgradeAllSharedCacheTableLocks(p);
    p->inTrans = 1;
  } else {
    if (p->inTrans != 0) {
      clearAllSharedCacheTableLocks(p);
      pBt->nTransaction--;
      if (0 == pBt->nTransaction) {
        pBt->inTransaction = 0;
      }
    }

    p->inTrans = 0;
    unlockBtreeIfUnused(pBt);
  }
}

int sqlite3BtreeCommitPhaseTwo(Btree *p, int bCleanup) {
  if (p->inTrans == 0)
    return SQLITE_OK;
  sqlite3BtreeEnter(p);

  if (p->inTrans == 2) {
    int rc;
    BtShared *pBt = p->pBt;

    rc = sqlite3PagerCommitPhaseTwo(pBt->pPager);
    if (rc != SQLITE_OK && bCleanup == 0) {
      sqlite3BtreeLeave(p);
      return rc;
    }
    p->iBDataVersion--;
    pBt->inTransaction = 1;
    btreeClearHasContent(pBt);
  }

  btreeEndTransaction(p);
  sqlite3BtreeLeave(p);
  return SQLITE_OK;
}

int sqlite3BtreeCommit(Btree *p) {
  int rc;
  sqlite3BtreeEnter(p);
  rc = sqlite3BtreeCommitPhaseOne(p, 0);
  if (rc == SQLITE_OK) {
    rc = sqlite3BtreeCommitPhaseTwo(p, 0);
  }
  sqlite3BtreeLeave(p);
  return rc;
}

int sqlite3BtreeTripAllCursors(Btree *pBtree, int errCode, int writeOnly) {
  BtCursor *p;
  int rc = 0;

  if (pBtree) {
    sqlite3BtreeEnter(pBtree);
    for (p = pBtree->pBt->pCursor; p; p = p->pNext) {
      if (writeOnly && (p->curFlags & 0x01) == 0) {
        if (p->eState == 0 || p->eState == 2) {
          rc = saveCursorPosition(p);
          if (rc != SQLITE_OK) {
            (void)sqlite3BtreeTripAllCursors(pBtree, rc, 0);
            break;
          }
        }
      } else {
        sqlite3BtreeClearCursor(p);
        p->eState = 4;
        p->skipNext = errCode;
      }
      btreeReleaseAllCursorPages(p);
    }
    sqlite3BtreeLeave(pBtree);
  }
  return rc;
}

int sqlite3BtreeRollback(Btree *p, int tripCode, int writeOnly) {
  int rc;
  BtShared *pBt = p->pBt;
  MemPage *pPage1;

  sqlite3BtreeEnter(p);
  if (tripCode == SQLITE_OK) {
    rc = tripCode = saveAllCursors(pBt, 0, 0);
    if (rc)
      writeOnly = 0;
  } else {
    rc = SQLITE_OK;
  }
  if (tripCode) {
    int rc2 = sqlite3BtreeTripAllCursors(p, tripCode, writeOnly);

    if (rc2 != SQLITE_OK)
      rc = rc2;
  }

  if (p->inTrans == 2) {
    int rc2;

    rc2 = sqlite3PagerRollback(pBt->pPager);
    if (rc2 != SQLITE_OK) {
      rc = rc2;
    }

    if (btreeGetPage(pBt, 1, &pPage1, 0) == SQLITE_OK) {
      btreeSetNPage(pBt, pPage1);
      releasePageOne(pPage1);
    }

    pBt->inTransaction = 1;
    btreeClearHasContent(pBt);
  }

  btreeEndTransaction(p);
  sqlite3BtreeLeave(p);
  return rc;
}

int sqlite3BtreeBeginStmt(Btree *p, int iStatement) {
  int rc;
  BtShared *pBt = p->pBt;
  sqlite3BtreeEnter(p);

  rc = sqlite3PagerOpenSavepoint(pBt->pPager, iStatement);
  sqlite3BtreeLeave(p);
  return rc;
}

int sqlite3BtreeSavepoint(Btree *p, int op, int iSavepoint) {
  int rc = SQLITE_OK;
  if (p && p->inTrans == 2) {
    BtShared *pBt = p->pBt;

    sqlite3BtreeEnter(p);
    if (op == 2) {
      rc = saveAllCursors(pBt, 0, 0);
    }
    if (rc == SQLITE_OK) {
      rc = sqlite3PagerSavepoint(pBt->pPager, op, iSavepoint);
    }
    if (rc == SQLITE_OK) {
      if (iSavepoint < 0 && (pBt->btsFlags & 0x0010) != 0) {
        pBt->nPage = 0;
      }
      rc = newDatabase(pBt);
      btreeSetNPage(pBt, pBt->pPage1);
    }
    sqlite3BtreeLeave(p);
  }
  return rc;
}

int btreeCursor(Btree *p, Pgno iTable, int wrFlag, struct KeyInfo *pKeyInfo, BtCursor *pCur) {
  BtShared *pBt = p->pBt;
  BtCursor *pX;

  if (iTable <= 1) {
    if (iTable < 1) {
      return sqlite3CorruptError(77946);
    } else if (btreePagecount(pBt) == 0) {
      iTable = 0;
    }
  }

  pCur->pgnoRoot = iTable;
  pCur->iPage = -1;
  pCur->pKeyInfo = pKeyInfo;
  pCur->pBtree = p;
  pCur->pBt = pBt;
  pCur->curFlags = 0;

  for (pX = pBt->pCursor; pX; pX = pX->pNext) {
    if (pX->pgnoRoot == iTable) {
      pX->curFlags |= 0x20;
      pCur->curFlags = 0x20;
    }
  }
  pCur->eState = 1;
  pCur->pNext = pBt->pCursor;
  pBt->pCursor = pCur;
  if (wrFlag) {
    pCur->curFlags |= 0x01;
    pCur->curPagerFlags = 0;
    if (pBt->pTmpSpace == 0)
      return allocateTempSpace(pBt);
  } else {
    pCur->curPagerFlags = 0x02;
  }
  return SQLITE_OK;
}

int btreeCursorWithLock(Btree *p, Pgno iTable, int wrFlag, struct KeyInfo *pKeyInfo, BtCursor *pCur) {
  int rc;
  sqlite3BtreeEnter(p);
  rc = btreeCursor(p, iTable, wrFlag, pKeyInfo, pCur);
  sqlite3BtreeLeave(p);
  return rc;
}

int sqlite3BtreeCursor(Btree *p, Pgno iTable, int wrFlag, struct KeyInfo *pKeyInfo, BtCursor *pCur) {
  if (p->sharable) {
    return btreeCursorWithLock(p, iTable, wrFlag, pKeyInfo, pCur);
  } else {
    return btreeCursor(p, iTable, wrFlag, pKeyInfo, pCur);
  }
}

int sqlite3BtreeCursorSize(void) {
  return (((sizeof(BtCursor)) + 7) & ~7);
}

int btreeCreateTable(Btree *p, Pgno *piTable, int createTabFlags) {
  BtShared *pBt = p->pBt;
  MemPage *pRoot;
  Pgno pgnoRoot;
  int rc;
  int ptfFlags;

  if (pBt->autoVacuum) {
    Pgno pgnoMove;
    MemPage *pPageMove;

    invalidateAllOverflowCache(pBt);

    sqlite3BtreeGetMeta(p, 4, &pgnoRoot);
    if (pgnoRoot > btreePagecount(pBt)) {
      return sqlite3CorruptError(83313);
    }
    pgnoRoot++;

    while (pgnoRoot == ptrmapPageno(pBt, pgnoRoot) ||
           pgnoRoot == ((Pgno)((sqlite3PendingByte / ((pBt)->pageSize)) + 1))) {
      pgnoRoot++;
    }

    rc = allocateBtreePage(pBt, &pPageMove, &pgnoMove, pgnoRoot, 1);
    if (rc != SQLITE_OK) {
      return rc;
    }

    if (pgnoMove != pgnoRoot) {
      u8 eType = 0;
      Pgno iPtrPage = 0;

      rc = saveAllCursors(pBt, 0, 0);
      releasePage(pPageMove);
      if (rc != SQLITE_OK) {
        return rc;
      }

      rc = btreeGetPage(pBt, pgnoRoot, &pRoot, 0);
      if (rc != SQLITE_OK) {
        return rc;
      }
      rc = ptrmapGet(pBt, pgnoRoot, &eType, &iPtrPage);
      if (eType == 1 || eType == 2) {
        rc = sqlite3CorruptError(83361);
      }
      if (rc != SQLITE_OK) {
        releasePage(pRoot);
        return rc;
      }

      rc = relocatePage(pBt, pRoot, eType, iPtrPage, pgnoMove, 0);
      releasePage(pRoot);

      if (rc != SQLITE_OK) {
        return rc;
      }
      rc = btreeGetPage(pBt, pgnoRoot, &pRoot, 0);
      if (rc != SQLITE_OK) {
        return rc;
      }
      rc = sqlite3PagerWrite(pRoot->pDbPage);
      if (rc != SQLITE_OK) {
        releasePage(pRoot);
        return rc;
      }
    } else {
      pRoot = pPageMove;
    }

    ptrmapPut(pBt, pgnoRoot, 1, 0, &rc);
    if (rc) {
      releasePage(pRoot);
      return rc;
    }

    rc = sqlite3BtreeUpdateMeta(p, 4, pgnoRoot);
    if ((rc)) {
      releasePage(pRoot);
      return rc;
    }

  } else {
    rc = allocateBtreePage(pBt, &pRoot, &pgnoRoot, 1, 0);
    if (rc)
      return rc;
  }

  if (createTabFlags & 1) {
    ptfFlags = 0x01 | 0x04 | 0x08;
  } else {
    ptfFlags = 0x02 | 0x08;
  }
  zeroPage(pRoot, ptfFlags);
  sqlite3PagerUnref(pRoot->pDbPage);

  *piTable = pgnoRoot;
  return SQLITE_OK;
}

int sqlite3BtreeCreateTable(Btree *p, Pgno *piTable, int flags) {
  int rc;
  sqlite3BtreeEnter(p);
  rc = btreeCreateTable(p, piTable, flags);
  sqlite3BtreeLeave(p);
  return rc;
}

int sqlite3BtreeClearTable(Btree *p, int iTable, i64 *pnChange) {
  int rc;
  BtShared *pBt = p->pBt;
  sqlite3BtreeEnter(p);

  rc = saveAllCursors(pBt, (Pgno)iTable, 0);

  if (SQLITE_OK == rc) {
    if (p->hasIncrblobCur) {
      invalidateIncrblobCursors(p, (Pgno)iTable, 0, 1);
    }
    rc = clearDatabasePage(pBt, (Pgno)iTable, 0, pnChange);
  }
  sqlite3BtreeLeave(p);
  return rc;
}

int btreeDropTable(Btree *p, Pgno iTable, int *piMoved) {
  int rc;
  MemPage *pPage = 0;
  BtShared *pBt = p->pBt;

  if (iTable > btreePagecount(pBt)) {
    return sqlite3CorruptError(83562);
  }

  rc = sqlite3BtreeClearTable(p, iTable, 0);
  if (rc)
    return rc;
  rc = btreeGetPage(pBt, (Pgno)iTable, &pPage, 0);
  if ((rc)) {
    releasePage(pPage);
    return rc;
  }

  *piMoved = 0;

  if (pBt->autoVacuum) {
    Pgno maxRootPgno;
    sqlite3BtreeGetMeta(p, 4, &maxRootPgno);

    if (iTable == maxRootPgno) {
      freePage(pPage, &rc);
      releasePage(pPage);
      if (rc != SQLITE_OK) {
        return rc;
      }
    } else {
      MemPage *pMove;
      releasePage(pPage);
      rc = btreeGetPage(pBt, maxRootPgno, &pMove, 0);
      if (rc != SQLITE_OK) {
        return rc;
      }
      rc = relocatePage(pBt, pMove, 1, 0, iTable, 0);
      releasePage(pMove);
      if (rc != SQLITE_OK) {
        return rc;
      }
      pMove = 0;
      rc = btreeGetPage(pBt, maxRootPgno, &pMove, 0);
      freePage(pMove, &rc);
      releasePage(pMove);
      if (rc != SQLITE_OK) {
        return rc;
      }
      *piMoved = maxRootPgno;
    }

    maxRootPgno--;
    while (maxRootPgno == ((Pgno)((sqlite3PendingByte / ((pBt)->pageSize)) + 1)) ||
           (ptrmapPageno((pBt), (maxRootPgno)) == (maxRootPgno))) {
      maxRootPgno--;
    }

    rc = sqlite3BtreeUpdateMeta(p, 4, maxRootPgno);
  } else {
    freePage(pPage, &rc);
    releasePage(pPage);
  }

  return rc;
}

int sqlite3BtreeDropTable(Btree *p, int iTable, int *piMoved) {
  int rc;
  sqlite3BtreeEnter(p);
  rc = btreeDropTable(p, iTable, piMoved);
  sqlite3BtreeLeave(p);
  return rc;
}

void sqlite3BtreeGetMeta(Btree *p, int idx, u32 *pMeta) {
  BtShared *pBt = p->pBt;

  sqlite3BtreeEnter(p);

  if (idx == 15) {
    *pMeta = sqlite3PagerDataVersion(pBt->pPager) + p->iBDataVersion;
  } else {
    *pMeta = sqlite3Get4byte(&pBt->pPage1->aData[36 + idx * 4]);
  }

  sqlite3BtreeLeave(p);
}

int sqlite3BtreeUpdateMeta(Btree *p, int idx, u32 iMeta) {
  BtShared *pBt = p->pBt;
  unsigned char *pP1;
  int rc;

  sqlite3BtreeEnter(p);

  pP1 = pBt->pPage1->aData;
  rc = sqlite3PagerWrite(pBt->pPage1->pDbPage);
  if (rc == SQLITE_OK) {
    sqlite3Put4byte(&pP1[36 + idx * 4], iMeta);

    if (idx == 7) {
      pBt->incrVacuum = (u8)iMeta;
    }
  }
  sqlite3BtreeLeave(p);
  return rc;
}

Pager *sqlite3BtreePager(Btree *p) {
  return p->pBt->pPager;
}

void btreeHeapInsert(u32 *aHeap, u32 x) {
  u32 j, i;

  i = ++aHeap[0];
  aHeap[i] = x;
  while ((j = i / 2) > 0 && aHeap[j] > aHeap[i]) {
    x = aHeap[j];
    aHeap[j] = aHeap[i];
    aHeap[i] = x;
    i = j;
  }
}

int btreeHeapPull(u32 *aHeap, u32 *pOut) {
  u32 j, i, x;
  if ((x = aHeap[0]) == 0)
    return 0;
  *pOut = aHeap[1];
  aHeap[1] = aHeap[x];
  aHeap[x] = 0xffffffff;
  aHeap[0]--;
  i = 1;
  while ((j = i * 2) <= aHeap[0]) {
    if (aHeap[j] > aHeap[j + 1])
      j++;
    if (aHeap[i] < aHeap[j])
      break;
    x = aHeap[i];
    aHeap[i] = aHeap[j];
    aHeap[j] = x;
    i = j;
  }
  return 1;
}

const char *sqlite3BtreeGetFilename(Btree *p) {
  return sqlite3PagerFilename(p->pBt->pPager, 1);
}

const char *sqlite3BtreeGetJournalname(Btree *p) {
  return sqlite3PagerJournalname(p->pBt->pPager);
}

int sqlite3BtreeTxnState(Btree *p) {
  return p ? p->inTrans : 0;
}

int sqlite3BtreeCheckpoint(Btree *p, int eMode, int *pnLog, int *pnCkpt) {
  int rc = SQLITE_OK;
  if (p) {
    BtShared *pBt = p->pBt;
    sqlite3BtreeEnter(p);
    if (pBt->inTransaction != 0) {
      rc = SQLITE_LOCKED;
    } else {
      rc = sqlite3PagerCheckpoint(pBt->pPager, p->db, eMode, pnLog, pnCkpt);
    }
    sqlite3BtreeLeave(p);
  }
  return rc;
}

int sqlite3BtreeIsInBackup(Btree *p) {
  return p->nBackup != 0;
}

void *sqlite3BtreeSchema(Btree *p, int nBytes, void (*xFree)(void *)) {
  BtShared *pBt = p->pBt;

  sqlite3BtreeEnter(p);
  if (!pBt->pSchema && nBytes) {
    pBt->pSchema = sqlite3DbMallocZero(0, nBytes);
    pBt->xFreeSchema = xFree;
  }
  sqlite3BtreeLeave(p);
  return pBt->pSchema;
}

int sqlite3BtreeSchemaLocked(Btree *p) {
  int rc;
  (void)(p);

  sqlite3BtreeEnter(p);
  rc = querySharedCacheTableLock(p, 1, 1);

  sqlite3BtreeLeave(p);
  return rc;
}

int sqlite3BtreeLockTable(Btree *p, int iTab, u8 isWriteLock) {
  int rc = 0;

  if (p->sharable) {
    u8 lockType = 1 + isWriteLock;

    sqlite3BtreeEnter(p);
    rc = querySharedCacheTableLock(p, iTab, lockType);
    if (rc == SQLITE_OK) {
      rc = setSharedCacheTableLock(p, iTab, lockType);
    }
    sqlite3BtreeLeave(p);
  }
  return rc;
}

int sqlite3BtreeSetVersion(Btree *pBtree, int iVersion) {
  BtShared *pBt = pBtree->pBt;
  int rc;

  pBt->btsFlags &= ~0x0020;
  if (iVersion == 1)
    pBt->btsFlags |= 0x0020;

  rc = sqlite3BtreeBeginTrans(pBtree, 0, 0);
  if (rc == SQLITE_OK) {
    u8 *aData = pBt->pPage1->aData;
    if (aData[18] != (u8)iVersion || aData[19] != (u8)iVersion) {
      rc = sqlite3BtreeBeginTrans(pBtree, 2, 0);
      if (rc == SQLITE_OK) {
        rc = sqlite3PagerWrite(pBt->pPage1->pDbPage);
        if (rc == SQLITE_OK) {
          aData[18] = (u8)iVersion;
          aData[19] = (u8)iVersion;
        }
      }
    }
  }

  pBt->btsFlags &= ~0x0020;
  return rc;
}

int sqlite3BtreeIsReadonly(Btree *p) {
  return (p->pBt->btsFlags & 0x0001) != 0;
}

void sqlite3BtreeClearCache(Btree *p) {
  BtShared *pBt = p->pBt;
  if (pBt->inTransaction == 0) {
    sqlite3PagerClearCache(pBt->pPager);
  }
}

int sqlite3BtreeSharable(Btree *p) {
  return p->sharable;
}

int sqlite3BtreeConnectionCount(Btree *p) {
  return p->pBt->nRef;
}

int setDestPgsz(Btree *pDest, Btree *pSrc) {
  return sqlite3BtreeSetPageSize(pDest, sqlite3BtreeGetPageSize(pSrc), 0, 0);
}

int sqlite3BtreeCopyFile(Btree *pTo, Btree *pFrom) {
  int rc;
  sqlite3_file *pFd;
  sqlite3_backup b;
  sqlite3BtreeEnter(pTo);
  sqlite3BtreeEnter(pFrom);

  pFd = sqlite3PagerFile(sqlite3BtreePager(pTo));
  if (pFd->pMethods) {
    i64 nByte = sqlite3BtreeGetPageSize(pFrom) * (i64)sqlite3BtreeLastPage(pFrom);
    rc = sqlite3OsFileControl(pFd, SQLITE_FCNTL_OVERWRITE, &nByte);
    if (rc == SQLITE_NOTFOUND)
      rc = SQLITE_OK;
    if (rc)
      goto copy_finished;
  }

  memset(&b, 0, sizeof(b));
  b.pSrcDb = pFrom->db;
  b.pSrc = pFrom;
  b.pDest = pTo;
  b.iNext = 1;

  sqlite3_backup_step(&b, 0x7FFFFFFF);

  rc = sqlite3_backup_finish(&b);
  if (rc == SQLITE_OK) {
    pTo->pBt->btsFlags &= ~0x0002;
  } else {
    sqlite3PagerClearCache(sqlite3BtreePager(b.pDest));
  }

copy_finished:
  sqlite3BtreeLeave(pFrom);
  sqlite3BtreeLeave(pTo);
  return rc;
}
