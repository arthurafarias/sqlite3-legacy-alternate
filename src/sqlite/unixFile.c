#define _GNU_SOURCE 1

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "sqlite/unixFile.h"

#include "sqlite/SqliteUnixSyscallIndex.h"
#include "sqlite/Sqlite3Config.h"
#include "sqlite/UnixUnusedFd.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_filename.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/sqlite3_syscall_ptr.h"
#include "sqlite/sqlite3_uint64.h"
#include "sqlite/u16.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/unixFileId.h"
#include "sqlite/unixInodeInfo.h"
#include "sqlite/unixShm.h"
#include "sqlite/unixShmNode.h"
#include "sqlite/unix_syscall.h"
void robust_close(unixFile *pFile, int h, int lineno) {
  if (((int (*)(int))aSyscall[SQLITE_SYSCALL_CLOSE].pCurrent)(h)) {
    unixLogErrorAtLine((10 | (16 << 8)), "close", pFile ? pFile->zPath : 0, lineno);
  }
}

void storeLastErrno(unixFile *pFile, int error) { pFile->lastErrno = error; }

void closePendingFds(unixFile *pFile) {
  unixInodeInfo *pInode = pFile->pInode;
  UnixUnusedFd *p;
  UnixUnusedFd *pNext;

  for (p = pInode->pUnused; p; p = pNext) {
    pNext = p->pNext;
    robust_close(pFile, p->fd, 41675);
    sqlite3_free(p);
  }
  pInode->pUnused = 0;
}

void releaseInodeInfo(unixFile *pFile) {
  unixInodeInfo *pInode = pFile->pInode;

  if ((pInode)) {
    pInode->nRef--;
    if (pInode->nRef == 0) {


      sqlite3_mutex_enter(pInode->pLockMutex);
      closePendingFds(pFile);
      sqlite3_mutex_leave(pInode->pLockMutex);
      if (pInode->pPrev) {


        pInode->pPrev->pNext = pInode->pNext;
      } else {


        inodeList = pInode->pNext;
      }
      if (pInode->pNext) {


        pInode->pNext->pPrev = pInode->pPrev;
      }
      sqlite3_mutex_free(pInode->pLockMutex);
      sqlite3_free(pInode);
    }
  }
}

int findInodeInfo(unixFile *pFile, unixInodeInfo **ppInode) {
  int rc;
  int fd;
  struct unixFileId fileId;
  struct stat statbuf;
  unixInodeInfo *pInode = 0;

  fd = pFile->h;
  rc = ((int (*)(int, struct stat *))aSyscall[SQLITE_SYSCALL_FSTAT].pCurrent)(fd, &statbuf);
  if (rc != 0) {
    storeLastErrno(pFile,

                   (*__errno_location())

    );

    return 10;
  }

  memset(&fileId, 0, sizeof(fileId));
  fileId.dev = statbuf.st_dev;

  fileId.ino = (u64)statbuf.st_ino;

  pInode = inodeList;
  while (pInode && memcmp(&fileId, &pInode->fileId, sizeof(fileId))) {
    pInode = pInode->pNext;
  }
  if (pInode == 0) {
    pInode = sqlite3_malloc64(sizeof(*pInode));
    if (pInode == 0) {
      return 7;
    }
    memset(pInode, 0, sizeof(*pInode));
    memcpy(&pInode->fileId, &fileId, sizeof(fileId));
    if (sqlite3Config.bCoreMutex) {
      pInode->pLockMutex = sqlite3_mutex_alloc(0);
      if (pInode->pLockMutex == 0) {
        sqlite3_free(pInode);
        return 7;
      }
    }
    pInode->nRef = 1;


    pInode->pNext = inodeList;
    pInode->pPrev = 0;
    if (inodeList)
      inodeList->pPrev = pInode;
    inodeList = pInode;
  } else {
    pInode->nRef++;
  }
  *ppInode = pInode;
  return 0;
}

int fileHasMoved(unixFile *pFile) {

  struct stat buf;
  return pFile->pInode != 0 && (((int (*)(const char *, struct stat *))aSyscall[SQLITE_SYSCALL_STAT].pCurrent)(pFile->zPath, &buf) != 0 || (u64)buf.st_ino != pFile->pInode->fileId.ino);
}

void verifyDbFile(unixFile *pFile) {
  struct stat buf;
  int rc;

  if (pFile->ctrlFlags & 0x80)
    return;

  rc = ((int (*)(int, struct stat *))aSyscall[SQLITE_SYSCALL_FSTAT].pCurrent)(pFile->h, &buf);
  if (rc != 0) {
    sqlite3_log(28, "cannot fstat db file %s", pFile->zPath);
    return;
  }
  if (buf.st_nlink == 0) {
    sqlite3_log(28, "file unlinked while open: %s", pFile->zPath);
    return;
  }
  if (buf.st_nlink > 1) {
    sqlite3_log(28, "multiple links to file: %s", pFile->zPath);
    return;
  }
  if (fileHasMoved(pFile)) {
    sqlite3_log(28, "file renamed while open: %s", pFile->zPath);
    return;
  }
}

int unixFileLock(unixFile *pFile, struct flock *pLock) {
  int rc;
  unixInodeInfo *pInode = pFile->pInode;

  if ((pFile->ctrlFlags & (0x01 | 0x02)) == 0x01) {
    if (pInode->bProcessLock == 0) {
      struct flock lock;

      lock.l_whence =

          0

          ;
      lock.l_start = (sqlite3PendingByte + 2);
      lock.l_len = 510;
      lock.l_type =

          1

          ;
      rc = ((int (*)(int, int, ...))aSyscall[SQLITE_SYSCALL_FCNTL].pCurrent)(pFile->h,

                                                          6

                                                          ,
                                                          &lock);
      if (rc < 0)
        return rc;
      pInode->bProcessLock = 1;
      pInode->nLock++;
    } else {
      rc = 0;
    }
  } else {

    rc = ((int (*)(int, int, ...))aSyscall[SQLITE_SYSCALL_FCNTL].pCurrent)(pFile->h,

                                                        6

                                                        ,
                                                        pLock);
  }
  return rc;
}

void setPendingFd(unixFile *pFile) {
  unixInodeInfo *pInode = pFile->pInode;
  UnixUnusedFd *p = pFile->pPreallocatedUnused;

  p->pNext = pInode->pUnused;
  pInode->pUnused = p;
  pFile->h = -1;
  pFile->pPreallocatedUnused = 0;
}

int seekAndRead(unixFile *id, sqlite3_int64 offset, void *pBuf, int cnt) {
  int got;
  int prior = 0;

  ;

  do {

    got = ((ssize_t (*)(int, void *, size_t, off64_t))aSyscall[SQLITE_SYSCALL_PREAD64].pCurrent)(id->h, pBuf, cnt, offset);
    ;

    if (got == cnt)
      break;
    if (got < 0) {
      if (

          (*__errno_location())

          ==

          4

      ) {
        got = 1;
        continue;
      }
      prior = 0;
      storeLastErrno((unixFile *)id,

                     (*__errno_location())

      );
      break;
    } else if (got > 0) {
      cnt -= got;
      offset += got;
      prior += got;
      pBuf = (void *)(got + (char *)pBuf);
    }
  } while (got > 0);
  ;

  ;
  return got + prior;
}

int seekAndWrite(unixFile *id, i64 offset, const void *pBuf, int cnt) { return seekAndWriteFd(id->h, offset, pBuf, cnt, &id->lastErrno); }

int fcntlSizeHint(unixFile *pFile, i64 nByte) {
  if (pFile->szChunk > 0) {
    i64 nSize;
    struct stat buf;

    if (((int (*)(int, struct stat *))aSyscall[SQLITE_SYSCALL_FSTAT].pCurrent)(pFile->h, &buf)) {
      return (10 | (7 << 8));
    }

    nSize = ((nByte + pFile->szChunk - 1) / pFile->szChunk) * pFile->szChunk;
    if (nSize > (i64)buf.st_size) {

      int nBlk = buf.st_blksize;
      int nWrite = 0;
      i64 iWrite;

      iWrite = (buf.st_size / nBlk) * nBlk + nBlk - 1;




      for (; iWrite < nSize + nBlk - 1; iWrite += nBlk) {
        if (iWrite >= nSize)
          iWrite = nSize - 1;
        nWrite = seekAndWrite(pFile, iWrite, "", 1);
        if (nWrite != 1)
          return (10 | (3 << 8));
      }
    }
  }

  if (pFile->mmapSizeMax > 0 && nByte > pFile->mmapSize) {
    int rc;
    if (pFile->szChunk <= 0) {
      if (robust_ftruncate(pFile->h, nByte)) {
        storeLastErrno(pFile,

                       (*__errno_location())

        );
        return unixLogErrorAtLine((10 | (6 << 8)), "ftruncate", pFile->zPath, 44297);
      }
    }

    rc = unixMapfile(pFile, nByte);
    return rc;
  }

  return 0;
}

void unixModeBit(unixFile *pFile, unsigned char mask, int *pArg) {
  if (*pArg < 0) {
    *pArg = (pFile->ctrlFlags & mask) != 0;
  } else if ((*pArg) == 0) {
    pFile->ctrlFlags &= ~mask;
  } else {
    pFile->ctrlFlags |= mask;
  }
}

void setDeviceCharacteristics(unixFile *pFd) {

  if (pFd->sectorSize == 0) {

    if (pFd->ctrlFlags & 0x10) {
      pFd->deviceCharacteristics |= 0x00001000;
    }
    pFd->deviceCharacteristics |= 0x00008000;

    pFd->sectorSize = 4096;
  }
}

int unixFcntlExternalReader(unixFile *pFile, int *piOut) {
  int rc = 0;
  *piOut = 0;
  if (pFile->pShm) {
    unixShmNode *pShmNode = pFile->pShm->pShmNode;
    struct flock f;

    memset(&f, 0, sizeof(f));
    f.l_type =

        1

        ;
    f.l_whence =

        0

        ;
    f.l_start = ((22 + 8) * 4) + 3;
    f.l_len = 8 - 3;

    sqlite3_mutex_enter(pShmNode->pShmMutex);
    if (((int (*)(int, int, ...))aSyscall[SQLITE_SYSCALL_FCNTL].pCurrent)(pShmNode->hShm,

                                                       5

                                                       ,
                                                       &f) < 0) {
      rc = (10 | (15 << 8));
    } else {
      *piOut = (f.l_type !=

                2

      );
    }
    sqlite3_mutex_leave(pShmNode->pShmMutex);
  }

  return rc;
}

int unixIsSharingShmNode(unixFile *pFile) {
  unixShmNode *pShmNode;
  struct flock lock;
  if (pFile->pShm == 0)
    return 0;
  if (pFile->ctrlFlags & 0x01)
    return 0;
  pShmNode = pFile->pShm->pShmNode;

  memset(&lock, 0, sizeof(lock));
  lock.l_whence =

      0

      ;
  lock.l_start = (((22 + 8) * 4) + 8);
  lock.l_len = 1;
  lock.l_type =

      1

      ;
  ((int (*)(int, int, ...))aSyscall[SQLITE_SYSCALL_FCNTL].pCurrent)(pShmNode->hShm,

                                                 5

                                                 ,
                                                 &lock);
  return (lock.l_type !=

          2

  );
}

int unixShmSystemLock(unixFile *pFile, int lockType, int ofst, int n) {
  unixShmNode *pShmNode;
  struct flock f;
  int rc = 0;

  pShmNode = pFile->pInode->pShmNode;

  if (ofst == (((22 + 8) * 4) + 8)) {




  } else {




  }

  if (pShmNode->hShm >= 0) {
    int res;

    f.l_type = lockType;
    f.l_whence =

        0

        ;
    f.l_start = ofst;
    f.l_len = n;
    res = ((int (*)(int, int, ...))aSyscall[SQLITE_SYSCALL_FCNTL].pCurrent)(pShmNode->hShm,

                                                         6

                                                         ,
                                                         &f);
    if (res == -1) {

      rc = 5;
    }
  }

  return rc;
}

void unixShmPurge(unixFile *pFd) {
  unixShmNode *p = pFd->pInode->pShmNode;

  if (p && (p->nRef == 0)) {
    int nShmPerMap = unixShmRegionPerMap();
    int i;


    sqlite3_mutex_free(p->pShmMutex);

    for (i = 0; i < p->nRegion; i += nShmPerMap) {
      if (p->hShm >= 0) {
        ((int (*)(void *, size_t))aSyscall[SQLITE_SYSCALL_MUNMAP].pCurrent)(p->apRegion[i], p->szRegion);
      } else {
        sqlite3_free(p->apRegion[i]);
      }
    }
    sqlite3_free(p->apRegion);
    if (p->hShm >= 0) {
      robust_close(pFd, p->hShm, 45030);
      p->hShm = -1;
    }
    p->pInode->pShmNode = 0;
    sqlite3_free(p);
  }
}

int unixLockSharedMemory(unixFile *pDbFd, unixShmNode *pShmNode) {
  struct flock lock;
  int rc = 0;

  lock.l_whence =

      0

      ;
  lock.l_start = (((22 + 8) * 4) + 8);
  lock.l_len = 1;
  lock.l_type =

      1

      ;
  if (((int (*)(int, int, ...))aSyscall[SQLITE_SYSCALL_FCNTL].pCurrent)(pShmNode->hShm,

                                                     5

                                                     ,
                                                     &lock) != 0) {
    rc = (10 | (15 << 8));
  } else if (lock.l_type ==

             2

  ) {
    if (pShmNode->isReadonly) {
      pShmNode->isUnlocked = 1;
      rc = (8 | (5 << 8));
    } else {

      rc = unixShmSystemLock(pDbFd,

                             1

                             ,
                             (((22 + 8) * 4) + 8), 1);

      if (rc == 0 && robust_ftruncate(pShmNode->hShm, 3)) {
        rc = unixLogErrorAtLine((10 | (18 << 8)), "ftruncate", pShmNode->zFilename, 45100);
      }
    }
  } else if (lock.l_type ==

             1

  ) {
    rc = 5;
  }

  if (rc == 0) {


    rc = unixShmSystemLock(pDbFd,

                           0

                           ,
                           (((22 + 8) * 4) + 8), 1);
  }
  return rc;
}

int unixOpenSharedMemory(unixFile *pDbFd) {
  struct unixShm *p = 0;
  struct unixShmNode *pShmNode;
  int rc = 0;
  unixInodeInfo *pInode;
  char *zShm;
  int nShmFilename;

  p = sqlite3_malloc64(sizeof(*p));
  if (p == 0)
    return 7;
  memset(p, 0, sizeof(*p));

  unixEnterMutex();
  pInode = pDbFd->pInode;
  pShmNode = pInode->pShmNode;
  if (pShmNode == 0) {
    struct stat sStat;

    const char *zBasePath = pDbFd->zPath;

    if (((int (*)(int, struct stat *))aSyscall[SQLITE_SYSCALL_FSTAT].pCurrent)(pDbFd->h, &sStat)) {
      rc = (10 | (7 << 8));
      goto shm_open_err;
    }

    nShmFilename = 6 + (int)strlen(zBasePath);

    pShmNode = sqlite3_malloc64(sizeof(*pShmNode) + nShmFilename);
    if (pShmNode == 0) {
      rc = 7;
      goto shm_open_err;
    }
    memset(pShmNode, 0, sizeof(*pShmNode) + nShmFilename);
    zShm = pShmNode->zFilename = (char *)&pShmNode[1];

    sqlite3_snprintf(nShmFilename, zShm, "%s-shm", zBasePath);
    ;

    pShmNode->hShm = -1;
    pDbFd->pInode->pShmNode = pShmNode;
    pShmNode->pInode = pDbFd->pInode;
    if (sqlite3Config.bCoreMutex) {
      pShmNode->pShmMutex = sqlite3_mutex_alloc(0);
      if (pShmNode->pShmMutex == 0) {
        rc = 7;
        goto shm_open_err;
      }
    }

    if (pInode->bProcessLock == 0) {
      if (0 == sqlite3_uri_boolean(pDbFd->zPath, "readonly_shm", 0)) {
        pShmNode->hShm = robust_open(zShm,

                                     02

                                         |

                                         0100

                                         |

                                         0400000

                                     ,
                                     (sStat.st_mode & 0777));
      }
      if (pShmNode->hShm < 0) {
        pShmNode->hShm = robust_open(zShm,

                                     00

                                         |

                                         0400000

                                     ,
                                     (sStat.st_mode & 0777));
        if (pShmNode->hShm < 0) {
          rc = unixLogErrorAtLine(sqlite3CantopenError(45237), "open", zShm, 45237);
          goto shm_open_err;
        }
        pShmNode->isReadonly = 1;
      }

      robustFchown(pShmNode->hShm, sStat.st_uid, sStat.st_gid);

      rc = unixLockSharedMemory(pDbFd, pShmNode);
      if (rc != 0 && rc != (8 | (5 << 8)))
        goto shm_open_err;
    }
  }

  p->pShmNode = pShmNode;

  pShmNode->nRef++;
  pDbFd->pShm = p;
  unixLeaveMutex();

  sqlite3_mutex_enter(pShmNode->pShmMutex);
  p->pNext = pShmNode->pFirst;
  pShmNode->pFirst = p;
  sqlite3_mutex_leave(pShmNode->pShmMutex);
  return rc;

shm_open_err:
  unixShmPurge(pDbFd);
  sqlite3_free(p);
  unixLeaveMutex();
  return rc;
}

void unixUnmapfile(unixFile *pFd) {

  if (pFd->pMapRegion) {
    ((int (*)(void *, size_t))aSyscall[SQLITE_SYSCALL_MUNMAP].pCurrent)(pFd->pMapRegion, pFd->mmapSizeActual);
    pFd->pMapRegion = 0;
    pFd->mmapSize = 0;
    pFd->mmapSizeActual = 0;
  }
}

void unixRemapfile(unixFile *pFd, i64 nNew) {
  const char *zErr = "mmap";
  int h = pFd->h;
  u8 *pOrig = (u8 *)pFd->pMapRegion;
  i64 nOrig = pFd->mmapSizeActual;
  u8 *pNew = 0;
  int flags =

      0x1

      ;

  if (pOrig) {

    i64 nReuse = pFd->mmapSize;

    u8 *pReq = &pOrig[nReuse];

    if (nReuse != nOrig) {
      ((int (*)(void *, size_t))aSyscall[SQLITE_SYSCALL_MUNMAP].pCurrent)(pReq, nOrig - nReuse);
    }

    pNew = ((void *(*)(void *, size_t, size_t, int, ...))aSyscall[SQLITE_SYSCALL_MREMAP].pCurrent)(pOrig, nReuse, nNew,

                                                                                1

    );
    zErr = "mremap";

    if (pNew ==

            ((void *)-1)

        || pNew == 0) {
      ((int (*)(void *, size_t))aSyscall[SQLITE_SYSCALL_MUNMAP].pCurrent)(pOrig, nReuse);
    }
  }

  if (pNew == 0) {
    pNew = ((void *(*)(void *, size_t, int, int, int, off_t))aSyscall[SQLITE_SYSCALL_MMAP].pCurrent)(0, nNew, flags,

                                                                                    0x01

                                                                                    ,
                                                                                    h, 0);
  }

  if (pNew ==

      ((void *)-1)

  ) {
    pNew = 0;
    nNew = 0;
    unixLogErrorAtLine(0, zErr, pFd->zPath, 45847);

    pFd->mmapSizeMax = 0;
  }
  pFd->pMapRegion = (void *)pNew;
  pFd->mmapSize = pFd->mmapSizeActual = nNew;
}

int unixMapfile(unixFile *pFd, i64 nMap) {

  if (pFd->nFetchOut > 0)
    return 0;

  if (nMap < 0) {
    struct stat statbuf;
    if (((int (*)(int, struct stat *))aSyscall[SQLITE_SYSCALL_FSTAT].pCurrent)(pFd->h, &statbuf)) {
      return (10 | (7 << 8));
    }
    nMap = statbuf.st_size;
  }
  if (nMap > pFd->mmapSizeMax) {
    nMap = pFd->mmapSizeMax;
  }

  if (nMap != pFd->mmapSize) {
    unixRemapfile(pFd, nMap);
  }

  return 0;
}
