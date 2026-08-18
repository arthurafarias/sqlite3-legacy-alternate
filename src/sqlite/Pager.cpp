#define _GNU_SOURCE 1
#include <string.h>
#include "sqlite/Pager.h"
#include "sqlite/Bitvec.h"
#include "sqlite/DbPage.h"
#include "sqlite/PCache.h"
#include "sqlite/PagerSavepoint.h"
#include "sqlite/PgHdr.h"
#include "sqlite/Pgno.h"
#include "sqlite/Sqlite3Config.h"
#include "sqlite/Wal.h"
#include "sqlite/i16.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_backup.h"
#include "sqlite/sqlite3_file.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_io_methods.h"
#include "sqlite/sqlite3_pcache_page.h"
#include "sqlite/sqlite3_vfs.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/SqliteAccessFlags.h"
#include "sqlite/SqliteCheckpointMode.h"
#include "sqlite/SqliteDbStatusParameter.h"
#include "sqlite/SqliteFileControlOpcode.h"
#include "sqlite/SqliteIoCap.h"
#include "sqlite/SqliteOpenFlags.h"
#include "sqlite/SqliteResultCode.h"
#include "sqlite/SqliteSyncFlags.h"
const unsigned char aJournalMagic[] = {
    0xd9, 0xd5, 0x05, 0xf9, 0x20, 0xa1, 0x63, 0xd7,
};

void freeSuperJournal(char *zSuper) {
  if (zSuper) {
    sqlite3_free(&zSuper[-4]);
  }
}

int sqlite3PagerDirectReadOk(Pager *pPager, Pgno pgno) {
  if (pPager->fd->pMethods == 0)
    return 0;
  if (sqlite3PCacheIsDirty(pPager->pPCache))
    return 0;

  if (pPager->pWal) {
    u32 iRead = 0;
    (void)sqlite3WalFindFrame(pPager->pWal, pgno, &iRead);
    if (iRead)
      return 0;
  }

  if ((pPager->fd->pMethods->xDeviceCharacteristics(pPager->fd) & SQLITE_IOCAP_SUBPAGE_READ) == 0) {
    return 0;
  }
  return 1;
}

void setGetterMethod(Pager *pPager) {
  if (pPager->errCode) {
    pPager->xGet = getPageError;

  } else if (((pPager)->bUseFetch)) {
    pPager->xGet = getPageMMap;

  } else {
    pPager->xGet = getPageNormal;
  }
}

int pagerUnlockDb(Pager *pPager, int eLock) {
  int rc = 0;

  if (((pPager->fd)->pMethods != 0)) {
    rc = pPager->noLock ? SQLITE_OK : sqlite3OsUnlock(pPager->fd, eLock);
    if (pPager->eLock != (4 + 1)) {
      pPager->eLock = (u8)eLock;
    }
  }
  pPager->changeCountDone = pPager->tempFile;
  return rc;
}

int pagerLockDb(Pager *pPager, int eLock) {
  int rc = 0;

  if (pPager->eLock < eLock || pPager->eLock == (4 + 1)) {
    rc = pPager->noLock ? SQLITE_OK : sqlite3OsLock(pPager->fd, eLock);
    if (rc == SQLITE_OK && (pPager->eLock != (4 + 1) || eLock == 4)) {
      pPager->eLock = (u8)eLock;
    }
  }
  return rc;
}

int jrnlBufferSize(Pager *pPager) {
  (void)(pPager);

  return 0;
}

i64 journalHdrOffset(Pager *pPager) {
  i64 offset = 0;
  i64 c = pPager->journalOff;
  if (c) {
    offset = ((c - 1) / (pPager->sectorSize) + 1) * (pPager->sectorSize);
  }

  return offset;
}

int zeroJournalHdr(Pager *pPager, int doTruncate) {
  int rc = 0;

  if (pPager->journalOff) {
    const i64 iLimit = pPager->journalSizeLimit;

    if (doTruncate || iLimit == 0) {
      rc = sqlite3OsTruncate(pPager->jfd, 0);
    } else {
      static const char zeroHdr[28] = {0};
      rc = sqlite3OsWrite(pPager->jfd, zeroHdr, sizeof(zeroHdr), 0);
    }
    if (rc == SQLITE_OK && !pPager->noSync) {
      rc = sqlite3OsSync(pPager->jfd, SQLITE_SYNC_DATAONLY | pPager->syncFlags);
    }

    if (rc == SQLITE_OK && iLimit > 0) {
      i64 sz;
      rc = sqlite3OsFileSize(pPager->jfd, &sz);
      if (rc == SQLITE_OK && sz > iLimit) {
        rc = sqlite3OsTruncate(pPager->jfd, iLimit);
      }
    }
  }
  return rc;
}

int writeJournalHdr(Pager *pPager) {
  int rc = SQLITE_OK;
  char *zHeader = pPager->pTmpSpace;
  u32 nHeader = (u32)pPager->pageSize;
  u32 nWrite;
  int ii;

  if (nHeader > (pPager->sectorSize)) {
    nHeader = (pPager->sectorSize);
  }

  for (ii = 0; ii < pPager->nSavepoint; ii++) {
    if (pPager->aSavepoint[ii].iHdrOffset == 0) {
      pPager->aSavepoint[ii].iHdrOffset = pPager->journalOff;
    }
  }

  pPager->journalHdr = pPager->journalOff = journalHdrOffset(pPager);

  if (pPager->noSync || (pPager->journalMode == 4) ||
      (sqlite3OsDeviceCharacteristics(pPager->fd) & SQLITE_IOCAP_SAFE_APPEND)) {
    memcpy(zHeader, aJournalMagic, sizeof(aJournalMagic));
    sqlite3Put4byte((u8 *)&zHeader[sizeof(aJournalMagic)], 0xffffffff);
  } else {
    memset(zHeader, 0, sizeof(aJournalMagic) + 4);
  }

  if (pPager->journalMode != 4) {
    sqlite3_randomness(sizeof(pPager->cksumInit), &pPager->cksumInit);
  }

  sqlite3Put4byte((u8 *)&zHeader[sizeof(aJournalMagic) + 4], pPager->cksumInit);

  sqlite3Put4byte((u8 *)&zHeader[sizeof(aJournalMagic) + 8], pPager->dbOrigSize);

  sqlite3Put4byte((u8 *)&zHeader[sizeof(aJournalMagic) + 12], pPager->sectorSize);

  sqlite3Put4byte((u8 *)&zHeader[sizeof(aJournalMagic) + 16], pPager->pageSize);

  memset(&zHeader[sizeof(aJournalMagic) + 20], 0, nHeader - (sizeof(aJournalMagic) + 20));

  for (nWrite = 0; rc == SQLITE_OK && nWrite < (pPager->sectorSize); nWrite += nHeader) {
    rc = sqlite3OsWrite(pPager->jfd, zHeader, nHeader, pPager->journalOff);

    pPager->journalOff += nHeader;
  }

  return rc;
}

int readJournalHdr(Pager *pPager, int isHot, i64 journalSize, u32 *pNRec, u32 *pDbSize) {
  int rc;
  unsigned char aMagic[8];
  i64 iHdrOff;

  pPager->journalOff = journalHdrOffset(pPager);
  if (pPager->journalOff + (pPager->sectorSize) > journalSize) {
    return SQLITE_DONE;
  }
  iHdrOff = pPager->journalOff;

  if (isHot || iHdrOff != pPager->journalHdr) {
    rc = sqlite3OsRead(pPager->jfd, aMagic, sizeof(aMagic), iHdrOff);
    if (rc) {
      return rc;
    }
    if (memcmp(aMagic, aJournalMagic, sizeof(aMagic)) != 0) {
      return SQLITE_DONE;
    }
  }

  if (SQLITE_OK != (rc = read32bits(pPager->jfd, iHdrOff + 8, pNRec)) ||
      SQLITE_OK != (rc = read32bits(pPager->jfd, iHdrOff + 12, &pPager->cksumInit)) ||
      SQLITE_OK != (rc = read32bits(pPager->jfd, iHdrOff + 16, pDbSize))) {
    return rc;
  }

  if (pPager->journalOff == 0) {
    u32 iPageSize;
    u32 iSectorSize;

    if (SQLITE_OK != (rc = read32bits(pPager->jfd, iHdrOff + 20, &iSectorSize)) ||
        SQLITE_OK != (rc = read32bits(pPager->jfd, iHdrOff + 24, &iPageSize))) {
      return rc;
    }

    if (iPageSize == 0) {
      iPageSize = pPager->pageSize;
    }

    if (iPageSize < 512 || iSectorSize < 32 || iPageSize > 65536 || iSectorSize > 0x10000 ||
        ((iPageSize - 1) & iPageSize) != 0 || ((iSectorSize - 1) & iSectorSize) != 0) {
      return SQLITE_DONE;
    }

    rc = sqlite3PagerSetPagesize(pPager, &iPageSize, -1);

    pPager->sectorSize = iSectorSize;
  }

  pPager->journalOff += (pPager->sectorSize);
  return rc;
}

int writeSuperJournal(Pager *pPager, const char *zSuper) {
  int rc;
  int nSuper;
  i64 iHdrOff;
  i64 jrnlSize;
  u32 cksum = 0;

  if (!zSuper || pPager->journalMode == 4 || !((pPager->jfd)->pMethods != 0)) {
    return SQLITE_OK;
  }
  pPager->setSuper = 1;

  for (nSuper = 0; zSuper[nSuper]; nSuper++) {
    cksum += zSuper[nSuper];
  }

  if (pPager->fullSync) {
    pPager->journalOff = journalHdrOffset(pPager);
  }
  iHdrOff = pPager->journalOff;

  if ((0 != (rc = write32bits(pPager->jfd, iHdrOff, ((pPager)->lckPgno)))) ||
      (0 != (rc = sqlite3OsWrite(pPager->jfd, zSuper, nSuper, iHdrOff + 4))) ||
      (0 != (rc = write32bits(pPager->jfd, iHdrOff + 4 + nSuper, nSuper))) ||
      (0 != (rc = write32bits(pPager->jfd, iHdrOff + 4 + nSuper + 4, cksum))) ||
      (0 != (rc = sqlite3OsWrite(pPager->jfd, aJournalMagic, 8, iHdrOff + 4 + nSuper + 8)))) {
    return rc;
  }
  pPager->journalOff += (nSuper + 20);

  if (SQLITE_OK == (rc = sqlite3OsFileSize(pPager->jfd, &jrnlSize)) && jrnlSize > pPager->journalOff) {
    rc = sqlite3OsTruncate(pPager->jfd, pPager->journalOff);
  }
  return rc;
}

void pager_reset(Pager *pPager) {
  pPager->iDataVersion++;
  sqlite3BackupRestart(pPager->pBackup);
  sqlite3PcacheClear(pPager->pPCache);
}

u32 sqlite3PagerDataVersion(Pager *pPager) {
  return pPager->iDataVersion;
}

void releaseAllSavepoints(Pager *pPager) {
  int ii;
  for (ii = 0; ii < pPager->nSavepoint; ii++) {
    sqlite3BitvecDestroy(pPager->aSavepoint[ii].pInSavepoint);
  }
  if (!pPager->exclusiveMode || sqlite3JournalIsInMemory(pPager->sjfd)) {
    sqlite3OsClose(pPager->sjfd);
  }
  sqlite3_free(pPager->aSavepoint);
  pPager->aSavepoint = 0;
  pPager->nSavepoint = 0;
  pPager->nSubRec = 0;
}

int addToSavepointBitvecs(Pager *pPager, Pgno pgno) {
  int ii;
  int rc = SQLITE_OK;

  for (ii = 0; ii < pPager->nSavepoint; ii++) {
    PagerSavepoint *p = &pPager->aSavepoint[ii];
    if (pgno <= p->nOrig) {
      rc |= sqlite3BitvecSet(p->pInSavepoint, pgno);
    }
  }
  return rc;
}

void pager_unlock(Pager *pPager) {
  sqlite3BitvecDestroy(pPager->pInJournal);
  pPager->pInJournal = 0;
  releaseAllSavepoints(pPager);

  if (((pPager)->pWal != 0)) {
    if (pPager->eState == 6) {
      (void)sqlite3WalEndWriteTransaction(pPager->pWal);
    }
    sqlite3WalEndReadTransaction(pPager->pWal);
    pPager->eState = 0;
  } else if (!pPager->exclusiveMode) {
    int rc;
    int iDc = ((pPager->fd)->pMethods != 0) ? sqlite3OsDeviceCharacteristics(pPager->fd) : 0;

    if (0 == (iDc & SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN) || 1 != (pPager->journalMode & 5)) {
      sqlite3OsClose(pPager->jfd);
    }

    rc = pagerUnlockDb(pPager, 0);
    if (rc != SQLITE_OK && pPager->eState == 6) {
      pPager->eLock = (4 + 1);
    }

    pPager->eState = 0;
  }

  if (pPager->errCode) {
    if (pPager->tempFile == 0) {
      pager_reset(pPager);
      pPager->changeCountDone = 0;
      pPager->eState = 0;
    } else {
      pPager->eState = (((pPager->jfd)->pMethods != 0) ? 0 : 1);
    }
    if (((pPager)->bUseFetch))
      sqlite3OsUnfetch(pPager->fd, 0, 0);
    pPager->errCode = SQLITE_OK;
    setGetterMethod(pPager);
  }

  pPager->journalOff = 0;
  pPager->journalHdr = 0;
  pPager->setSuper = 0;
}

int pager_error(Pager *pPager, int rc) {
  int rc2 = rc & 0xff;

  if (rc2 == SQLITE_FULL || rc2 == SQLITE_IOERR) {
    pPager->errCode = rc;
    pPager->eState = 6;
    setGetterMethod(pPager);
  }
  return rc;
}

int pagerFlushOnCommit(Pager *pPager, int bCommit) {
  if (pPager->tempFile == 0)
    return 1;
  if (!bCommit)
    return 0;
  if (!((pPager->fd)->pMethods != 0))
    return 0;
  return (sqlite3PCachePercentDirty(pPager->pPCache) >= 25);
}

int pager_end_transaction(Pager *pPager, int hasSuper, int bCommit) {
  int rc = SQLITE_OK;
  int rc2 = 0;

  if (pPager->eState < 2 && pPager->eLock < 2) {
    return SQLITE_OK;
  }

  releaseAllSavepoints(pPager);

  if (((pPager->jfd)->pMethods != 0)) {
    if (sqlite3JournalIsInMemory(pPager->jfd)) {
      sqlite3OsClose(pPager->jfd);
    } else if (pPager->journalMode == 3) {
      if (pPager->journalOff == 0) {
        rc = SQLITE_OK;
      } else {
        rc = sqlite3OsTruncate(pPager->jfd, 0);
        if (rc == SQLITE_OK && pPager->fullSync) {
          rc = sqlite3OsSync(pPager->jfd, pPager->syncFlags);
        }
      }
      pPager->journalOff = 0;
    } else if (pPager->journalMode == 1 || (pPager->exclusiveMode && pPager->journalMode < 5)) {
      rc = zeroJournalHdr(pPager, hasSuper || pPager->tempFile);
      pPager->journalOff = 0;
    } else {
      int bDelete = !pPager->tempFile;

      sqlite3OsClose(pPager->jfd);
      if (bDelete) {
        rc = sqlite3OsDelete(pPager->pVfs, pPager->zJournal, pPager->extraSync);
      }
    }
  }

  sqlite3BitvecDestroy(pPager->pInJournal);
  pPager->pInJournal = 0;
  pPager->nRec = 0;
  if (rc == SQLITE_OK) {
    if (pPager->memDb || pagerFlushOnCommit(pPager, bCommit)) {
      sqlite3PcacheCleanAll(pPager->pPCache);
    } else {
      sqlite3PcacheClearWritable(pPager->pPCache);
    }
    sqlite3PcacheTruncate(pPager->pPCache, pPager->dbSize);
  }

  if (((pPager)->pWal != 0)) {
    rc2 = sqlite3WalEndWriteTransaction(pPager->pWal);

  } else if (rc == SQLITE_OK && bCommit && pPager->dbFileSize > pPager->dbSize) {
    rc = pager_truncate(pPager, pPager->dbSize);
  }

  if (rc == SQLITE_OK && bCommit) {
    rc = sqlite3OsFileControl(pPager->fd, SQLITE_FCNTL_COMMIT_PHASETWO, 0);
    if (rc == SQLITE_NOTFOUND)
      rc = SQLITE_OK;
  }

  if (!pPager->exclusiveMode && (!((pPager)->pWal != 0) || sqlite3WalExclusiveMode(pPager->pWal, 0))) {
    rc2 = pagerUnlockDb(pPager, 1);
  }
  pPager->eState = 1;
  pPager->setSuper = 0;

  return (rc == SQLITE_OK ? rc2 : rc);
}

void pagerUnlockAndRollback(Pager *pPager) {
  if (pPager->eState != 6 && pPager->eState != 0) {
    if (pPager->eState >= 2) {
      sqlite3BeginBenignMalloc();
      sqlite3PagerRollback(pPager);
      sqlite3EndBenignMalloc();
    } else if (!pPager->exclusiveMode) {
      pager_end_transaction(pPager, 0, 0);
    }
  } else if (pPager->eState == 6 && pPager->journalMode == 4 && ((pPager->jfd)->pMethods != 0)) {
    int errCode = pPager->errCode;
    u8 eLock = pPager->eLock;
    pPager->eState = 0;
    pPager->errCode = SQLITE_OK;
    pPager->eLock = 4;
    pager_playback(pPager, 1);
    pPager->errCode = errCode;
    pPager->eLock = eLock;
  }
  pager_unlock(pPager);
}

u32 pager_cksum(Pager *pPager, const u8 *aData) {
  u32 cksum = pPager->cksumInit;
  int i = pPager->pageSize - 200;
  while (i > 0) {
    cksum += aData[i];
    i -= 200;
  }
  return cksum;
}

int pager_playback_one_page(Pager *pPager, i64 *pOffset, Bitvec *pDone, int isMainJrnl, int isSavepnt) {
  int rc;
  PgHdr *pPg;
  Pgno pgno;
  u32 cksum;
  char *aData;
  sqlite3_file *jfd;
  int isSynced;

  aData = pPager->pTmpSpace;

  jfd = isMainJrnl ? pPager->jfd : pPager->sjfd;
  rc = read32bits(jfd, *pOffset, &pgno);
  if (rc != SQLITE_OK)
    return rc;
  rc = sqlite3OsRead(jfd, (u8 *)aData, pPager->pageSize, (*pOffset) + 4);
  if (rc != SQLITE_OK)
    return rc;
  *pOffset += pPager->pageSize + 4 + isMainJrnl * 4;

  if (pgno == 0 || pgno == ((pPager)->lckPgno)) {
    return SQLITE_DONE;
  }
  if (pgno > (Pgno)pPager->dbSize || sqlite3BitvecTest(pDone, pgno)) {
    return SQLITE_OK;
  }
  if (isMainJrnl) {
    rc = read32bits(jfd, (*pOffset) - 4, &cksum);
    if (rc)
      return rc;
    if (!isSavepnt && pager_cksum(pPager, (u8 *)aData) != cksum) {
      return SQLITE_DONE;
    }
  }

  if (pDone && (rc = sqlite3BitvecSet(pDone, pgno)) != SQLITE_OK) {
    return rc;
  }

  if (pgno == 1 && pPager->nReserve != ((u8 *)aData)[20]) {
    pPager->nReserve = ((u8 *)aData)[20];
  }

  if (((pPager)->pWal != 0)) {
    pPg = 0;
  } else {
    pPg = sqlite3PagerLookup(pPager, pgno);
  }

  if (isMainJrnl) {
    isSynced = pPager->noSync || (*pOffset <= pPager->journalHdr);
  } else {
    isSynced = (pPg == 0 || 0 == (pPg->flags & 0x008));
  }
  if (((pPager->fd)->pMethods != 0) && (pPager->eState >= 4 || pPager->eState == 0) && isSynced) {
    i64 ofst = (pgno - 1) * (i64)pPager->pageSize;

    rc = sqlite3OsWrite(pPager->fd, (u8 *)aData, pPager->pageSize, ofst);

    if (pgno > pPager->dbFileSize) {
      pPager->dbFileSize = pgno;
    }
    if (pPager->pBackup) {
      sqlite3BackupUpdate(pPager->pBackup, pgno, (u8 *)aData);
    }
  } else if (!isMainJrnl && pPg == 0) {
    pPager->doNotSpill |= 0x02;
    rc = sqlite3PagerGet(pPager, pgno, &pPg, 1);

    pPager->doNotSpill &= ~0x02;
    if (rc != SQLITE_OK)
      return rc;
    sqlite3PcacheMakeDirty(pPg);
  }
  if (pPg) {
    void *pData;
    pData = pPg->pData;
    memcpy(pData, (u8 *)aData, pPager->pageSize);
    pPager->xReiniter(pPg);

    if (pgno == 1) {
      memcpy(&pPager->dbFileVers, &((u8 *)pData)[24], sizeof(pPager->dbFileVers));
    }
    sqlite3PcacheRelease(pPg);
  }
  return rc;
}

static int pagerIsSuperJrnlName(const char *zSuper) {
  const int nSuper = sqlite3Strlen30(zSuper);
  int ii;

  if (nSuper < 4)
    return 0;
  if (zSuper[nSuper - 3] != '9')
    return 0;

  if (nSuper < 12)
    return 0;
  if (memcmp(&zSuper[nSuper - 12], "-mj", 3))
    return 0;
  for (ii = nSuper - 9; ii < nSuper; ii++) {
    if ((sqlite3CtypeMap[(unsigned char)(zSuper[ii])] & 0x08) == 0)
      return 0;
  }
  return 1;
}

int pager_delsuper(Pager *pPager, const char *zSuper) {
  sqlite3_vfs *pVfs = pPager->pVfs;
  int rc;
  sqlite3_file *pSuper;
  sqlite3_file *pJournal;
  char *zSuperJournal = 0;
  i64 nSuperJournal;
  char *zJournal;
  char *zFree = 0;
  int bSeen = 0;

  if (pagerIsSuperJrnlName(zSuper) == 0) {
    return SQLITE_OK;
  }

  pSuper = (sqlite3_file *)sqlite3MallocZero(2 * (i64)pVfs->szOsFile);
  if (!pSuper) {
    rc = 7;
    pJournal = 0;
  } else {
    const int flags = (SQLITE_OPEN_READONLY | SQLITE_OPEN_SUPER_JOURNAL);
    rc = sqlite3OsOpen(pVfs, zSuper, pSuper, flags, 0);
    pJournal = (sqlite3_file *)(((u8 *)pSuper) + pVfs->szOsFile);
  }
  if (rc != SQLITE_OK)
    goto delsuper_out;

  rc = sqlite3OsFileSize(pSuper, &nSuperJournal);
  if (rc != SQLITE_OK)
    goto delsuper_out;

  zFree = (char*)(sqlite3Malloc(4 + nSuperJournal + 2));
  if (!zFree) {
    rc = 7;
    goto delsuper_out;
  } else {
  }
  zFree[0] = zFree[1] = zFree[2] = zFree[3] = 0;
  zSuperJournal = &zFree[4];
  rc = sqlite3OsRead(pSuper, zSuperJournal, (int)nSuperJournal, 0);
  if (rc != SQLITE_OK)
    goto delsuper_out;
  zSuperJournal[nSuperJournal] = 0;
  zSuperJournal[nSuperJournal + 1] = 0;

  zJournal = zSuperJournal;
  while ((zJournal - zSuperJournal) < nSuperJournal) {
    if (strcmp(zJournal, pPager->zJournal) == 0) {
      bSeen = 1;
    } else {
      int exists;
      rc = sqlite3OsAccess(pVfs, zJournal, SQLITE_ACCESS_EXISTS, &exists);
      if (rc != SQLITE_OK) {
        goto delsuper_out;
      }
      if (exists) {
        char *zSuperPtr = 0;

        int c;
        int flags = (SQLITE_OPEN_READONLY | SQLITE_OPEN_SUPER_JOURNAL);
        rc = sqlite3OsOpen(pVfs, zJournal, pJournal, flags, 0);
        if (rc != SQLITE_OK) {
          goto delsuper_out;
        }

        rc = readSuperJournal(pJournal, 1 + (u64)pVfs->mxPathname, &zSuperPtr);
        sqlite3OsClose(pJournal);
        if (rc != SQLITE_OK) {
          goto delsuper_out;
        }

        c = zSuperPtr != 0 && strcmp(zSuperPtr, zSuper) == 0;
        freeSuperJournal(zSuperPtr);
        if (c) {
          goto delsuper_out;
        }
      }
    }
    zJournal += (sqlite3Strlen30(zJournal) + 1);
  }

  sqlite3OsClose(pSuper);
  if (bSeen) {
    rc = sqlite3OsDelete(pVfs, zSuper, 0);
  }

delsuper_out:
  sqlite3_free(zFree);
  if (pSuper) {
    sqlite3OsClose(pSuper);

    sqlite3_free(pSuper);
  }
  return rc;
}

int pager_truncate(Pager *pPager, Pgno nPage) {
  int rc = 0;

  if (((pPager->fd)->pMethods != 0) && (pPager->eState >= 4 || pPager->eState == 0)) {
    i64 currentSize, newSize;
    int szPage = pPager->pageSize;

    rc = sqlite3OsFileSize(pPager->fd, &currentSize);
    newSize = szPage * (i64)nPage;
    if (rc == SQLITE_OK && currentSize != newSize) {
      if (currentSize > newSize) {
        rc = sqlite3OsTruncate(pPager->fd, newSize);
      } else if ((currentSize + szPage) <= newSize) {
        char *pTmp = pPager->pTmpSpace;
        memset(pTmp, 0, szPage);
        sqlite3OsFileControlHint(pPager->fd, SQLITE_FCNTL_SIZE_HINT, &newSize);
        rc = sqlite3OsWrite(pPager->fd, pTmp, szPage, newSize - szPage);
      }
      if (rc == SQLITE_OK) {
        pPager->dbFileSize = nPage;
      }
    }
  }
  return rc;
}

void setSectorSize(Pager *pPager) {
  if (pPager->tempFile || (sqlite3OsDeviceCharacteristics(pPager->fd) & SQLITE_IOCAP_POWERSAFE_OVERWRITE) != 0) {
    pPager->sectorSize = 512;
  } else {
    pPager->sectorSize = sqlite3SectorSize(pPager->fd);
  }
}

int pager_playback(Pager *pPager, int isHot) {
  sqlite3_vfs *pVfs = pPager->pVfs;
  i64 szJ;
  u32 nRec;
  u32 u;
  Pgno mxPg = 0;
  int rc;
  int res = 1;
  char *zSuper = 0;
  int needPagerReset;
  int nPlayback = 0;
  u32 savedPageSize = pPager->pageSize;

  rc = sqlite3OsFileSize(pPager->jfd, &szJ);
  if (rc != SQLITE_OK) {
    goto end_playback;
  }

  rc = readSuperJournal(pPager->jfd, 1 + (i64)pPager->pVfs->mxPathname, &zSuper);
  if (rc == SQLITE_OK && zSuper) {
    rc = sqlite3OsAccess(pVfs, zSuper, SQLITE_ACCESS_EXISTS, &res);
  }
  if (rc != SQLITE_OK || !res) {
    goto end_playback;
  }
  pPager->journalOff = 0;
  needPagerReset = isHot;

  while (1) {
    rc = readJournalHdr(pPager, isHot, szJ, &nRec, &mxPg);
    if (rc != SQLITE_OK) {
      if (rc == SQLITE_DONE) {
        rc = SQLITE_OK;
      }
      goto end_playback;
    }

    if (nRec == 0xffffffff) {
      nRec = (int)((szJ - (pPager->sectorSize)) / ((pPager->pageSize) + 8));
    }

    if (nRec == 0 && !isHot && pPager->journalHdr + (pPager->sectorSize) == pPager->journalOff) {
      nRec = (int)((szJ - pPager->journalOff) / ((pPager->pageSize) + 8));
    }

    if (pPager->journalOff == (pPager->sectorSize)) {
      rc = pager_truncate(pPager, mxPg);
      if (rc != SQLITE_OK) {
        goto end_playback;
      }
      pPager->dbSize = mxPg;
      if (pPager->mxPgno < mxPg) {
        pPager->mxPgno = mxPg;
      }
    }

    for (u = 0; u < nRec; u++) {
      if (needPagerReset) {
        pager_reset(pPager);
        needPagerReset = 0;
      }
      rc = pager_playback_one_page(pPager, &pPager->journalOff, 0, 1, 0);
      if (rc == SQLITE_OK) {
        nPlayback++;
      } else {
        if (rc == SQLITE_DONE) {
          pPager->journalOff = szJ;
          break;
        } else if (rc == (10 | (2 << 8))) {
          rc = SQLITE_OK;
          goto end_playback;
        } else {
          goto end_playback;
        }
      }
    }
  }

end_playback:
  if (rc == SQLITE_OK) {
    rc = sqlite3PagerSetPagesize(pPager, &savedPageSize, -1);
  }

  pPager->changeCountDone = pPager->tempFile;

  if (rc == SQLITE_OK && (pPager->eState >= 4 || pPager->eState == 0)) {
    rc = sqlite3PagerSync(pPager, 0);
  }
  if (rc == SQLITE_OK) {
    rc = pager_end_transaction(pPager, zSuper != 0, 0);
  }
  if (rc == SQLITE_OK && zSuper && res) {
    rc = pager_delsuper(pPager, zSuper);
  }
  if (isHot && nPlayback) {
    sqlite3_log((27 | (2 << 8)), "recovered %d pages from %s", nPlayback, pPager->zJournal);
  }

  freeSuperJournal(zSuper);
  setSectorSize(pPager);
  return rc;
}

static int pagerUndoCallback(void *pCtx, Pgno iPg) {
  int rc = SQLITE_OK;
  Pager *pPager = (Pager *)pCtx;
  PgHdr *pPg;

  pPg = sqlite3PagerLookup(pPager, iPg);
  if (pPg) {
    if (sqlite3PcachePageRefcount(pPg) == 1) {
      sqlite3PcacheDrop(pPg);
    } else {
      rc = readDbPage(pPg);
      if (rc == SQLITE_OK) {
        pPager->xReiniter(pPg);
      }
      sqlite3PagerUnrefNotNull(pPg);
    }
  }

  sqlite3BackupRestart(pPager->pBackup);

  return rc;
}

int pagerRollbackWal(Pager *pPager) {
  int rc;
  PgHdr *pList;

  pPager->dbSize = pPager->dbOrigSize;
  rc = sqlite3WalUndo(pPager->pWal, pagerUndoCallback, (void *)pPager);
  pList = sqlite3PcacheDirtyList(pPager->pPCache);
  while (pList && rc == SQLITE_OK) {
    PgHdr *pNext = pList->pDirty;
    rc = pagerUndoCallback((void *)pPager, pList->pgno);
    pList = pNext;
  }

  return rc;
}

int pagerWalFrames(Pager *pPager, PgHdr *pList, Pgno nTruncate, int isCommit) {
  int rc;
  int nList;
  PgHdr *p;

  if (isCommit) {
    PgHdr **ppNext = &pList;
    nList = 0;
    for (p = pList; (*ppNext = p) != 0; p = p->pDirty) {
      if (p->pgno <= nTruncate) {
        ppNext = &p->pDirty;
        nList++;
      }
    }

  } else {
    nList = 1;
  }
  pPager->aStat[2] += nList;

  if (pList->pgno == 1)
    pager_write_changecounter(pList);
  rc = sqlite3WalFrames(pPager->pWal, pPager->pageSize, pList, nTruncate, isCommit, pPager->walSyncFlags);
  if (rc == SQLITE_OK && pPager->pBackup) {
    for (p = pList; p; p = p->pDirty) {
      sqlite3BackupUpdate(pPager->pBackup, p->pgno, (u8 *)p->pData);
    }
  }

  return rc;
}

int pagerBeginReadTransaction(Pager *pPager) {
  int rc;
  int changed = 0;

  sqlite3WalEndReadTransaction(pPager->pWal);

  rc = sqlite3WalBeginReadTransaction(pPager->pWal, &changed);
  if (rc != SQLITE_OK || changed) {
    pager_reset(pPager);
    if (((pPager)->bUseFetch))
      sqlite3OsUnfetch(pPager->fd, 0, 0);
  }

  return rc;
}

int pagerPagecount(Pager *pPager, Pgno *pnPage) {
  Pgno nPage;

  nPage = sqlite3WalDbsize(pPager->pWal);

  if (nPage == 0 && (((pPager->fd)->pMethods != 0))) {
    i64 n = 0;
    int rc = sqlite3OsFileSize(pPager->fd, &n);
    if (rc != SQLITE_OK) {
      return rc;
    }
    nPage = (Pgno)((n + pPager->pageSize - 1) / pPager->pageSize);
  }

  if (nPage > pPager->mxPgno) {
    pPager->mxPgno = (Pgno)nPage;
  }

  *pnPage = nPage;
  return SQLITE_OK;
}

int pagerOpenWalIfPresent(Pager *pPager) {
  int rc = 0;

  if (!pPager->tempFile) {
    int isWal;
    rc = sqlite3OsAccess(pPager->pVfs, pPager->zWal, SQLITE_ACCESS_EXISTS, &isWal);
    if (rc == SQLITE_OK) {
      if (isWal) {
        Pgno nPage;

        rc = pagerPagecount(pPager, &nPage);
        if (rc)
          return rc;
        if (nPage == 0) {
          rc = sqlite3OsDelete(pPager->pVfs, pPager->zWal, 0);
        } else {
          rc = sqlite3PagerOpenWal(pPager, 0);
        }
      } else if (pPager->journalMode == 5) {
        pPager->journalMode = 0;
      }
    }
  }
  return rc;
}

int pagerPlaybackSavepoint(Pager *pPager, PagerSavepoint *pSavepoint) {
  i64 szJ;
  i64 iHdrOff;
  int rc = SQLITE_OK;
  Bitvec *pDone = 0;

  if (pSavepoint) {
    pDone = sqlite3BitvecCreate(pSavepoint->nOrig);
    if (!pDone) {
      return 7;
    }
  }

  pPager->dbSize = pSavepoint ? pSavepoint->nOrig : pPager->dbOrigSize;
  pPager->changeCountDone = pPager->tempFile;

  if (!pSavepoint && ((pPager)->pWal != 0)) {
    return pagerRollbackWal(pPager);
  }

  szJ = pPager->journalOff;

  if (pSavepoint && !((pPager)->pWal != 0)) {
    iHdrOff = pSavepoint->iHdrOffset ? pSavepoint->iHdrOffset : szJ;
    pPager->journalOff = pSavepoint->iOffset;
    while (rc == SQLITE_OK && pPager->journalOff < iHdrOff) {
      rc = pager_playback_one_page(pPager, &pPager->journalOff, pDone, 1, 1);
    }

  } else {
    pPager->journalOff = 0;
  }

  while (rc == SQLITE_OK && pPager->journalOff < szJ) {
    u32 ii;
    u32 nJRec = 0;
    u32 dummy;
    rc = readJournalHdr(pPager, 0, szJ, &nJRec, &dummy);

    if (nJRec == 0 && pPager->journalHdr + (pPager->sectorSize) == pPager->journalOff) {
      nJRec = (u32)((szJ - pPager->journalOff) / ((pPager->pageSize) + 8));
    }
    for (ii = 0; rc == SQLITE_OK && ii < nJRec && pPager->journalOff < szJ; ii++) {
      rc = pager_playback_one_page(pPager, &pPager->journalOff, pDone, 1, 1);
    }
  }

  if (pSavepoint) {
    u32 ii;
    i64 offset = (i64)pSavepoint->iSubRec * (4 + pPager->pageSize);

    if (((pPager)->pWal != 0)) {
      rc = sqlite3WalSavepointUndo(pPager->pWal, pSavepoint->aWalData);
    }
    for (ii = pSavepoint->iSubRec; rc == SQLITE_OK && ii < pPager->nSubRec; ii++) {
      rc = pager_playback_one_page(pPager, &offset, pDone, 0, 1);
    }
  }

  sqlite3BitvecDestroy(pDone);
  if (rc == SQLITE_OK) {
    pPager->journalOff = szJ;
  }

  return rc;
}

void sqlite3PagerSetCachesize(Pager *pPager, int mxPage) {
  sqlite3PcacheSetCachesize(pPager->pPCache, mxPage);
}

int sqlite3PagerSetSpillsize(Pager *pPager, int mxPage) {
  return sqlite3PcacheSetSpillsize(pPager->pPCache, mxPage);
}

void pagerFixMaplimit(Pager *pPager) {
  sqlite3_file *fd = pPager->fd;
  if (((fd)->pMethods != 0) && fd->pMethods->iVersion >= 3) {
    sqlite3_int64 sz;
    sz = pPager->szMmap;
    pPager->bUseFetch = (sz > 0);
    setGetterMethod(pPager);
    sqlite3OsFileControlHint(pPager->fd, SQLITE_FCNTL_MMAP_SIZE, &sz);
  }
}

void sqlite3PagerSetMmapLimit(Pager *pPager, sqlite3_int64 szMmap) {
  pPager->szMmap = szMmap;
  pagerFixMaplimit(pPager);
}

void sqlite3PagerShrink(Pager *pPager) {
  sqlite3PcacheShrink(pPager->pPCache);
}

void sqlite3PagerSetFlags(Pager *pPager, unsigned pgFlags) {
  unsigned level = pgFlags & 0x07;
  if (pPager->tempFile || level == 0x01) {
    pPager->noSync = 1;
    pPager->fullSync = 0;
    pPager->extraSync = 0;
  } else {
    pPager->noSync = 0;
    pPager->fullSync = level >= 0x03 ? 1 : 0;

    if (level == 0x04) {
      pPager->extraSync = 1;
    } else {
      pPager->extraSync = 0;
    }
  }
  if (pPager->noSync) {
    pPager->syncFlags = 0;
  } else if (pgFlags & 0x08) {
    pPager->syncFlags = SQLITE_SYNC_FULL;
  } else {
    pPager->syncFlags = SQLITE_SYNC_NORMAL;
  }
  pPager->walSyncFlags = (pPager->syncFlags << 2);
  if (pPager->fullSync) {
    pPager->walSyncFlags |= pPager->syncFlags;
  }
  if ((pgFlags & 0x10) && !pPager->noSync) {
    pPager->walSyncFlags |= (SQLITE_SYNC_FULL << 2);
  }
  if (pgFlags & 0x20) {
    pPager->doNotSpill &= ~0x01;
  } else {
    pPager->doNotSpill |= 0x01;
  }
}

int pagerOpentemp(Pager *pPager, sqlite3_file *pFile, int vfsFlags) {
  int rc;

  vfsFlags |= SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXCLUSIVE | SQLITE_OPEN_DELETEONCLOSE;
  rc = sqlite3OsOpen(pPager->pVfs, 0, pFile, vfsFlags, 0);

  return rc;
}

void sqlite3PagerSetBusyHandler(Pager *pPager, int (*xBusyHandler)(void *), void *pBusyHandlerArg) {
  void **ap;
  pPager->xBusyHandler = xBusyHandler;
  pPager->pBusyHandlerArg = pBusyHandlerArg;
  ap = (void **)&pPager->xBusyHandler;

  sqlite3OsFileControlHint(pPager->fd, SQLITE_FCNTL_BUSYHANDLER, (void *)ap);
}

int sqlite3PagerSetPagesize(Pager *pPager, u32 *pPageSize, int nReserve) {
  int rc = SQLITE_OK;

  u32 pageSize = *pPageSize;

  if ((pPager->memDb == 0 || pPager->dbSize == 0) && sqlite3PcacheRefCount(pPager->pPCache) == 0 && pageSize &&
      pageSize != (u32)pPager->pageSize) {
    char *pNew = nullptr;
    i64 nByte = 0;

    if (pPager->eState > 0 && ((pPager->fd)->pMethods != 0)) {
      rc = sqlite3OsFileSize(pPager->fd, &nByte);
    }
    if (rc == SQLITE_OK) {
      pNew = (char *)sqlite3PageMalloc(pageSize + 8);
      if (!pNew) {
        rc = 7;
      } else {
        memset(pNew + pageSize, 0, 8);
      }
    }

    if (rc == SQLITE_OK) {
      pager_reset(pPager);
      rc = sqlite3PcacheSetPageSize(pPager->pPCache, pageSize);
    }
    if (rc == SQLITE_OK) {
      sqlite3PageFree(pPager->pTmpSpace);
      pPager->pTmpSpace = pNew;
      pPager->dbSize = (Pgno)((nByte + pageSize - 1) / pageSize);
      pPager->pageSize = pageSize;
      pPager->lckPgno = (Pgno)(sqlite3PendingByte / pageSize) + 1;
    } else {
      sqlite3PageFree(pNew);
    }
  }

  *pPageSize = pPager->pageSize;
  if (rc == SQLITE_OK) {
    if (nReserve < 0)
      nReserve = pPager->nReserve;

    pPager->nReserve = (i16)nReserve;
    pagerFixMaplimit(pPager);
  }
  return rc;
}

void *sqlite3PagerTempSpace(Pager *pPager) {
  return pPager->pTmpSpace;
}

Pgno sqlite3PagerMaxPageCount(Pager *pPager, Pgno mxPage) {
  if (mxPage > 0) {
    pPager->mxPgno = mxPage;
  }

  return pPager->mxPgno;
}

int sqlite3PagerReadFileheader(Pager *pPager, int N, unsigned char *pDest) {
  int rc = SQLITE_OK;
  memset(pDest, 0, N);

  if (((pPager->fd)->pMethods != 0)) {
    rc = sqlite3OsRead(pPager->fd, pDest, N, 0);
    if (rc == (10 | (2 << 8))) {
      rc = SQLITE_OK;
    }
  }
  return rc;
}

void sqlite3PagerPagecount(Pager *pPager, int *pnPage) {
  *pnPage = (int)pPager->dbSize;
}

int pager_wait_on_lock(Pager *pPager, int locktype) {
  int rc;

  do {
    rc = pagerLockDb(pPager, locktype);
  } while (rc == SQLITE_BUSY && pPager->xBusyHandler(pPager->pBusyHandlerArg));
  return rc;
}

void sqlite3PagerTruncateImage(Pager *pPager, Pgno nPage) {
  pPager->dbSize = nPage;
}

int pagerSyncHotJournal(Pager *pPager) {
  int rc = SQLITE_OK;
  if (!pPager->noSync) {
    rc = sqlite3OsSync(pPager->jfd, SQLITE_SYNC_NORMAL);
  }
  if (rc == SQLITE_OK) {
    rc = sqlite3OsFileSize(pPager->jfd, &pPager->journalHdr);
  }
  return rc;
}

int pagerAcquireMapPage(Pager *pPager, Pgno pgno, void *pData, PgHdr **ppPage) {
  PgHdr *p;

  if (pPager->pMmapFreelist) {
    *ppPage = p = pPager->pMmapFreelist;
    pPager->pMmapFreelist = p->pDirty;
    p->pDirty = 0;

    memset(p->pExtra, 0, 8);
  } else {
    *ppPage = p = (PgHdr *)sqlite3MallocZero(sizeof(PgHdr) + pPager->nExtra);
    if (p == 0) {
      sqlite3OsUnfetch(pPager->fd, (i64)(pgno - 1) * pPager->pageSize, pData);
      return 7;
    }
    p->pExtra = (void *)&p[1];

    p->flags = 0x020;
    p->nRef = 1;
    p->pPager = pPager;
  }

  p->pgno = pgno;
  p->pData = pData;
  pPager->nMmapOut++;

  return SQLITE_OK;
}

void pagerFreeMapHdrs(Pager *pPager) {
  PgHdr *p;
  PgHdr *pNext;
  for (p = pPager->pMmapFreelist; p; p = pNext) {
    pNext = p->pDirty;
    sqlite3_free(p);
  }
}

int databaseIsUnmoved(Pager *pPager) {
  int bHasMoved = 0;
  int rc;

  if (pPager->tempFile)
    return SQLITE_OK;
  if (pPager->dbSize == 0)
    return 0;

  rc = sqlite3OsFileControl(pPager->fd, SQLITE_FCNTL_HAS_MOVED, &bHasMoved);
  if (rc == SQLITE_NOTFOUND) {
    rc = SQLITE_OK;
  } else if (rc == SQLITE_OK && bHasMoved) {
    rc = (8 | (4 << 8));
  }
  return rc;
}

int sqlite3PagerClose(Pager *pPager, sqlite3 *db) {
  u8 *pTmp = (u8 *)pPager->pTmpSpace;

  sqlite3BeginBenignMalloc();
  pagerFreeMapHdrs(pPager);

  pPager->exclusiveMode = 0;

  {
    u8 *a = 0;

    if (db && 0 == (db->flags & 0x00000800) && SQLITE_OK == databaseIsUnmoved(pPager)) {
      a = pTmp;
    }
    sqlite3WalClose(pPager->pWal, db, pPager->walSyncFlags, pPager->pageSize, a);
    pPager->pWal = 0;
  }

  pager_reset(pPager);
  if (pPager->memDb) {
    pager_unlock(pPager);
  } else {
    if (((pPager->jfd)->pMethods != 0)) {
      pager_error(pPager, pagerSyncHotJournal(pPager));
    }
    pagerUnlockAndRollback(pPager);
  }
  sqlite3EndBenignMalloc();

  sqlite3OsClose(pPager->jfd);
  sqlite3OsClose(pPager->fd);
  sqlite3PageFree(pTmp);
  sqlite3PcacheClose(pPager->pPCache);

  sqlite3_free(pPager);
  return SQLITE_OK;
}

int syncJournal(Pager *pPager, int newHdr) {
  int rc;

  rc = sqlite3PagerExclusiveLock(pPager);
  if (rc != SQLITE_OK)
    return rc;

  if (!pPager->noSync) {
    if (((pPager->jfd)->pMethods != 0) && pPager->journalMode != 4) {
      const int iDc = sqlite3OsDeviceCharacteristics(pPager->fd);

      if (0 == (iDc & SQLITE_IOCAP_SAFE_APPEND)) {
        i64 iNextHdrOffset;
        u8 aMagic[8];
        u8 zHeader[sizeof(aJournalMagic) + 4];

        memcpy(zHeader, aJournalMagic, sizeof(aJournalMagic));
        sqlite3Put4byte((u8 *)&zHeader[sizeof(aJournalMagic)], pPager->nRec);

        iNextHdrOffset = journalHdrOffset(pPager);
        rc = sqlite3OsRead(pPager->jfd, aMagic, 8, iNextHdrOffset);
        if (rc == SQLITE_OK && 0 == memcmp(aMagic, aJournalMagic, 8)) {
          static const u8 zerobyte = 0;
          rc = sqlite3OsWrite(pPager->jfd, &zerobyte, 1, iNextHdrOffset);
        }
        if (rc != SQLITE_OK && rc != (10 | (2 << 8))) {
          return rc;
        }

        if (pPager->fullSync && 0 == (iDc & SQLITE_IOCAP_SEQUENTIAL)) {
          rc = sqlite3OsSync(pPager->jfd, pPager->syncFlags);
          if (rc != SQLITE_OK)
            return rc;
        };
        rc = sqlite3OsWrite(pPager->jfd, zHeader, sizeof(zHeader), pPager->journalHdr);
        if (rc != SQLITE_OK)
          return rc;
      }
      if (0 == (iDc & SQLITE_IOCAP_SEQUENTIAL)) {
        rc = sqlite3OsSync(pPager->jfd,
                           pPager->syncFlags | (pPager->syncFlags == SQLITE_SYNC_FULL ? SQLITE_SYNC_DATAONLY : 0));
        if (rc != SQLITE_OK)
          return rc;
      }

      pPager->journalHdr = pPager->journalOff;
      if (newHdr && 0 == (iDc & SQLITE_IOCAP_SAFE_APPEND)) {
        pPager->nRec = 0;
        rc = writeJournalHdr(pPager);
        if (rc != SQLITE_OK)
          return rc;
      }
    } else {
      pPager->journalHdr = pPager->journalOff;
    }
  }

  sqlite3PcacheClearSyncFlags(pPager->pPCache);
  pPager->eState = 4;

  return SQLITE_OK;
}

int pager_write_pagelist(Pager *pPager, PgHdr *pList) {
  int rc = SQLITE_OK;

  if (!((pPager->fd)->pMethods != 0)) {
    rc = pagerOpentemp(pPager, pPager->fd, pPager->vfsFlags);
  }

  if (rc == SQLITE_OK && pPager->dbHintSize < pPager->dbSize && (pList->pDirty || pList->pgno > pPager->dbHintSize)) {
    sqlite3_int64 szFile = pPager->pageSize * (sqlite3_int64)pPager->dbSize;
    sqlite3OsFileControlHint(pPager->fd, SQLITE_FCNTL_SIZE_HINT, &szFile);
    pPager->dbHintSize = pPager->dbSize;
  }

  while (rc == SQLITE_OK && pList) {
    Pgno pgno = pList->pgno;

    if (pgno <= pPager->dbSize && 0 == (pList->flags & 0x010)) {
      i64 offset = (pgno - 1) * (i64)pPager->pageSize;
      char *pData;

      if (pList->pgno == 1)
        pager_write_changecounter(pList);

      pData = (char*)(pList->pData);

      rc = sqlite3OsWrite(pPager->fd, pData, pPager->pageSize, offset);

      if (pgno == 1) {
        memcpy(&pPager->dbFileVers, &pData[24], sizeof(pPager->dbFileVers));
      }
      if (pgno > pPager->dbFileSize) {
        pPager->dbFileSize = pgno;
      }
      pPager->aStat[2]++;

      sqlite3BackupUpdate(pPager->pBackup, pgno, (u8 *)pList->pData);

    } else {
    };
    pList = pList->pDirty;
  }

  return rc;
}

int openSubJournal(Pager *pPager) {
  int rc = SQLITE_OK;
  if (!((pPager->sjfd)->pMethods != 0)) {
    const int flags = SQLITE_OPEN_SUBJOURNAL | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXCLUSIVE |
                      SQLITE_OPEN_DELETEONCLOSE;
    int nStmtSpill = sqlite3Config.nStmtSpill;
    if (pPager->journalMode == 4 || pPager->subjInMemory) {
      nStmtSpill = -1;
    }
    rc = sqlite3JournalOpen(pPager->pVfs, 0, pPager->sjfd, flags, nStmtSpill);
  }
  return rc;
}

int pagerStress(void *p, PgHdr *pPg) {
  Pager *pPager = (Pager *)p;
  int rc = SQLITE_OK;

  if ((pPager->errCode))
    return 0;
  if (pPager->doNotSpill && ((pPager->doNotSpill & (0x02 | 0x01)) != 0 || (pPg->flags & 0x008) != 0)) {
    return SQLITE_OK;
  }

  pPager->aStat[3]++;
  pPg->pDirty = 0;
  if (((pPager)->pWal != 0)) {
    rc = subjournalPageIfRequired(pPg);
    if (rc == SQLITE_OK) {
      rc = pagerWalFrames(pPager, pPg, 0, 0);
    }
  } else {
    if (pPg->flags & 0x008 || pPager->eState == 3) {
      rc = syncJournal(pPager, 1);
    }

    if (rc == SQLITE_OK) {
      rc = pager_write_pagelist(pPager, pPg);
    }
  }

  if (rc == SQLITE_OK) {
    sqlite3PcacheMakeClean(pPg);
  }

  return pager_error(pPager, rc);
}

int sqlite3PagerFlush(Pager *pPager) {
  int rc = pPager->errCode;
  if (!pPager->memDb) {
    PgHdr *pList = sqlite3PcacheDirtyList(pPager->pPCache);

    while (rc == SQLITE_OK && pList) {
      PgHdr *pNext = pList->pDirty;
      if (pList->nRef == 0) {
        rc = pagerStress((void *)pPager, pList);
      }
      pList = pNext;
    }
  }

  return rc;
}

int hasHotJournal(Pager *pPager, int *pExists) {
  sqlite3_vfs *const pVfs = pPager->pVfs;
  int rc = SQLITE_OK;
  int exists = 1;
  int jrnlOpen = !!((pPager->jfd)->pMethods != 0);

  *pExists = 0;
  if (!jrnlOpen) {
    rc = sqlite3OsAccess(pVfs, pPager->zJournal, SQLITE_ACCESS_EXISTS, &exists);
  }
  if (rc == SQLITE_OK && exists) {
    int locked = 0;

    rc = sqlite3OsCheckReservedLock(pPager->fd, &locked);
    if (rc == SQLITE_OK && !locked) {
      Pgno nPage;

      rc = pagerPagecount(pPager, &nPage);
      if (rc == SQLITE_OK) {
        if (nPage == 0 && !jrnlOpen) {
          sqlite3BeginBenignMalloc();
          if (pagerLockDb(pPager, 2) == SQLITE_OK) {
            sqlite3OsDelete(pVfs, pPager->zJournal, 0);
            if (!pPager->exclusiveMode)
              pagerUnlockDb(pPager, 1);
          }
          sqlite3EndBenignMalloc();
        } else {
          if (!jrnlOpen) {
            int f = SQLITE_OPEN_READONLY | SQLITE_OPEN_MAIN_JOURNAL;
            rc = sqlite3OsOpen(pVfs, pPager->zJournal, pPager->jfd, f, &f);
          }
          if (rc == SQLITE_OK) {
            u8 first = 0;
            rc = sqlite3OsRead(pPager->jfd, (void *)&first, 1, 0);
            if (rc == (10 | (2 << 8))) {
              rc = SQLITE_OK;
            }
            if (!jrnlOpen) {
              sqlite3OsClose(pPager->jfd);
            }
            *pExists = (first != 0);
          } else if (rc == SQLITE_CANTOPEN) {
            *pExists = 1;
            rc = SQLITE_OK;
          }
        }
      }
    }
  }

  return rc;
}

int sqlite3PagerSharedLock(Pager *pPager) {
  int rc = 0;

  if (!((pPager)->pWal != 0) && pPager->eState == 0) {
    int bHotJournal = 1;

    rc = pager_wait_on_lock(pPager, 1);
    if (rc != SQLITE_OK) {
      goto failed;
    }

    if (pPager->eLock <= 1) {
      rc = hasHotJournal(pPager, &bHotJournal);
    }
    if (rc != SQLITE_OK) {
      goto failed;
    }
    if (bHotJournal) {
      if (pPager->readOnly) {
        rc = (8 | (3 << 8));
        goto failed;
      }

      rc = pagerLockDb(pPager, 4);
      if (rc != SQLITE_OK) {
        goto failed;
      }

      if (!((pPager->jfd)->pMethods != 0) && pPager->journalMode != 2) {
        sqlite3_vfs *const pVfs = pPager->pVfs;
        int bExists;
        rc = sqlite3OsAccess(pVfs, pPager->zJournal, SQLITE_ACCESS_EXISTS, &bExists);
        if (rc == SQLITE_OK && bExists) {
          int fout = 0;
          int f = SQLITE_OPEN_READWRITE | 0x00000800;

          rc = sqlite3OsOpen(pVfs, pPager->zJournal, pPager->jfd, f, &fout);

          if (rc == SQLITE_OK && fout & SQLITE_OPEN_READONLY) {
            rc = sqlite3CantopenError(65019);
            sqlite3OsClose(pPager->jfd);
          }
        }
      }

      if (((pPager->jfd)->pMethods != 0)) {
        rc = pagerSyncHotJournal(pPager);
        if (rc == SQLITE_OK) {
          rc = pager_playback(pPager, !pPager->tempFile);
          pPager->eState = 0;
        }
      } else if (!pPager->exclusiveMode) {
        pagerUnlockDb(pPager, 1);
      }

      if (rc != SQLITE_OK) {
        pager_error(pPager, rc);
        goto failed;
      }
    }

    if (!pPager->tempFile && pPager->hasHeldSharedLock) {
      char dbFileVers[sizeof(pPager->dbFileVers)];

      rc = sqlite3OsRead(pPager->fd, &dbFileVers, sizeof(dbFileVers), 24);
      if (rc != SQLITE_OK) {
        if (rc != (10 | (2 << 8))) {
          goto failed;
        }
        memset(dbFileVers, 0, sizeof(dbFileVers));
      }

      if (memcmp(pPager->dbFileVers, dbFileVers, sizeof(dbFileVers)) != 0) {
        pager_reset(pPager);

        if (((pPager)->bUseFetch)) {
          sqlite3OsUnfetch(pPager->fd, 0, 0);
        }
      }
    }

    rc = pagerOpenWalIfPresent(pPager);
  }

  if (((pPager)->pWal != 0)) {
    rc = pagerBeginReadTransaction(pPager);
  }

  if (pPager->tempFile == 0 && pPager->eState == 0 && rc == SQLITE_OK) {
    rc = pagerPagecount(pPager, &pPager->dbSize);
  }

failed:
  if (rc != SQLITE_OK) {
    pager_unlock(pPager);

  } else {
    pPager->eState = 1;
    pPager->hasHeldSharedLock = 1;
  }
  return rc;
}

void pagerUnlockIfUnused(Pager *pPager) {
  if (sqlite3PcacheRefCount(pPager->pPCache) == 0) {
    pagerUnlockAndRollback(pPager);
  }
}

int getPageNormal(Pager *pPager, Pgno pgno, DbPage **ppPage, int flags) {
  int rc = SQLITE_OK;
  PgHdr *pPg;
  u8 noContent;
  sqlite3_pcache_page *pBase;

  if (pgno == 0)
    return sqlite3CorruptError(65232);
  pBase = sqlite3PcacheFetch(pPager->pPCache, pgno, 3);
  if (pBase == 0) {
    pPg = 0;
    rc = sqlite3PcacheFetchStress(pPager->pPCache, pgno, &pBase);
    if (rc != SQLITE_OK)
      goto pager_acquire_err;
    if (pBase == 0) {
      rc = 7;
      goto pager_acquire_err;
    }
  }
  pPg = *ppPage = sqlite3PcacheFetchFinish(pPager->pPCache, pgno, pBase);

  noContent = (flags & 0x01) != 0;
  if (pPg->pPager && !noContent) {
    pPager->aStat[0]++;
    return SQLITE_OK;

  } else {
    if (pgno == ((pPager)->lckPgno)) {
      rc = sqlite3CorruptError(65264);
      goto pager_acquire_err;
    }

    pPg->pPager = pPager;

    if (!((pPager->fd)->pMethods != 0) || pPager->dbSize < pgno || noContent) {
      if (pgno > pPager->mxPgno) {
        rc = SQLITE_FULL;
        if (pgno <= pPager->dbSize) {
          sqlite3PcacheRelease(pPg);
          pPg = 0;
        }
        goto pager_acquire_err;
      }
      if (noContent) {
        sqlite3BeginBenignMalloc();
        if (pgno <= pPager->dbOrigSize) {
          sqlite3BitvecSet(pPager->pInJournal, pgno);
        }
        addToSavepointBitvecs(pPager, pgno);
        sqlite3EndBenignMalloc();
      }
      memset(pPg->pData, 0, pPager->pageSize);
    } else {
      pPager->aStat[1]++;
      rc = readDbPage(pPg);
      if (rc != SQLITE_OK) {
        goto pager_acquire_err;
      }
    };
  }
  return SQLITE_OK;

pager_acquire_err:
  if (pPg) {
    sqlite3PcacheDrop(pPg);
  }
  pagerUnlockIfUnused(pPager);
  *ppPage = 0;
  return rc;
}

int getPageMMap(Pager *pPager, Pgno pgno, DbPage **ppPage, int flags) {
  int rc = SQLITE_OK;
  PgHdr *pPg = 0;
  u32 iFrame = 0;

  const int bMmapOk = (pgno > 1 && (pPager->eState == 1 || (flags & 0x02)));

  if (pgno <= 1 && pgno == 0) {
    return sqlite3CorruptError(65347);
  }

  if (bMmapOk && ((pPager)->pWal != 0)) {
    rc = sqlite3WalFindFrame(pPager->pWal, pgno, &iFrame);
    if (rc != SQLITE_OK) {
      *ppPage = 0;
      return rc;
    }
  }
  if (bMmapOk && iFrame == 0) {
    void *pData = 0;
    rc = sqlite3OsFetch(pPager->fd, (i64)(pgno - 1) * pPager->pageSize, pPager->pageSize, &pData);
    if (rc == SQLITE_OK && pData) {
      if (pPager->eState > 1 || pPager->tempFile) {
        pPg = sqlite3PagerLookup(pPager, pgno);
      }
      if (pPg == 0) {
        rc = pagerAcquireMapPage(pPager, pgno, pData, &pPg);
      } else {
        sqlite3OsUnfetch(pPager->fd, (i64)(pgno - 1) * pPager->pageSize, pData);
      }
      if (pPg) {
        *ppPage = pPg;
        return SQLITE_OK;
      }
    }
    if (rc != SQLITE_OK) {
      *ppPage = 0;
      return rc;
    }
  }
  return getPageNormal(pPager, pgno, ppPage, flags);
}

int getPageError(Pager *pPager, Pgno pgno, DbPage **ppPage, int flags) {
  (void)(pgno);
  (void)(flags);

  *ppPage = 0;
  return pPager->errCode;
}

int sqlite3PagerGet(Pager *pPager, Pgno pgno, DbPage **ppPage, int flags) {
  return pPager->xGet(pPager, pgno, ppPage, flags);
}

DbPage *sqlite3PagerLookup(Pager *pPager, Pgno pgno) {
  sqlite3_pcache_page *pPage;

  pPage = sqlite3PcacheFetch(pPager->pPCache, pgno, 0);

  if (pPage == 0)
    return 0;
  return sqlite3PcacheFetchFinish(pPager->pPCache, pgno, pPage);
}

int pager_open_journal(Pager *pPager) {
  int rc = SQLITE_OK;
  sqlite3_vfs *const pVfs = pPager->pVfs;

  if ((pPager->errCode))
    return pPager->errCode;

  if (!((pPager)->pWal != 0) && pPager->journalMode != 2) {
    pPager->pInJournal = sqlite3BitvecCreate(pPager->dbSize);
    if (pPager->pInJournal == 0) {
      return 7;
    }

    if (!((pPager->jfd)->pMethods != 0)) {
      if (pPager->journalMode == 4) {
        sqlite3MemJournalOpen(pPager->jfd);
      } else {
        int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
        int nSpill;

        if (pPager->tempFile) {
          flags |= (SQLITE_OPEN_DELETEONCLOSE | SQLITE_OPEN_TEMP_JOURNAL);
          flags |= SQLITE_OPEN_EXCLUSIVE;
          nSpill = sqlite3Config.nStmtSpill;
        } else {
          flags |= SQLITE_OPEN_MAIN_JOURNAL;
          nSpill = jrnlBufferSize(pPager);
        }

        rc = databaseIsUnmoved(pPager);
        if (rc == SQLITE_OK) {
          rc = sqlite3JournalOpen(pVfs, pPager->zJournal, pPager->jfd, flags, nSpill);
        }
      }
    }

    if (rc == SQLITE_OK) {
      pPager->nRec = 0;
      pPager->journalOff = 0;
      pPager->setSuper = 0;
      pPager->journalHdr = 0;
      rc = writeJournalHdr(pPager);
    }
  }

  if (rc != SQLITE_OK) {
    sqlite3BitvecDestroy(pPager->pInJournal);
    pPager->pInJournal = 0;
    pPager->journalOff = 0;
  } else {
    pPager->eState = 3;
  }

  return rc;
}

int sqlite3PagerBegin(Pager *pPager, int exFlag, int subjInMemory) {
  int rc = SQLITE_OK;

  if (pPager->errCode)
    return pPager->errCode;

  pPager->subjInMemory = (u8)subjInMemory;

  if (pPager->eState == 1) {
    if (((pPager)->pWal != 0)) {
      if (pPager->exclusiveMode && sqlite3WalExclusiveMode(pPager->pWal, -1)) {
        rc = pagerLockDb(pPager, 4);
        if (rc != SQLITE_OK) {
          return rc;
        }
        (void)sqlite3WalExclusiveMode(pPager->pWal, 1);
      }

      rc = sqlite3WalBeginWriteTransaction(pPager->pWal);
    } else {
      rc = pagerLockDb(pPager, 2);
      if (rc == SQLITE_OK && exFlag) {
        rc = pager_wait_on_lock(pPager, 4);
      }
    }

    if (rc == SQLITE_OK) {
      pPager->eState = 2;
      pPager->dbHintSize = pPager->dbSize;
      pPager->dbFileSize = pPager->dbSize;
      pPager->dbOrigSize = pPager->dbSize;
      pPager->journalOff = 0;
    }
  }

  return rc;
}

int pager_incr_changecounter(Pager *pPager, int isDirectMode) {
  int rc = SQLITE_OK;

  (void)(isDirectMode);

  if (!pPager->changeCountDone && pPager->dbSize > 0) {
    PgHdr *pPgHdr;

    rc = sqlite3PagerGet(pPager, 1, &pPgHdr, 0);

    if (!0 && (rc == SQLITE_OK)) {
      rc = sqlite3PagerWrite(pPgHdr);
    }

    if (rc == SQLITE_OK) {
      pager_write_changecounter(pPgHdr);

      if (0) {
        const void *zBuf;

        zBuf = pPgHdr->pData;
        if (rc == SQLITE_OK) {
          rc = sqlite3OsWrite(pPager->fd, zBuf, pPager->pageSize, 0);
          pPager->aStat[2]++;
        }
        if (rc == SQLITE_OK) {
          const void *pCopy = (const void *)&((const char *)zBuf)[24];
          memcpy(&pPager->dbFileVers, pCopy, sizeof(pPager->dbFileVers));
          pPager->changeCountDone = 1;
        }
      } else {
        pPager->changeCountDone = 1;
      }
    }

    sqlite3PagerUnref(pPgHdr);
  }
  return rc;
}

int sqlite3PagerSync(Pager *pPager, const char *zSuper) {
  int rc = SQLITE_OK;
  void *pArg = (void *)zSuper;
  rc = sqlite3OsFileControl(pPager->fd, SQLITE_FCNTL_SYNC, pArg);
  if (rc == SQLITE_NOTFOUND)
    rc = SQLITE_OK;
  if (rc == SQLITE_OK && !pPager->noSync) {
    rc = sqlite3OsSync(pPager->fd, pPager->syncFlags);
  }
  return rc;
}

int sqlite3PagerExclusiveLock(Pager *pPager) {
  int rc = pPager->errCode;

  if (rc == SQLITE_OK) {
    if (0 == ((pPager)->pWal != 0)) {
      rc = pager_wait_on_lock(pPager, 4);
    }
  }
  return rc;
}

int sqlite3PagerCommitPhaseOne(Pager *pPager, const char *zSuper, int noSync) {
  int rc = SQLITE_OK;

  if ((pPager->errCode))
    return pPager->errCode;

  if (sqlite3FaultSim(400))
    return SQLITE_IOERR;

  if (pPager->eState < 3)
    return 0;

  if (0 == pagerFlushOnCommit(pPager, 1)) {
    sqlite3BackupRestart(pPager->pBackup);
  } else {
    PgHdr *pList;
    if (((pPager)->pWal != 0)) {
      PgHdr *pPageOne = 0;
      pList = sqlite3PcacheDirtyList(pPager->pPCache);
      if (pList == 0) {
        rc = sqlite3PagerGet(pPager, 1, &pPageOne, 0);
        pList = pPageOne;
        pList->pDirty = 0;
      }

      if ((pList)) {
        rc = pagerWalFrames(pPager, pList, pPager->dbSize, 1);
      }
      sqlite3PagerUnref(pPageOne);
      if (rc == SQLITE_OK) {
        sqlite3PcacheCleanAll(pPager->pPCache);
      }
    } else {
      rc = pager_incr_changecounter(pPager, 0);

      if (rc != SQLITE_OK)
        goto commit_phase_one_exit;

      rc = writeSuperJournal(pPager, zSuper);
      if (rc != SQLITE_OK)
        goto commit_phase_one_exit;

      rc = syncJournal(pPager, 0);
      if (rc != SQLITE_OK)
        goto commit_phase_one_exit;

      pList = sqlite3PcacheDirtyList(pPager->pPCache);

      if (0 == 0) {
        rc = pager_write_pagelist(pPager, pList);
      }
      if (rc != SQLITE_OK) {
        goto commit_phase_one_exit;
      }
      sqlite3PcacheCleanAll(pPager->pPCache);

      if (pPager->dbSize > pPager->dbFileSize) {
        Pgno nNew = pPager->dbSize - (pPager->dbSize == ((pPager)->lckPgno));

        rc = pager_truncate(pPager, nNew);
        if (rc != SQLITE_OK)
          goto commit_phase_one_exit;
      }

      if (!noSync) {
        rc = sqlite3PagerSync(pPager, zSuper);
      }
    }
  }

commit_phase_one_exit:
  if (rc == SQLITE_OK && !((pPager)->pWal != 0)) {
    pPager->eState = 5;
  }
  return rc;
}

int sqlite3PagerCommitPhaseTwo(Pager *pPager) {
  int rc = SQLITE_OK;

  if ((pPager->errCode))
    return pPager->errCode;
  pPager->iDataVersion++;

  if (pPager->eState == 2 && pPager->exclusiveMode && pPager->journalMode == 1) {
    pPager->eState = 1;
    return SQLITE_OK;
  }

  rc = pager_end_transaction(pPager, pPager->setSuper, 1);
  return pager_error(pPager, rc);
}

int sqlite3PagerRollback(Pager *pPager) {
  int rc = 0;

  if (pPager->eState == 6)
    return pPager->errCode;
  if (pPager->eState <= 1)
    return SQLITE_OK;

  if (((pPager)->pWal != 0)) {
    int rc2;
    rc = sqlite3PagerSavepoint(pPager, 2, -1);
    rc2 = pager_end_transaction(pPager, pPager->setSuper, 0);
    if (rc == SQLITE_OK)
      rc = rc2;
  } else if (!((pPager->jfd)->pMethods != 0) || pPager->eState == 2) {
    int eState = pPager->eState;
    rc = pager_end_transaction(pPager, 0, 0);
    if (!pPager->memDb && eState > 2) {
      pPager->errCode = SQLITE_ABORT;
      pPager->eState = 6;
      setGetterMethod(pPager);
      return rc;
    }
  } else {
    rc = pager_playback(pPager, 0);
  }

  return pager_error(pPager, rc);
}

u8 sqlite3PagerIsreadonly(Pager *pPager) {
  return pPager->readOnly;
}

int sqlite3PagerMemUsed(Pager *pPager) {
  int perPageSize = pPager->pageSize + pPager->nExtra + (int)(sizeof(PgHdr) + 5 * sizeof(void *));
  return perPageSize * sqlite3PcachePagecount(pPager->pPCache) + sqlite3MallocSize(pPager) + pPager->pageSize;
}

void sqlite3PagerCacheStat(Pager *pPager, int eStat, int reset, u64 *pnVal) {
  eStat -= SQLITE_DBSTATUS_CACHE_HIT;
  *pnVal += pPager->aStat[eStat];
  if (reset) {
    pPager->aStat[eStat] = 0;
  }
}

int sqlite3PagerIsMemdb(Pager *pPager) {
  return pPager->tempFile || pPager->memVfs;
}

__attribute__((noinline)) int pagerOpenSavepoint(Pager *pPager, int nSavepoint) {
  int rc = SQLITE_OK;
  int nCurrent = pPager->nSavepoint;
  int ii;
  PagerSavepoint *aNew;

  aNew = (PagerSavepoint *)sqlite3Realloc(pPager->aSavepoint, sizeof(PagerSavepoint) * nSavepoint);
  if (!aNew) {
    return 7;
  }
  memset(&aNew[nCurrent], 0, (nSavepoint - nCurrent) * sizeof(PagerSavepoint));
  pPager->aSavepoint = aNew;

  for (ii = nCurrent; ii < nSavepoint; ii++) {
    aNew[ii].nOrig = pPager->dbSize;
    if (((pPager->jfd)->pMethods != 0) && pPager->journalOff > 0) {
      aNew[ii].iOffset = pPager->journalOff;
    } else {
      aNew[ii].iOffset = (pPager->sectorSize);
    }
    aNew[ii].iSubRec = pPager->nSubRec;
    aNew[ii].pInSavepoint = sqlite3BitvecCreate(pPager->dbSize);
    aNew[ii].bTruncateOnRelease = 1;
    if (!aNew[ii].pInSavepoint) {
      return 7;
    }
    if (((pPager)->pWal != 0)) {
      sqlite3WalSavepoint(pPager->pWal, aNew[ii].aWalData);
    }
    pPager->nSavepoint = ii + 1;
  }

  return rc;
}

int sqlite3PagerOpenSavepoint(Pager *pPager, int nSavepoint) {
  if (nSavepoint > pPager->nSavepoint && pPager->useJournal) {
    return pagerOpenSavepoint(pPager, nSavepoint);
  } else {
    return SQLITE_OK;
  }
}

int sqlite3PagerSavepoint(Pager *pPager, int op, int iSavepoint) {
  int rc = pPager->errCode;

  if (rc == SQLITE_OK && iSavepoint < pPager->nSavepoint) {
    int ii;
    int nNew;

    nNew = iSavepoint + ((op == 1) ? 0 : 1);
    for (ii = nNew; ii < pPager->nSavepoint; ii++) {
      sqlite3BitvecDestroy(pPager->aSavepoint[ii].pInSavepoint);
    }
    pPager->nSavepoint = nNew;

    if (op == 1) {
      PagerSavepoint *pRel = &pPager->aSavepoint[nNew];
      if (pRel->bTruncateOnRelease && ((pPager->sjfd)->pMethods != 0)) {
        if (sqlite3JournalIsInMemory(pPager->sjfd)) {
          i64 sz = (pPager->pageSize + 4) * (i64)pRel->iSubRec;
          rc = sqlite3OsTruncate(pPager->sjfd, sz);
        }
        pPager->nSubRec = pRel->iSubRec;
      }
    }

    else if (((pPager)->pWal != 0) || ((pPager->jfd)->pMethods != 0)) {
      PagerSavepoint *pSavepoint = (nNew == 0) ? 0 : &pPager->aSavepoint[nNew - 1];
      rc = pagerPlaybackSavepoint(pPager, pSavepoint);
    }
  }

  return rc;
}

const char *sqlite3PagerFilename(const Pager *pPager, int nullIfMemDb) {
  static const char zFake[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  if (nullIfMemDb && (pPager->memDb || sqlite3IsMemdb(pPager->pVfs))) {
    return &zFake[4];
  } else {
    return pPager->zFilename;
  }
}

sqlite3_vfs *sqlite3PagerVfs(Pager *pPager) {
  return pPager->pVfs;
}

sqlite3_file *sqlite3PagerFile(Pager *pPager) {
  return pPager->fd;
}

sqlite3_file *sqlite3PagerJrnlFile(Pager *pPager) {
  return pPager->pWal ? sqlite3WalFile(pPager->pWal) : pPager->jfd;
}

const char *sqlite3PagerJournalname(Pager *pPager) {
  return pPager->zJournal;
}

int sqlite3PagerMovepage(Pager *pPager, DbPage *pPg, Pgno pgno, int isCommit) {
  PgHdr *pPgOld;
  Pgno needSyncPgno = 0;
  int rc;
  Pgno origPgno;

  if (pPager->tempFile) {
    rc = sqlite3PagerWrite(pPg);
    if (rc)
      return rc;
  }

  if ((pPg->flags & 0x002) != 0 && SQLITE_OK != (rc = subjournalPageIfRequired(pPg))) {
    return rc;
  }

  if ((pPg->flags & 0x008) && !isCommit) {
    needSyncPgno = pPg->pgno;
  }

  pPg->flags &= ~0x008;
  pPgOld = sqlite3PagerLookup(pPager, pgno);

  if (pPgOld) {
    if ((pPgOld->nRef > 1)) {
      sqlite3PagerUnrefNotNull(pPgOld);
      return sqlite3CorruptError(66913);
    }
    pPg->flags |= (pPgOld->flags & 0x008);
    if (pPager->tempFile) {
      sqlite3PcacheMove(pPgOld, pPager->dbSize + 1);
    } else {
      sqlite3PcacheDrop(pPgOld);
    }
  }

  origPgno = pPg->pgno;
  sqlite3PcacheMove(pPg, pgno);
  sqlite3PcacheMakeDirty(pPg);

  if (pPager->tempFile && pPgOld) {
    sqlite3PcacheMove(pPgOld, origPgno);
    sqlite3PagerUnrefNotNull(pPgOld);
  }

  if (needSyncPgno) {
    PgHdr *pPgHdr;
    rc = sqlite3PagerGet(pPager, needSyncPgno, &pPgHdr, 0);
    if (rc != SQLITE_OK) {
      if (needSyncPgno <= pPager->dbOrigSize) {
        sqlite3BitvecClear(pPager->pInJournal, needSyncPgno, pPager->pTmpSpace);
      }
      return rc;
    }
    pPgHdr->flags |= 0x008;
    sqlite3PcacheMakeDirty(pPgHdr);
    sqlite3PagerUnrefNotNull(pPgHdr);
  }

  return SQLITE_OK;
}

int sqlite3PagerLockingMode(Pager *pPager, int eMode) {
  if (eMode >= 0 && !pPager->tempFile && !sqlite3WalHeapMemory(pPager->pWal)) {
    pPager->exclusiveMode = (u8)eMode;
  }
  return (int)pPager->exclusiveMode;
}

int sqlite3PagerSetJournalMode(Pager *pPager, int eMode) {
  u8 eOld = pPager->journalMode;

  if (pPager->memDb) {
    if (eMode != 4 && eMode != 2) {
      eMode = eOld;
    }
  }

  if (eMode != eOld) {
    pPager->journalMode = (u8)eMode;

    if (!pPager->exclusiveMode && (eOld & 5) == 1 && (eMode & 1) == 0) {
      sqlite3OsClose(pPager->jfd);
      if (pPager->eLock >= 2) {
        sqlite3OsDelete(pPager->pVfs, pPager->zJournal, 0);
      } else {
        int rc = SQLITE_OK;
        int state = pPager->eState;

        if (state == 0) {
          rc = sqlite3PagerSharedLock(pPager);
        }
        if (pPager->eState == 1) {
          rc = pagerLockDb(pPager, 2);
        }
        if (rc == SQLITE_OK) {
          sqlite3OsDelete(pPager->pVfs, pPager->zJournal, 0);
        }
        if (rc == SQLITE_OK && state == 1) {
          pagerUnlockDb(pPager, 1);
        } else if (state == 0) {
          pager_unlock(pPager);
        }
      }
    } else if (eMode == 2 || eMode == 4) {
      sqlite3OsClose(pPager->jfd);
    }
  }

  return (int)pPager->journalMode;
}

int sqlite3PagerGetJournalMode(Pager *pPager) {
  return (int)pPager->journalMode;
}

int sqlite3PagerOkToChangeJournalMode(Pager *pPager) {
  if (pPager->eState >= 3)
    return 0;
  if ((((pPager->jfd)->pMethods != 0) && pPager->journalOff > 0))
    return 0;
  return 1;
}

i64 sqlite3PagerJournalSizeLimit(Pager *pPager, i64 iLimit) {
  if (iLimit >= -1) {
    pPager->journalSizeLimit = iLimit;
    sqlite3WalLimit(pPager->pWal, iLimit);
  }
  return pPager->journalSizeLimit;
}

sqlite3_backup **sqlite3PagerBackupPtr(Pager *pPager) {
  return &pPager->pBackup;
}

void sqlite3PagerClearCache(Pager *pPager) {
  if (pPager->tempFile == 0)
    pager_reset(pPager);
}

int sqlite3PagerCheckpoint(Pager *pPager, sqlite3 *db, int eMode, int *pnLog, int *pnCkpt) {
  int rc = SQLITE_OK;
  if (pPager->pWal == 0 && pPager->journalMode == 5) {
    sqlite3_exec(db, "PRAGMA table_list", 0, 0, 0);
  }
  if (pPager->pWal) {
    rc = sqlite3WalCheckpoint(pPager->pWal, db, eMode, (eMode <= SQLITE_CHECKPOINT_PASSIVE ? 0 : pPager->xBusyHandler),
                              pPager->pBusyHandlerArg, pPager->walSyncFlags, pPager->pageSize, (u8 *)pPager->pTmpSpace,
                              pnLog, pnCkpt);
  }
  return rc;
}

int sqlite3PagerWalCallback(Pager *pPager) {
  return sqlite3WalCallback(pPager->pWal);
}

int sqlite3PagerWalSupported(Pager *pPager) {
  const sqlite3_io_methods *pMethods = pPager->fd->pMethods;
  if (pPager->noLock)
    return 0;
  return pPager->exclusiveMode || (pMethods->iVersion >= 2 && pMethods->xShmMap);
}

int pagerExclusiveLock(Pager *pPager) {
  int rc;
  u8 eOrigLock;

  eOrigLock = pPager->eLock;
  rc = pagerLockDb(pPager, 4);
  if (rc != SQLITE_OK) {
    pagerUnlockDb(pPager, eOrigLock);
  }

  return rc;
}

int pagerOpenWal(Pager *pPager) {
  int rc = SQLITE_OK;

  if (pPager->exclusiveMode) {
    rc = pagerExclusiveLock(pPager);
  }

  if (rc == SQLITE_OK) {
    rc = sqlite3WalOpen(pPager->pVfs, pPager->fd, pPager->zWal, pPager->exclusiveMode, pPager->journalSizeLimit,
                        &pPager->pWal);
  }
  pagerFixMaplimit(pPager);

  return rc;
}

int sqlite3PagerOpenWal(Pager *pPager, int *pbOpen) {
  int rc = 0;

  if (!pPager->tempFile && !pPager->pWal) {
    if (!sqlite3PagerWalSupported(pPager))
      return SQLITE_CANTOPEN;

    sqlite3OsClose(pPager->jfd);

    rc = pagerOpenWal(pPager);
    if (rc == SQLITE_OK) {
      pPager->journalMode = 5;
      pPager->eState = 0;
    }
  } else {
    *pbOpen = 1;
  }

  return rc;
}

int sqlite3PagerCloseWal(Pager *pPager, sqlite3 *db) {
  int rc = SQLITE_OK;

  if (!pPager->pWal) {
    int logexists = 0;
    rc = pagerLockDb(pPager, 1);
    if (rc == SQLITE_OK) {
      rc = sqlite3OsAccess(pPager->pVfs, pPager->zWal, SQLITE_ACCESS_EXISTS, &logexists);
    }
    if (rc == SQLITE_OK && logexists) {
      rc = pagerOpenWal(pPager);
    }
  }

  if (rc == SQLITE_OK && pPager->pWal) {
    rc = pagerExclusiveLock(pPager);
    if (rc == SQLITE_OK) {
      rc = sqlite3WalClose(pPager->pWal, db, pPager->walSyncFlags, pPager->pageSize, (u8 *)pPager->pTmpSpace);
      pPager->pWal = 0;
      pagerFixMaplimit(pPager);
      if (rc && !pPager->exclusiveMode)
        pagerUnlockDb(pPager, 1);
    }
  }
  return rc;
}