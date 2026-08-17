#define _GNU_SOURCE 1
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <utime.h>
#include "sqlite/sqlite3_file.h"
#include "sqlite/SqliteUnixSyscallIndex.h"
#include "sqlite/FileChunk.h"
#include "sqlite/FilePoint.h"
#include "sqlite/MemFS.h"
#include "sqlite/MemFile.h"
#include "sqlite/MemJournal.h"
#include "sqlite/MemStore.h"
#include "sqlite/Pager.h"
#include "sqlite/PmaWriter.h"
#include "sqlite/Sqlite3Config.h"
#include "sqlite/UnixUnusedFd.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_io_methods.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/sqlite3_syscall_ptr.h"
#include "sqlite/sqlite3_uint64.h"
#include "sqlite/sqlite3_vfs.h"
#include "sqlite/sqlite_int64.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/unixFile.h"
#include "sqlite/unixInodeInfo.h"
#include "sqlite/unixShm.h"
#include "sqlite/unixShmNode.h"
#include "sqlite/unix_syscall.h"
#include "sqlite/SqliteDeserializeFlags.h"
#include "sqlite/SqliteFileControlOpcode.h"
#include "sqlite/SqliteIoCap.h"
#include "sqlite/SqliteLockLevel.h"
#include "sqlite/SqliteMutexType.h"
#include "sqlite/SqliteResultCode.h"
#include "sqlite/SqliteShmFlags.h"
#include "sqlite/SqliteSyncFlags.h"
/* Private helpers, formerly declared in _Uncategorized.h. */
static int sqliteErrorFromPosixError(int posixError, int sqliteIOErr);

static int sqliteErrorFromPosixError(int posixError, int sqliteIOErr) {
  switch (posixError) {
    case 13:
    case 11:
    case 110:
    case 16:
    case 4:
    case 37:
      return SQLITE_BUSY;

    case 1:
      return SQLITE_PERM;

    default:
      return sqliteIOErr;
  }
}

void sqlite3OsClose(sqlite3_file *pId) {
  if (pId->pMethods) {
    pId->pMethods->xClose(pId);
    pId->pMethods = 0;
  }
}

int sqlite3OsRead(sqlite3_file *id, void *pBuf, int amt, i64 offset) {
  return id->pMethods->xRead(id, pBuf, amt, offset);
}

int sqlite3OsWrite(sqlite3_file *id, const void *pBuf, int amt, i64 offset) {
  return id->pMethods->xWrite(id, pBuf, amt, offset);
}

int sqlite3OsTruncate(sqlite3_file *id, i64 size) {
  return id->pMethods->xTruncate(id, size);
}

int sqlite3OsSync(sqlite3_file *id, int flags) {
  return flags ? id->pMethods->xSync(id, flags) : SQLITE_OK;
}

int sqlite3OsFileSize(sqlite3_file *id, i64 *pSize) {
  return id->pMethods->xFileSize(id, pSize);
}

int sqlite3OsLock(sqlite3_file *id, int lockType) {
  return id->pMethods->xLock(id, lockType);
}

int sqlite3OsUnlock(sqlite3_file *id, int lockType) {
  return id->pMethods->xUnlock(id, lockType);
}

int sqlite3OsCheckReservedLock(sqlite3_file *id, int *pResOut) {
  return id->pMethods->xCheckReservedLock(id, pResOut);
}

int sqlite3OsFileControl(sqlite3_file *id, int op, void *pArg) {
  if (id->pMethods == 0)
    return SQLITE_NOTFOUND;

  return id->pMethods->xFileControl(id, op, pArg);
}

void sqlite3OsFileControlHint(sqlite3_file *id, int op, void *pArg) {
  if (id->pMethods)
    (void)id->pMethods->xFileControl(id, op, pArg);
}

int sqlite3OsSectorSize(sqlite3_file *id) {
  int (*xSectorSize)(sqlite3_file *) = id->pMethods->xSectorSize;
  return (xSectorSize ? xSectorSize(id) : 4096);
}

int sqlite3OsDeviceCharacteristics(sqlite3_file *id) {
  if (id->pMethods == 0)
    return 0;
  return id->pMethods->xDeviceCharacteristics(id);
}

int sqlite3OsShmLock(sqlite3_file *id, int offset, int n, int flags) {
  return id->pMethods->xShmLock(id, offset, n, flags);
}

void sqlite3OsShmBarrier(sqlite3_file *id) {
  id->pMethods->xShmBarrier(id);
}

int sqlite3OsShmUnmap(sqlite3_file *id, int deleteFlag) {
  return id->pMethods->xShmUnmap(id, deleteFlag);
}

int sqlite3OsShmMap(sqlite3_file *id, int iPage, int pgsz, int bExtend, void volatile **pp) {
  return id->pMethods->xShmMap(id, iPage, pgsz, bExtend, pp);
}

int sqlite3OsFetch(sqlite3_file *id, i64 iOff, int iAmt, void **pp) {
  return id->pMethods->xFetch(id, iOff, iAmt, pp);
}

int sqlite3OsUnfetch(sqlite3_file *id, i64 iOff, void *p) {
  return id->pMethods->xUnfetch(id, iOff, p);
}

void sqlite3OsCloseFree(sqlite3_file *pFile) {
  sqlite3OsClose(pFile);
  sqlite3_free(pFile);
}

int unixCheckReservedLock(sqlite3_file *id, int *pResOut) {
  int rc = SQLITE_OK;
  int reserved = 0;
  unixFile *pFile = (unixFile *)id;

  sqlite3_mutex_enter(pFile->pInode->pLockMutex);

  if (pFile->pInode->eFileLock > 1) {
    reserved = 1;
  }

  if (!reserved && !pFile->pInode->bProcessLock) {
    struct flock lock;
    lock.l_whence = 0;
    lock.l_start = (sqlite3PendingByte + 1);
    lock.l_len = 1;
    lock.l_type = 1;
    if (((int (*)(int, int, ...))aSyscall[SQLITE_SYSCALL_FCNTL].pCurrent)(pFile->h, 5, &lock)) {
      rc = (10 | (14 << 8));
      storeLastErrno(pFile, (*__errno_location()));
    } else if (lock.l_type != 2) {
      reserved = 1;
    }
  }

  sqlite3_mutex_leave(pFile->pInode->pLockMutex);

  *pResOut = reserved;
  return rc;
}

int unixLock(sqlite3_file *id, int eFileLock) {
  int rc = SQLITE_OK;
  unixFile *pFile = (unixFile *)id;
  unixInodeInfo *pInode;
  struct flock lock;
  int tErrno = 0;

  if (pFile->eFileLock >= eFileLock) {
    return SQLITE_OK;
  }

  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->pLockMutex);

  if ((pFile->eFileLock != pInode->eFileLock && (pInode->eFileLock >= 3 || eFileLock > 1))) {
    rc = SQLITE_BUSY;
    goto end_lock;
  }

  if (eFileLock == 1 && (pInode->eFileLock == 1 || pInode->eFileLock == 2)) {
    pFile->eFileLock = 1;
    pInode->nShared++;
    pInode->nLock++;
    goto end_lock;
  }

  lock.l_len = 1L;
  lock.l_whence = 0;
  if (eFileLock == 1 || (eFileLock == 4 && pFile->eFileLock == 2)) {
    lock.l_type = (eFileLock == 1 ? 0 : 1);
    lock.l_start = sqlite3PendingByte;
    if (unixFileLock(pFile, &lock)) {
      tErrno = (*__errno_location());
      rc = sqliteErrorFromPosixError(tErrno, (10 | (15 << 8)));
      if (rc != SQLITE_BUSY) {
        storeLastErrno(pFile, tErrno);
      }
      goto end_lock;
    } else if (eFileLock == 4) {
      pFile->eFileLock = 3;
      pInode->eFileLock = 3;
    }
  }

  if (eFileLock == 1) {
    lock.l_start = (sqlite3PendingByte + 2);
    lock.l_len = 510;
    if (unixFileLock(pFile, &lock)) {
      tErrno = (*__errno_location());
      rc = sqliteErrorFromPosixError(tErrno, (10 | (15 << 8)));
    }

    lock.l_start = sqlite3PendingByte;
    lock.l_len = 1L;
    lock.l_type = 2;
    if (unixFileLock(pFile, &lock) && rc == SQLITE_OK) {
      tErrno = (*__errno_location());
      rc = (10 | (8 << 8));
    }

    if (rc) {
      if (rc != SQLITE_BUSY) {
        storeLastErrno(pFile, tErrno);
      }
      goto end_lock;
    } else {
      pFile->eFileLock = 1;
      pInode->nLock++;
      pInode->nShared = 1;
    }
  } else if (eFileLock == 4 && pInode->nShared > 1) {
    rc = SQLITE_BUSY;
  } else if (unixIsSharingShmNode(pFile)) {
    rc = SQLITE_BUSY;
  } else {
    lock.l_type = 1;

    if (eFileLock == 2) {
      lock.l_start = (sqlite3PendingByte + 1);
      lock.l_len = 1L;
    } else {
      lock.l_start = (sqlite3PendingByte + 2);
      lock.l_len = 510;
    }

    if (unixFileLock(pFile, &lock)) {
      tErrno = (*__errno_location());
      rc = sqliteErrorFromPosixError(tErrno, (10 | (15 << 8)));
      if (rc != SQLITE_BUSY) {
        storeLastErrno(pFile, tErrno);
      }
    }
  }

  if (rc == 0) {
    pFile->eFileLock = eFileLock;
    pInode->eFileLock = eFileLock;
  }

end_lock:
  sqlite3_mutex_leave(pInode->pLockMutex);

  return rc;
}

int posixUnlock(sqlite3_file *id, int eFileLock, int handleNFSUnlock) {
  unixFile *pFile = (unixFile *)id;
  unixInodeInfo *pInode;
  struct flock lock;
  int rc = 0;

  if (pFile->eFileLock <= eFileLock) {
    return SQLITE_OK;
  }
  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->pLockMutex);

  if (pFile->eFileLock > 1) {
    if (eFileLock == 1) {
      (void)handleNFSUnlock;

      {
        lock.l_type = 0;
        lock.l_whence = 0;
        lock.l_start = (sqlite3PendingByte + 2);
        lock.l_len = 510;
        if (unixFileLock(pFile, &lock)) {
          rc = (10 | (9 << 8));
          storeLastErrno(pFile, (*__errno_location()));
          goto end_unlock;
        }
      }
    }
    lock.l_type = 2;
    lock.l_whence = 0;
    lock.l_start = sqlite3PendingByte;
    lock.l_len = 2L;

    if (unixFileLock(pFile, &lock) == 0) {
      pInode->eFileLock = 1;
    } else {
      rc = (10 | (8 << 8));
      storeLastErrno(pFile, (*__errno_location()));
      goto end_unlock;
    }
  }
  if (eFileLock == 0) {
    pInode->nShared--;
    if (pInode->nShared == 0) {
      lock.l_type = 2;
      lock.l_whence = 0;
      lock.l_start = lock.l_len = 0L;
      if (unixFileLock(pFile, &lock) == 0) {
        pInode->eFileLock = 0;
      } else {
        rc = (10 | (8 << 8));
        storeLastErrno(pFile, (*__errno_location()));
        pInode->eFileLock = 0;
        pFile->eFileLock = 0;
      }
    }

    pInode->nLock--;

    if (pInode->nLock == 0)
      closePendingFds(pFile);
  }

end_unlock:
  sqlite3_mutex_leave(pInode->pLockMutex);
  if (rc == SQLITE_OK) {
    pFile->eFileLock = eFileLock;
  }
  return rc;
}

int unixUnlock(sqlite3_file *id, int eFileLock) {
  return posixUnlock(id, eFileLock, 0);
}

int closeUnixFile(sqlite3_file *id) {
  unixFile *pFile = (unixFile *)id;

  unixUnmapfile(pFile);

  if (pFile->h >= 0) {
    robust_close(pFile, pFile->h, 42509);
    pFile->h = -1;
  }

  sqlite3_free(pFile->pPreallocatedUnused);
  memset(pFile, 0, sizeof(unixFile));
  return SQLITE_OK;
}

int unixClose(sqlite3_file *id) {
  int rc = SQLITE_OK;
  unixFile *pFile = (unixFile *)id;
  unixInodeInfo *pInode = pFile->pInode;

  verifyDbFile(pFile);
  unixUnlock(id, 0);

  unixEnterMutex();

  sqlite3_mutex_enter(pInode->pLockMutex);
  if (pInode->nLock) {
    setPendingFd(pFile);
  }
  sqlite3_mutex_leave(pInode->pLockMutex);
  releaseInodeInfo(pFile);

  rc = closeUnixFile(id);
  unixLeaveMutex();
  return rc;
}

int nolockCheckReservedLock(sqlite3_file *NotUsed, int *pResOut) {
  (void)(NotUsed);
  *pResOut = 0;
  return SQLITE_OK;
}

int nolockLock(sqlite3_file *NotUsed, int NotUsed2) {
  (void)(NotUsed), (void)(NotUsed2);
  return SQLITE_OK;
}

int nolockUnlock(sqlite3_file *NotUsed, int NotUsed2) {
  (void)(NotUsed), (void)(NotUsed2);
  return SQLITE_OK;
}

int nolockClose(sqlite3_file *id) {
  return closeUnixFile(id);
}

int dotlockCheckReservedLock(sqlite3_file *id, int *pResOut) {
  unixFile *pFile = (unixFile *)id;

  if (pFile->eFileLock >= 1) {
    *pResOut = 0;
  } else {
    *pResOut = ((int (*)(const char *, int))aSyscall[SQLITE_SYSCALL_ACCESS].pCurrent)(
                   (const char *)pFile->lockingContext, 0) == 0;
  };
  return SQLITE_OK;
}

int dotlockLock(sqlite3_file *id, int eFileLock) {
  unixFile *pFile = (unixFile *)id;
  char *zLockFile = (char *)pFile->lockingContext;
  int rc = SQLITE_OK;

  if (pFile->eFileLock > 0) {
    pFile->eFileLock = eFileLock;

    utime(zLockFile, ((void *)0));

    return SQLITE_OK;
  }

  rc = ((int (*)(const char *, mode_t))aSyscall[SQLITE_SYSCALL_MKDIR].pCurrent)(zLockFile, 0777);
  if (rc < 0) {
    int tErrno = (*__errno_location());
    if (17 == tErrno) {
      rc = SQLITE_BUSY;
    } else {
      rc = sqliteErrorFromPosixError(tErrno, (10 | (15 << 8)));
      if (rc != SQLITE_BUSY) {
        storeLastErrno(pFile, tErrno);
      }
    }
    return rc;
  }

  pFile->eFileLock = eFileLock;
  return rc;
}

int dotlockUnlock(sqlite3_file *id, int eFileLock) {
  unixFile *pFile = (unixFile *)id;
  char *zLockFile = (char *)pFile->lockingContext;
  int rc;

  if (pFile->eFileLock == eFileLock) {
    return SQLITE_OK;
  }

  if (eFileLock == 1) {
    pFile->eFileLock = 1;
    return SQLITE_OK;
  }

  rc = ((int (*)(const char *))aSyscall[SQLITE_SYSCALL_RMDIR].pCurrent)(zLockFile);
  if (rc < 0) {
    int tErrno = (*__errno_location());
    if (tErrno == 2) {
      rc = SQLITE_OK;
    } else {
      rc = (10 | (8 << 8));
      storeLastErrno(pFile, tErrno);
    }
    return rc;
  }
  pFile->eFileLock = 0;
  return SQLITE_OK;
}

int dotlockClose(sqlite3_file *id) {
  unixFile *pFile = (unixFile *)id;

  dotlockUnlock(id, 0);
  sqlite3_free(pFile->lockingContext);
  return closeUnixFile(id);
}

int unixRead(sqlite3_file *id, void *pBuf, int amt, sqlite3_int64 offset) {
  unixFile *pFile = (unixFile *)id;
  int got;

  if (offset < pFile->mmapSize) {
    if (offset + amt <= pFile->mmapSize) {
      memcpy(pBuf, &((u8 *)(pFile->pMapRegion))[offset], amt);
      return SQLITE_OK;
    } else {
      int nCopy = pFile->mmapSize - offset;
      memcpy(pBuf, &((u8 *)(pFile->pMapRegion))[offset], nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      offset += nCopy;
    }
  }

  got = seekAndRead(pFile, offset, pBuf, amt);
  if (got == amt) {
    return SQLITE_OK;
  } else if (got < 0) {
    switch (pFile->lastErrno) {
      case 34:
      case 5:
      case 6:
        return (10 | (33 << 8));
    }
    return (10 | (1 << 8));
  } else {
    storeLastErrno(pFile, 0);

    memset(&((char *)pBuf)[got], 0, amt - got);
    return (10 | (2 << 8));
  }
}

int unixWrite(sqlite3_file *id, const void *pBuf, int amt, sqlite3_int64 offset) {
  unixFile *pFile = (unixFile *)id;
  int wrote = 0;

  while ((wrote = seekAndWrite(pFile, offset, pBuf, amt)) < amt && wrote > 0) {
    amt -= wrote;
    offset += wrote;
    pBuf = &((char *)pBuf)[wrote];
  };

  if (amt > wrote) {
    if (wrote < 0 && pFile->lastErrno != 28) {
      return (10 | (3 << 8));
    } else {
      storeLastErrno(pFile, 0);
      return SQLITE_FULL;
    }
  }

  return SQLITE_OK;
}

int unixSync(sqlite3_file *id, int flags) {
  int rc;
  unixFile *pFile = (unixFile *)id;

  int isDataOnly = (flags & SQLITE_SYNC_DATAONLY);
  int isFullsync = (flags & 0x0F) == 0x00003;

  rc = full_fsync(pFile->h, isFullsync, isDataOnly);
  if (rc) {
    storeLastErrno(pFile, (*__errno_location()));
    return unixLogErrorAtLine((10 | (4 << 8)), "full_fsync", pFile->zPath, 44131);
  }

  if (pFile->ctrlFlags & 0x08) {
    int dirfd;

    rc = ((int (*)(const char *, int *))aSyscall[SQLITE_SYSCALL_OPENDIRECTORY].pCurrent)(pFile->zPath, &dirfd);
    if (rc == SQLITE_OK) {
      full_fsync(dirfd, 0, 0);
      robust_close(pFile, dirfd, 44145);
    } else {
      rc = SQLITE_OK;
    }
    pFile->ctrlFlags &= ~0x08;
  }
  return rc;
}

int unixTruncate(sqlite3_file *id, i64 nByte) {
  unixFile *pFile = (unixFile *)id;
  int rc;

  if (pFile->szChunk > 0) {
    nByte = ((nByte + pFile->szChunk - 1) / pFile->szChunk) * pFile->szChunk;
  }

  rc = robust_ftruncate(pFile->h, nByte);
  if (rc) {
    storeLastErrno(pFile, (*__errno_location()));
    return unixLogErrorAtLine((10 | (6 << 8)), "ftruncate", pFile->zPath, 44176);
  } else {
    if (nByte < pFile->mmapSize) {
      pFile->mmapSize = nByte;
    }

    return SQLITE_OK;
  }
}

int unixFileSize(sqlite3_file *id, i64 *pSize) {
  int rc;
  struct stat buf;

  rc = ((int (*)(int, struct stat *))aSyscall[SQLITE_SYSCALL_FSTAT].pCurrent)(((unixFile *)id)->h, &buf);
  if (rc != 0) {
    storeLastErrno((unixFile *)id, (*__errno_location()));
    return (10 | (7 << 8));
  }
  *pSize = buf.st_size;

  if (*pSize == 1)
    *pSize = 0;

  return SQLITE_OK;
}

int unixFileControl(sqlite3_file *id, int op, void *pArg) {
  unixFile *pFile = (unixFile *)id;
  switch (op) {
    case 43: {
      ((int (*)(int))aSyscall[SQLITE_SYSCALL_CLOSE].pCurrent)(pFile->h);
      pFile->h = -1;
      return SQLITE_OK;
    }
    case SQLITE_FCNTL_LOCKSTATE: {
      *(int *)pArg = pFile->eFileLock;
      return SQLITE_OK;
    }
    case SQLITE_FCNTL_LAST_ERRNO: {
      *(int *)pArg = pFile->lastErrno;
      return SQLITE_OK;
    }
    case SQLITE_FCNTL_CHUNK_SIZE: {
      pFile->szChunk = *(int *)pArg;
      return SQLITE_OK;
    }
    case SQLITE_FCNTL_SIZE_HINT: {
      int rc;
      rc = fcntlSizeHint(pFile, *(i64 *)pArg);
      return rc;
    }
    case SQLITE_FCNTL_PERSIST_WAL: {
      unixModeBit(pFile, 0x04, (int *)pArg);
      return SQLITE_OK;
    }
    case SQLITE_FCNTL_POWERSAFE_OVERWRITE: {
      unixModeBit(pFile, 0x10, (int *)pArg);
      return SQLITE_OK;
    }
    case SQLITE_FCNTL_VFSNAME: {
      *(char **)pArg = sqlite3_mprintf("%s", pFile->pVfs->zName);
      return SQLITE_OK;
    }
    case SQLITE_FCNTL_TEMPFILENAME: {
      char *zTFile = sqlite3_malloc64(pFile->pVfs->mxPathname);
      if (zTFile) {
        unixGetTempname(pFile->pVfs->mxPathname, zTFile);
        *(char **)pArg = zTFile;
      }
      return SQLITE_OK;
    }
    case SQLITE_FCNTL_HAS_MOVED: {
      *(int *)pArg = fileHasMoved(pFile);
      return 0;
    }

    case 18: {
      i64 newLimit = *(i64 *)pArg;
      int rc = SQLITE_OK;
      if (newLimit > sqlite3Config.mxMmap) {
        newLimit = sqlite3Config.mxMmap;
      }

      if (newLimit > 0 && sizeof(size_t) < 8) {
        newLimit = (newLimit & 0x7FFFFFFF);
      }

      *(i64 *)pArg = pFile->mmapSizeMax;
      if (newLimit >= 0 && newLimit != pFile->mmapSizeMax && pFile->nFetchOut == 0) {
        pFile->mmapSizeMax = newLimit;
        if (pFile->mmapSize > 0) {
          unixUnmapfile(pFile);
          rc = unixMapfile(pFile, -1);
        }
      }
      return rc;
    }

    case 40: {
      return unixFcntlExternalReader((unixFile *)id, (int *)pArg);
    }
  }
  return 12;
}

int unixSectorSize(sqlite3_file *id) {
  unixFile *pFd = (unixFile *)id;
  setDeviceCharacteristics(pFd);
  return pFd->sectorSize;
}

int unixDeviceCharacteristics(sqlite3_file *id) {
  unixFile *pFd = (unixFile *)id;
  setDeviceCharacteristics(pFd);
  return pFd->deviceCharacteristics;
}

int unixShmMap(sqlite3_file *fd, int iRegion, int szRegion, int bExtend, void volatile **pp) {
  unixFile *pDbFd = (unixFile *)fd;
  unixShm *p;
  unixShmNode *pShmNode;
  int rc = SQLITE_OK;
  int nShmPerMap = unixShmRegionPerMap();
  int nReqRegion;

  if (pDbFd->pShm == 0) {
    rc = unixOpenSharedMemory(pDbFd);
    if (rc != SQLITE_OK)
      return rc;
  }

  p = pDbFd->pShm;
  pShmNode = p->pShmNode;
  sqlite3_mutex_enter(pShmNode->pShmMutex);
  if (pShmNode->isUnlocked) {
    rc = unixLockSharedMemory(pDbFd, pShmNode);
    if (rc != SQLITE_OK)
      goto shmpage_out;
    pShmNode->isUnlocked = 0;
  }

  nReqRegion = ((iRegion + nShmPerMap) / nShmPerMap) * nShmPerMap;

  if (pShmNode->nRegion < nReqRegion) {
    char **apNew;
    i64 nByte = nReqRegion * (i64)szRegion;
    struct stat sStat;

    pShmNode->szRegion = szRegion;

    if (pShmNode->hShm >= 0) {
      if (((int (*)(int, struct stat *))aSyscall[SQLITE_SYSCALL_FSTAT].pCurrent)(pShmNode->hShm, &sStat)) {
        rc = (10 | (19 << 8));
        goto shmpage_out;
      }

      if (sStat.st_size < nByte) {
        if (!bExtend) {
          goto shmpage_out;
        }

        else {
          static const int pgsz = 4096;
          i64 iPg;

          for (iPg = (sStat.st_size / pgsz); iPg < (nByte / pgsz); iPg++) {
            int x = 0;
            if (seekAndWriteFd(pShmNode->hShm, iPg * pgsz + pgsz - 1, "", 1, &x) != 1) {
              const char *zFile = pShmNode->zFilename;
              rc = unixLogErrorAtLine((10 | (19 << 8)), "write", zFile, 45381);
              goto shmpage_out;
            }
          }
        }
      }
    }

    apNew = (char **)sqlite3_realloc64(pShmNode->apRegion, nReqRegion * sizeof(char *));
    if (!apNew) {
      rc = (10 | (12 << 8));
      goto shmpage_out;
    }
    pShmNode->apRegion = apNew;
    while (pShmNode->nRegion < nReqRegion) {
      i64 nMap = (i64)szRegion * (i64)nShmPerMap;
      i64 i;
      void *pMem;
      if (pShmNode->hShm >= 0) {
        pMem = ((void *(*)(void *, size_t, int, int, int, off_t))aSyscall[SQLITE_SYSCALL_MMAP].pCurrent)(
            0, nMap, pShmNode->isReadonly ? 0x1 : 0x1 | 0x2, 0x01, pShmNode->hShm, szRegion * (i64)pShmNode->nRegion);
        if (pMem == ((void *)-1)) {
          rc = unixLogErrorAtLine((10 | (21 << 8)), "mmap", pShmNode->zFilename, 45408);
          goto shmpage_out;
        }
      } else {
        pMem = sqlite3_malloc64(nMap);
        if (pMem == 0) {
          rc = 7;
          goto shmpage_out;
        }
        memset(pMem, 0, nMap);
      }

      for (i = 0; i < nShmPerMap; i++) {
        pShmNode->apRegion[pShmNode->nRegion + i] = &((char *)pMem)[szRegion * i];
      }
      pShmNode->nRegion += nShmPerMap;
    }
  }

shmpage_out:
  if (pShmNode->nRegion > iRegion) {
    *pp = pShmNode->apRegion[iRegion];
  } else {
    *pp = 0;
  }
  if (pShmNode->isReadonly && rc == SQLITE_OK)
    rc = SQLITE_READONLY;
  sqlite3_mutex_leave(pShmNode->pShmMutex);
  return rc;
}

int unixShmLock(sqlite3_file *fd, int ofst, int n, int flags) {
  unixFile *pDbFd = (unixFile *)fd;
  unixShm *p;
  unixShmNode *pShmNode;
  int rc = SQLITE_OK;
  u16 mask = (1 << (ofst + n)) - (1 << ofst);
  int *aLock;

  p = pDbFd->pShm;
  if (p == 0)
    return (10 | (20 << 8));
  pShmNode = p->pShmNode;
  if (pShmNode == 0)
    return (10 | (20 << 8));
  aLock = pShmNode->aLock;

  if (((flags & SQLITE_SHM_UNLOCK) && ((p->exclMask | p->sharedMask) & mask)) ||
      (flags == (SQLITE_SHM_SHARED | SQLITE_SHM_LOCK) && 0 == (p->sharedMask & mask)) ||
      (flags == (SQLITE_SHM_EXCLUSIVE | SQLITE_SHM_LOCK))) {
    sqlite3_mutex_enter(pShmNode->pShmMutex);

    if (rc == 0) {
      if (flags & SQLITE_SHM_UNLOCK) {
        int bUnlock = 1;

        if (flags & SQLITE_SHM_SHARED) {
          if (aLock[ofst] > 1) {
            bUnlock = 0;
            aLock[ofst]--;
            p->sharedMask &= ~mask;
          }
        }

        if (bUnlock) {
          rc = unixShmSystemLock(pDbFd, 2, ofst + ((22 + 8) * 4), n);
          if (rc == SQLITE_OK) {
            memset(&aLock[ofst], 0, sizeof(int) * n);
            p->sharedMask &= ~mask;
            p->exclMask &= ~mask;
          }
        }
      } else if (flags & SQLITE_SHM_SHARED) {
        if (aLock[ofst] < 0) {
          rc = SQLITE_BUSY;
        } else if (aLock[ofst] == 0) {
          rc = unixShmSystemLock(pDbFd, 0, ofst + ((22 + 8) * 4), n);
        }

        if (rc == SQLITE_OK) {
          p->sharedMask |= mask;
          aLock[ofst]++;
        }
      } else {
        int ii;

        for (ii = ofst; ii < ofst + n; ii++) {
          if (aLock[ii]) {
            rc = SQLITE_BUSY;
            break;
          }
        }

        if (rc == SQLITE_OK) {
          rc = unixShmSystemLock(pDbFd, 1, ofst + ((22 + 8) * 4), n);
          if (rc == SQLITE_OK) {
            p->exclMask |= mask;
            for (ii = ofst; ii < ofst + n; ii++) {
              aLock[ii] = -1;
            }
          }
        }
      }
    }

    sqlite3_mutex_leave(pShmNode->pShmMutex);
  }

  return rc;
}

void unixShmBarrier(sqlite3_file *fd) {
  (void)(fd);
  sqlite3MemoryBarrier();

  unixEnterMutex();
  unixLeaveMutex();
}

int unixShmUnmap(sqlite3_file *fd, int deleteFlag) {
  unixShm *p;
  unixShmNode *pShmNode;
  unixShm **pp;
  unixFile *pDbFd;

  pDbFd = (unixFile *)fd;
  p = pDbFd->pShm;
  if (p == 0)
    return SQLITE_OK;
  pShmNode = p->pShmNode;

  sqlite3_mutex_enter(pShmNode->pShmMutex);
  for (pp = &pShmNode->pFirst; (*pp) != p; pp = &(*pp)->pNext) {
  }
  *pp = p->pNext;

  sqlite3_free(p);
  pDbFd->pShm = 0;
  sqlite3_mutex_leave(pShmNode->pShmMutex);

  unixEnterMutex();

  pShmNode->nRef--;
  if (pShmNode->nRef == 0) {
    if (deleteFlag && pShmNode->hShm >= 0) {
      ((int (*)(const char *))aSyscall[SQLITE_SYSCALL_UNLINK].pCurrent)(pShmNode->zFilename);
    }
    unixShmPurge(pDbFd);
  }
  unixLeaveMutex();

  return SQLITE_OK;
}

int unixFetch(sqlite3_file *fd, i64 iOff, int nAmt, void **pp) {
  unixFile *pFd = (unixFile *)fd;

  *pp = 0;

  if (pFd->mmapSizeMax > 0) {
    const int nEofBuffer = 256;
    if (pFd->pMapRegion == 0) {
      int rc = unixMapfile(pFd, -1);
      if (rc != SQLITE_OK)
        return rc;
    }
    if (pFd->mmapSize >= (iOff + nAmt + nEofBuffer)) {
      *pp = &((u8 *)pFd->pMapRegion)[iOff];
      pFd->nFetchOut++;
    }
  }

  return SQLITE_OK;
}

int unixUnfetch(sqlite3_file *fd, i64 iOff, void *p) {
  unixFile *pFd = (unixFile *)fd;
  (void)(iOff);

  if (p) {
    pFd->nFetchOut--;
  } else {
    unixUnmapfile(pFd);
  }

  return 0;
}

int memdbClose(sqlite3_file *pFile) {
  MemStore *p = ((MemFile *)pFile)->pStore;
  if (p->zFName) {
    int i;

    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);

    sqlite3_mutex_enter(pVfsMutex);
    for (i = 0; (i < memdb_g.nMemStore); i++) {
      if (memdb_g.apMemStore[i] == p) {
        memdbEnter(p);
        if (p->nRef == 1) {
          memdb_g.apMemStore[i] = memdb_g.apMemStore[--memdb_g.nMemStore];
          if (memdb_g.nMemStore == 0) {
            sqlite3_free(memdb_g.apMemStore);
            memdb_g.apMemStore = 0;
          }
        }
        break;
      }
    }
    sqlite3_mutex_leave(pVfsMutex);
  } else {
    memdbEnter(p);
  }
  p->nRef--;
  if (p->nRef <= 0) {
    if (p->mFlags & SQLITE_DESERIALIZE_FREEONCLOSE) {
      sqlite3_free(p->aData);
    }
    memdbLeave(p);
    sqlite3_mutex_free(p->pMutex);
    sqlite3_free(p);
  } else {
    memdbLeave(p);
  }
  return SQLITE_OK;
}

int memdbRead(sqlite3_file *pFile, void *zBuf, int iAmt, sqlite_int64 iOfst) {
  MemStore *p = ((MemFile *)pFile)->pStore;
  memdbEnter(p);
  if (iOfst + iAmt > p->sz) {
    memset(zBuf, 0, iAmt);
    if (iOfst < p->sz)
      memcpy(zBuf, p->aData + iOfst, p->sz - iOfst);
    memdbLeave(p);
    return (10 | (2 << 8));
  }
  memcpy(zBuf, p->aData + iOfst, iAmt);
  memdbLeave(p);
  return SQLITE_OK;
}

int memdbWrite(sqlite3_file *pFile, const void *z, int iAmt, sqlite_int64 iOfst) {
  MemStore *p = ((MemFile *)pFile)->pStore;
  memdbEnter(p);
  if ((p->mFlags & SQLITE_DESERIALIZE_READONLY)) {
    memdbLeave(p);
    return (10 | (3 << 8));
  }
  if (iOfst + iAmt > p->sz) {
    int rc;
    if (iOfst + iAmt > p->szAlloc && (rc = memdbEnlarge(p, iOfst + iAmt)) != SQLITE_OK) {
      memdbLeave(p);
      return rc;
    }
    if (iOfst > p->sz)
      memset(p->aData + p->sz, 0, iOfst - p->sz);
    p->sz = iOfst + iAmt;
  }
  memcpy(p->aData + iOfst, z, iAmt);
  memdbLeave(p);
  return SQLITE_OK;
}

int memdbTruncate(sqlite3_file *pFile, sqlite_int64 size) {
  MemStore *p = ((MemFile *)pFile)->pStore;
  int rc = SQLITE_OK;
  memdbEnter(p);
  if (size > p->sz) {
    rc = SQLITE_CORRUPT;
  } else {
    p->sz = size;
  }
  memdbLeave(p);
  return rc;
}

int memdbSync(sqlite3_file *pFile, int flags) {
  (void)(pFile);
  (void)(flags);
  return SQLITE_OK;
}

int memdbFileSize(sqlite3_file *pFile, sqlite_int64 *pSize) {
  MemStore *p = ((MemFile *)pFile)->pStore;
  memdbEnter(p);
  *pSize = p->sz;
  memdbLeave(p);
  return SQLITE_OK;
}

int memdbLock(sqlite3_file *pFile, int eLock) {
  MemFile *pThis = (MemFile *)pFile;
  MemStore *p = pThis->pStore;
  int rc = SQLITE_OK;
  if (eLock <= pThis->eLock)
    return SQLITE_OK;
  memdbEnter(p);

  if (eLock > SQLITE_LOCK_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY)) {
    rc = SQLITE_READONLY;
  } else {
    switch (eLock) {
      case SQLITE_LOCK_SHARED: {
        if (p->nWrLock > 0) {
          rc = SQLITE_BUSY;
        } else {
          p->nRdLock++;
        }
        break;
      };

      case SQLITE_LOCK_RESERVED:
      case SQLITE_LOCK_PENDING: {
        if (pThis->eLock == 1) {
          if (p->nWrLock > 0) {
            rc = SQLITE_BUSY;
          } else {
            p->nWrLock = 1;
          }
        }
        break;
      }

      default: {
        if (p->nRdLock > 1) {
          rc = SQLITE_BUSY;
        } else if (pThis->eLock == SQLITE_LOCK_SHARED) {
          p->nWrLock = 1;
        }
        break;
      }
    }
  }
  if (rc == SQLITE_OK)
    pThis->eLock = eLock;
  memdbLeave(p);
  return rc;
}

int memdbUnlock(sqlite3_file *pFile, int eLock) {
  MemFile *pThis = (MemFile *)pFile;
  MemStore *p = pThis->pStore;
  if (eLock >= pThis->eLock)
    return SQLITE_OK;
  memdbEnter(p);

  if (eLock == SQLITE_LOCK_SHARED) {
    if ((pThis->eLock > SQLITE_LOCK_SHARED)) {
      p->nWrLock--;
    }
  } else {
    if (pThis->eLock > SQLITE_LOCK_SHARED) {
      p->nWrLock--;
    }
    p->nRdLock--;
  }

  pThis->eLock = eLock;
  memdbLeave(p);
  return SQLITE_OK;
}

int memdbFileControl(sqlite3_file *pFile, int op, void *pArg) {
  MemStore *p = ((MemFile *)pFile)->pStore;
  int rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  if (op == SQLITE_FCNTL_VFSNAME) {
    *(char **)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->aData, p->sz);
    rc = SQLITE_OK;
  }
  if (op == SQLITE_FCNTL_SIZE_LIMIT) {
    sqlite3_int64 iLimit = *(sqlite3_int64 *)pArg;
    if (iLimit < p->sz) {
      if (iLimit < 0) {
        iLimit = p->szMax;
      } else {
        iLimit = p->sz;
      }
    }
    p->szMax = iLimit;
    *(sqlite3_int64 *)pArg = iLimit;
    rc = SQLITE_OK;
  }
  memdbLeave(p);
  return rc;
}

int memdbDeviceCharacteristics(sqlite3_file *pFile) {
  (void)(pFile);
  return SQLITE_IOCAP_ATOMIC | SQLITE_IOCAP_POWERSAFE_OVERWRITE | SQLITE_IOCAP_SAFE_APPEND | SQLITE_IOCAP_SEQUENTIAL;
}

int memdbFetch(sqlite3_file *pFile, sqlite3_int64 iOfst, int iAmt, void **pp) {
  MemStore *p = ((MemFile *)pFile)->pStore;
  memdbEnter(p);
  if (iOfst + iAmt > p->sz || (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE) != 0) {
    *pp = 0;
  } else {
    p->nMmap++;
    *pp = (void *)(p->aData + iOfst);
  }
  memdbLeave(p);
  return SQLITE_OK;
}

int memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, void *pPage) {
  MemStore *p = ((MemFile *)pFile)->pStore;
  (void)(iOfst);
  (void)(pPage);
  memdbEnter(p);
  p->nMmap--;
  memdbLeave(p);
  return SQLITE_OK;
}

int read32bits(sqlite3_file *fd, i64 offset, u32 *pRes) {
  unsigned char ac[4];
  int rc = sqlite3OsRead(fd, ac, sizeof(ac), offset);
  if (rc == SQLITE_OK) {
    *pRes = sqlite3Get4byte(ac);
  }
  return rc;
}

int write32bits(sqlite3_file *fd, i64 offset, u32 val) {
  char ac[4];
  sqlite3Put4byte((u8 *)ac, val);
  return sqlite3OsWrite(fd, ac, 4, offset);
}

int readSuperJournal(sqlite3_file *pJrnl, u64 nSuper, char **pzSuper) {
  int rc;
  u32 len;
  i64 szJ;
  u32 cksum;
  unsigned char aMagic[8];
  char *zOut = 0;

  *pzSuper = 0;
  if (SQLITE_OK != (rc = sqlite3OsFileSize(pJrnl, &szJ)) || szJ < 16 ||
      SQLITE_OK != (rc = read32bits(pJrnl, szJ - 16, &len)) || len >= nSuper || len > szJ - 16 || len == 0 ||
      SQLITE_OK != (rc = read32bits(pJrnl, szJ - 12, &cksum)) ||
      SQLITE_OK != (rc = sqlite3OsRead(pJrnl, aMagic, 8, szJ - 8)) || memcmp(aMagic, aJournalMagic, 8)) {
    return rc;
  }

  zOut = (char *)sqlite3MallocZero(4 + len + 2);
  if (!zOut) {
    rc = 7;
  } else {
    zOut = &zOut[4];
    if (SQLITE_OK == (rc = sqlite3OsRead(pJrnl, zOut, len, szJ - 16 - len))) {
      u32 u;

      for (u = 0; u < len; u++) {
        cksum -= zOut[u];
      }
    }
    if (rc != SQLITE_OK || cksum || zOut[0] == 0) {
      freeSuperJournal(zOut);
      zOut = 0;
    }
  }

  *pzSuper = zOut;
  return rc;
}

int sqlite3SectorSize(sqlite3_file *pFile) {
  int iRet = sqlite3OsSectorSize(pFile);
  if (iRet < 32) {
    iRet = 512;
  } else if (iRet > 0x10000) {
    iRet = 0x10000;
  }
  return iRet;
}

int backupTruncateFile(sqlite3_file *pFile, i64 iSize) {
  i64 iCurrent;
  int rc = sqlite3OsFileSize(pFile, &iCurrent);
  if (rc == SQLITE_OK && iCurrent > iSize) {
    rc = sqlite3OsTruncate(pFile, iSize);
  }
  return rc;
}

void vdbePmaWriterInit(sqlite3_file *pFd, PmaWriter *p, int nBuf, i64 iStart) {
  memset(p, 0, sizeof(PmaWriter));
  p->aBuffer = (u8 *)sqlite3Malloc(nBuf);
  if (!p->aBuffer) {
    p->eFWErr = 7;
  } else {
    p->iBufEnd = p->iBufStart = (iStart % nBuf);
    p->iWriteOff = iStart - p->iBufStart;
    p->nBuffer = nBuf;
    p->pFd = pFd;
  }
}

int memjrnlRead(sqlite3_file *pJfd, void *zBuf, int iAmt, sqlite_int64 iOfst) {
  MemJournal *p = (MemJournal *)pJfd;
  u8 *zOut = zBuf;
  int nRead = iAmt;
  int iChunkOffset;
  FileChunk *pChunk;

  if ((iAmt + iOfst) > p->endpoint.iOffset) {
    return (10 | (2 << 8));
  }

  if (p->readpoint.iOffset != iOfst || iOfst == 0) {
    sqlite3_int64 iOff = 0;
    for (pChunk = p->pFirst; (pChunk) && (iOff + p->nChunkSize) <= iOfst; pChunk = pChunk->pNext) {
      iOff += p->nChunkSize;
    }
  } else {
    pChunk = p->readpoint.pChunk;
  }

  iChunkOffset = (int)(iOfst % p->nChunkSize);
  do {
    int iSpace = p->nChunkSize - iChunkOffset;
    int nCopy = ((nRead) < ((p->nChunkSize - iChunkOffset)) ? (nRead) : ((p->nChunkSize - iChunkOffset)));
    memcpy(zOut, (u8 *)pChunk->zChunk + iChunkOffset, nCopy);
    zOut += nCopy;
    nRead -= iSpace;
    iChunkOffset = 0;
  } while (nRead >= 0 && (pChunk = pChunk->pNext) != 0 && nRead > 0);
  p->readpoint.iOffset = pChunk ? iOfst + iAmt : 0;
  p->readpoint.pChunk = pChunk;

  return SQLITE_OK;
}

int memjrnlWrite(sqlite3_file *pJfd, const void *zBuf, int iAmt, sqlite_int64 iOfst) {
  MemJournal *p = (MemJournal *)pJfd;
  int nWrite = iAmt;
  u8 *zWrite = (u8 *)zBuf;

  if (p->nSpill > 0 && (iAmt + iOfst) > p->nSpill) {
    int rc = memjrnlCreateFile(p);
    if (rc == SQLITE_OK) {
      rc = sqlite3OsWrite(pJfd, zBuf, iAmt, iOfst);
    }
    return rc;
  }

  else {
    if (iOfst > 0 && iOfst != p->endpoint.iOffset) {
      memjrnlTruncate(pJfd, iOfst);
    }
    if (iOfst == 0 && p->pFirst) {
      memcpy((u8 *)p->pFirst->zChunk, zBuf, iAmt);
    } else {
      while (nWrite > 0) {
        FileChunk *pChunk = p->endpoint.pChunk;
        int iChunkOffset = (int)(p->endpoint.iOffset % p->nChunkSize);
        int iSpace = ((nWrite) < (p->nChunkSize - iChunkOffset) ? (nWrite) : (p->nChunkSize - iChunkOffset));

        if (iChunkOffset == 0) {
          FileChunk *pNew = sqlite3_malloc((sizeof(FileChunk) + ((p->nChunkSize) - 8)));
          if (!pNew) {
            return (10 | (12 << 8));
          }
          pNew->pNext = 0;
          if (pChunk) {
            pChunk->pNext = pNew;
          } else {
            p->pFirst = pNew;
          }
          pChunk = p->endpoint.pChunk = pNew;
        }

        memcpy((u8 *)pChunk->zChunk + iChunkOffset, zWrite, iSpace);
        zWrite += iSpace;
        nWrite -= iSpace;
        p->endpoint.iOffset += iSpace;
      }
    }
  }

  return SQLITE_OK;
}

int memjrnlTruncate(sqlite3_file *pJfd, sqlite_int64 size) {
  MemJournal *p = (MemJournal *)pJfd;

  if (size < p->endpoint.iOffset) {
    FileChunk *pIter = 0;
    if (size == 0) {
      memjrnlFreeChunks(p->pFirst);
      p->pFirst = 0;
    } else {
      i64 iOff = p->nChunkSize;
      for (pIter = p->pFirst; (pIter) && iOff < size; pIter = pIter->pNext) {
        iOff += p->nChunkSize;
      }
      if ((pIter)) {
        memjrnlFreeChunks(pIter->pNext);
        pIter->pNext = 0;
      }
    }

    p->endpoint.pChunk = pIter;
    p->endpoint.iOffset = size;
    p->readpoint.pChunk = 0;
    p->readpoint.iOffset = 0;
  }
  return SQLITE_OK;
}

int memjrnlClose(sqlite3_file *pJfd) {
  MemJournal *p = (MemJournal *)pJfd;
  memjrnlFreeChunks(p->pFirst);
  return SQLITE_OK;
}

int memjrnlSync(sqlite3_file *pJfd, int flags) {
  (void)(pJfd), (void)(flags);
  return SQLITE_OK;
}

int memjrnlFileSize(sqlite3_file *pJfd, sqlite_int64 *pSize) {
  MemJournal *p = (MemJournal *)pJfd;
  *pSize = (sqlite_int64)p->endpoint.iOffset;
  return SQLITE_OK;
}

void sqlite3MemJournalOpen(sqlite3_file *pJfd) {
  sqlite3JournalOpen(0, 0, pJfd, 0, -1);
}

int sqlite3JournalIsInMemory(sqlite3_file *p) {
  return p->pMethods == &MemJournalMethods;
}