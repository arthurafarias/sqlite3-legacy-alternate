#define _GNU_SOURCE 1

#include <string.h>

#include "sqlite/sqlite3_backup.h"

#include "sqlite/BtShared.h"
#include "sqlite/Btree.h"
#include "sqlite/DbPage.h"
#include "sqlite/Pager.h"
#include "sqlite/PgHdr.h"
#include "sqlite/Pgno.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_file.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
/* Private helpers, formerly declared in _Uncategorized.h. */
static int isFatalError(int rc);

static int isFatalError(int rc) { return (rc != 0 && rc != 5 && (rc != 6)); }


int backupOnePage(sqlite3_backup *p, Pgno iSrcPg, const u8 *zSrcData, int bUpdate) {
  Pager *const pDestPager = sqlite3BtreePager(p->pDest);
  const int nSrcPgsz = sqlite3BtreeGetPageSize(p->pSrc);
  int nDestPgsz = sqlite3BtreeGetPageSize(p->pDest);
  const int nCopy = ((nSrcPgsz) < (nDestPgsz) ? (nSrcPgsz) : (nDestPgsz));
  const i64 iEnd = (i64)iSrcPg * (i64)nSrcPgsz;
  int rc = 0;
  i64 iOff;

  for (iOff = iEnd - (i64)nSrcPgsz; rc == 0 && iOff < iEnd; iOff += nDestPgsz) {
    DbPage *pDestPg = 0;
    Pgno iDest = (Pgno)(iOff / nDestPgsz) + 1;
    if (iDest == ((Pgno)((sqlite3PendingByte / ((p->pDest->pBt)->pageSize)) + 1)))
      continue;
    if (0 == (rc = sqlite3PagerGet(pDestPager, iDest, &pDestPg, 0)) && 0 == (rc = sqlite3PagerWrite(pDestPg))) {
      const u8 *zIn = &zSrcData[iOff % nSrcPgsz];
      u8 *zDestData = sqlite3PagerGetData(pDestPg);
      u8 *zOut = &zDestData[iOff % nDestPgsz];

      memcpy(zOut, zIn, nCopy);
      ((u8 *)sqlite3PagerGetExtra(pDestPg))[0] = 0;
      if (iOff == 0 && bUpdate == 0) {
        sqlite3Put4byte(&zOut[28], sqlite3BtreeLastPage(p->pSrc));
      }
    }
    sqlite3PagerUnref(pDestPg);
  }

  return rc;
}

void attachBackupObject(sqlite3_backup *p) {
  sqlite3_backup **pp;

  pp = sqlite3PagerBackupPtr(sqlite3BtreePager(p->pSrc));
  p->pNext = *pp;
  *pp = p;
  p->isAttached = 1;
}

int sqlite3_backup_step(sqlite3_backup *p, int nPage) {
  int rc;
  int destMode = 0;
  int pgszSrc = 0;
  int pgszDest = 0;

  sqlite3_mutex_enter(p->pSrcDb->mutex);
  sqlite3BtreeEnter(p->pSrc);
  if (p->pDestDb) {
    sqlite3_mutex_enter(p->pDestDb->mutex);
  }

  rc = p->rc;
  if (!isFatalError(rc)) {
    Pager *const pSrcPager = sqlite3BtreePager(p->pSrc);
    Btree *pDest = 0;
    Pager *pDestPager = 0;
    int ii;
    int nSrcPage = -1;
    int bCloseTrans = 0;

    if (p->pDestDb && p->pSrc->pBt->inTransaction == 2) {
      rc = 5;
    } else {
      rc = 0;
    }

    if (rc == 0 && 0 == sqlite3BtreeTxnState(p->pSrc)) {
      rc = sqlite3BtreeBeginTrans(p->pSrc, 0, 0);
      bCloseTrans = 1;
    }

    if ((pDest = p->pDest) == 0) {
      pDest = findBtree(p->pDestDb, p->pDestDb, p->zDestDb);
    }
    if (pDest == 0) {
      rc = 1;
    } else {
      pDestPager = sqlite3BtreePager(pDest);
    }

    if (p->bDestLocked == 0 && rc == 0 && setDestPgsz(pDest, p->pSrc) == 7) {
      rc = 7;
    }

    if (0 == rc && p->bDestLocked == 0 && 0 == (rc = sqlite3BtreeBeginTrans(pDest, 2, (int *)&p->iDestSchema))) {
      p->bDestLocked = 1;
      p->pDest = pDest;
    }

    if (rc == 0) {
      pgszSrc = sqlite3BtreeGetPageSize(p->pSrc);
      pgszDest = sqlite3BtreeGetPageSize(p->pDest);
      destMode = sqlite3PagerGetJournalMode(sqlite3BtreePager(p->pDest));
      if ((destMode == 5 || sqlite3PagerIsMemdb(pDestPager)) && pgszSrc != pgszDest) {
        rc = 8;
      }
    }

    nSrcPage = (int)sqlite3BtreeLastPage(p->pSrc);

    ((void)(0))

        ;
    for (ii = 0; (nPage < 0 || ii < nPage) && p->iNext <= (Pgno)nSrcPage && !rc; ii++) {
      const Pgno iSrcPg = p->iNext;
      if (iSrcPg != ((Pgno)((sqlite3PendingByte / ((p->pSrc->pBt)->pageSize)) + 1))) {
        DbPage *pSrcPg;
        rc = sqlite3PagerGet(pSrcPager, iSrcPg, &pSrcPg, 0x02);
        if (rc == 0) {
          rc = backupOnePage(p, iSrcPg, sqlite3PagerGetData(pSrcPg), 0);
          sqlite3PagerUnref(pSrcPg);
        }
      }
      p->iNext++;
    }
    if (rc == 0) {
      p->nPagecount = nSrcPage;
      p->nRemaining = nSrcPage + 1 - p->iNext;
      if (p->iNext > (Pgno)nSrcPage) {
        rc = 101;
      } else if (!p->isAttached) {
        attachBackupObject(p);
      }
    }

    if (rc == 101) {
      if (nSrcPage == 0) {
        rc = sqlite3BtreeNewDb(p->pDest);
        nSrcPage = 1;
      }
      if (rc == 0 || rc == 101) {
        rc = sqlite3BtreeUpdateMeta(p->pDest, 1, p->iDestSchema + 1);
      }
      if (rc == 0) {
        if (p->pDestDb) {
          sqlite3ResetAllSchemasOfConnection(p->pDestDb);
        }
        if (destMode == 5) {
          rc = sqlite3BtreeSetVersion(p->pDest, 2);
        }
      }
      if (rc == 0) {
        int nDestTruncate;

        ((void)(0))

            ;

        ((void)(0))

            ;
        if (pgszSrc < pgszDest) {
          int ratio = pgszDest / pgszSrc;
          nDestTruncate = (nSrcPage + ratio - 1) / ratio;
          if (nDestTruncate == (int)((Pgno)((sqlite3PendingByte / ((p->pDest->pBt)->pageSize)) + 1))) {
            nDestTruncate--;
          }
        } else {
          nDestTruncate = nSrcPage * (pgszSrc / pgszDest);
        }

        ((void)(0))

            ;

        if (pgszSrc < pgszDest) {

          const i64 iSize = (i64)pgszSrc * (i64)nSrcPage;
          sqlite3_file *const pFile = sqlite3PagerFile(pDestPager);
          Pgno iPg;
          int nDstPage;
          i64 iOff;
          i64 iEnd;

          ((void)(0))

              ;

          ((void)(0))

              ;

          sqlite3PagerPagecount(pDestPager, &nDstPage);
          for (iPg = nDestTruncate; rc == 0 && iPg <= (Pgno)nDstPage; iPg++) {
            if (iPg != ((Pgno)((sqlite3PendingByte / ((p->pDest->pBt)->pageSize)) + 1))) {
              DbPage *pPg;
              rc = sqlite3PagerGet(pDestPager, iPg, &pPg, 0);
              if (rc == 0) {
                rc = sqlite3PagerWrite(pPg);
                sqlite3PagerUnref(pPg);
              }
            }
          }
          if (rc == 0) {
            rc = sqlite3PagerCommitPhaseOne(pDestPager, 0, 1);
          }

          iEnd = ((sqlite3PendingByte + pgszDest) < (iSize) ? (sqlite3PendingByte + pgszDest) : (iSize));
          for (iOff = sqlite3PendingByte + pgszSrc; rc == 0 && iOff < iEnd; iOff += pgszSrc) {
            PgHdr *pSrcPg = 0;
            const Pgno iSrcPg = (Pgno)((iOff / pgszSrc) + 1);
            rc = sqlite3PagerGet(pSrcPager, iSrcPg, &pSrcPg, 0);
            if (rc == 0) {
              u8 *zData = sqlite3PagerGetData(pSrcPg);
              rc = sqlite3OsWrite(pFile, zData, pgszSrc, iOff);
            }
            sqlite3PagerUnref(pSrcPg);
          }
          if (rc == 0) {
            rc = backupTruncateFile(pFile, iSize);
          }

          if (rc == 0) {
            rc = sqlite3PagerSync(pDestPager, 0);
          }
        } else {
          sqlite3PagerTruncateImage(pDestPager, nDestTruncate);
          rc = sqlite3PagerCommitPhaseOne(pDestPager, 0, 0);
        }

        if (0 == rc && 0 == (rc = sqlite3BtreeCommitPhaseTwo(p->pDest, 0))) {
          rc = 101;
        }
      }
    }

    if (bCloseTrans) {
      ;
      sqlite3BtreeCommitPhaseOne(p->pSrc, 0);
      sqlite3BtreeCommitPhaseTwo(p->pSrc, 0);

      ((void)(0))

          ;
    }

    if (rc == (10 | (12 << 8))) {
      rc = 7;
    }
    p->rc = rc;
  }
  if (p->pDestDb) {
    sqlite3_mutex_leave(p->pDestDb->mutex);
  }
  sqlite3BtreeLeave(p->pSrc);
  sqlite3_mutex_leave(p->pSrcDb->mutex);
  return rc;
}

int sqlite3_backup_finish(sqlite3_backup *p) {
  sqlite3_backup **pp;
  sqlite3 *pSrcDb;
  int rc;

  if (p == 0)
    return 0;
  pSrcDb = p->pSrcDb;
  sqlite3_mutex_enter(pSrcDb->mutex);
  sqlite3BtreeEnter(p->pSrc);
  if (p->pDestDb) {
    sqlite3_mutex_enter(p->pDestDb->mutex);
  }

  if (p->pDestDb) {
    p->pSrc->nBackup--;
  }
  if (p->isAttached) {
    pp = sqlite3PagerBackupPtr(sqlite3BtreePager(p->pSrc));

    ((void)(0))

        ;
    while (*pp != p) {
      pp = &(*pp)->pNext;

      ((void)(0))

          ;
    }
    *pp = p->pNext;
  }

  if (p->pDest) {
    sqlite3BtreeRollback(p->pDest, 0, 0);
  }

  rc = (p->rc == 101) ? 0 : p->rc;
  if (p->pDestDb) {
    sqlite3Error(p->pDestDb, rc);

    sqlite3LeaveMutexAndCloseZombie(p->pDestDb);
  }
  sqlite3BtreeLeave(p->pSrc);
  if (p->pDestDb) {

    sqlite3_free(p);
  }
  sqlite3LeaveMutexAndCloseZombie(pSrcDb);
  return rc;
}

int sqlite3_backup_remaining(sqlite3_backup *p) { return p->nRemaining; }

int sqlite3_backup_pagecount(sqlite3_backup *p) { return p->nPagecount; }

__attribute__((noinline)) void backupUpdate(sqlite3_backup *p, Pgno iPage, const u8 *aData) {

  do {

    ((void)(0))

        ;
    if (!isFatalError(p->rc) && iPage < p->iNext) {

      int rc;

      ((void)(0))

          ;
      sqlite3_mutex_enter(p->pDestDb->mutex);
      rc = backupOnePage(p, iPage, aData, 1);
      sqlite3_mutex_leave(p->pDestDb->mutex);

      ((void)(0))

          ;
      if (rc != 0) {
        p->rc = rc;
      }
    }
  } while ((p = p->pNext) != 0);
}

void sqlite3BackupUpdate(sqlite3_backup *pBackup, Pgno iPage, const u8 *aData) {
  if (pBackup)
    backupUpdate(pBackup, iPage, aData);
}

void sqlite3BackupRestart(sqlite3_backup *pBackup) {
  sqlite3_backup *p;
  for (p = pBackup; p; p = p->pNext) {

    ((void)(0))

        ;
    p->iNext = 1;
  }
}