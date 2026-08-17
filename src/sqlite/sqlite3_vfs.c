#define _GNU_SOURCE 1

#include <dlfcn.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "sqlite/sqlite3_vfs.h"

#include "sqlite/SqliteUnixSyscallIndex.h"
#include "sqlite/BtCursor.h"
#include "sqlite/BtLock.h"
#include "sqlite/BtShared.h"
#include "sqlite/Btree.h"
#include "sqlite/Db.h"
#include "sqlite/DbPage.h"
#include "sqlite/DbPath.h"
#include "sqlite/FileChunk.h"
#include "sqlite/MemFS.h"
#include "sqlite/MemFile.h"
#include "sqlite/MemJournal.h"
#include "sqlite/MemPage.h"
#include "sqlite/MemStore.h"
#include "sqlite/PCache.h"
#include "sqlite/Pager.h"
#include "sqlite/Pgno.h"
#include "sqlite/Sqlite3Config.h"
#include "sqlite/UnixUnusedFd.h"
#include "sqlite/Wal.h"
#include "sqlite/finder_type.h"
#include "sqlite/i16.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_file.h"
#include "sqlite/sqlite3_filename.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_io_methods.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/sqlite3_syscall_ptr.h"
#include "sqlite/sqlite3_uint64.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/unixFile.h"
#include "sqlite/unixFileId.h"
#include "sqlite/unixInodeInfo.h"
#include "sqlite/unix_syscall.h"
#include "sqlite/uptr.h"

/* Private helpers, formerly declared in _Uncategorized.h. */
static int findCreateFileMode(const char *zPath, int flags, mode_t *pMode, uid_t *pUid, gid_t *pGid);
static UnixUnusedFd *findReusableFd(const char *zPath, int flags);
static int getFileMode(const char *zFile, mode_t *pMode, uid_t *pUid, gid_t *pGid);

extern pid_t randomnessPid;
pid_t randomnessPid = 0;

static UnixUnusedFd *findReusableFd(const char *zPath, int flags) {
  UnixUnusedFd *pUnused = 0;

  struct stat sStat;

  unixEnterMutex();

  if (inodeList != 0 && 0 == ((int (*)(const char *, struct stat *))aSyscall[SQLITE_SYSCALL_STAT].pCurrent)(zPath, &sStat)) {
    unixInodeInfo *pInode;

    pInode = inodeList;
    while (pInode && (pInode->fileId.dev != sStat.st_dev || pInode->fileId.ino != (u64)sStat.st_ino)) {
      pInode = pInode->pNext;
    }
    if (pInode) {
      UnixUnusedFd **pp;


      sqlite3_mutex_enter(pInode->pLockMutex);
      flags &= (0x00000001 | 0x00000002);
      for (pp = &pInode->pUnused; *pp && (*pp)->flags != flags; pp = &((*pp)->pNext))
        ;
      pUnused = *pp;
      if (pUnused) {
        *pp = pUnused->pNext;
      }
      sqlite3_mutex_leave(pInode->pLockMutex);
    }
  }
  unixLeaveMutex();

  return pUnused;
}

static int getFileMode(const char *zFile, mode_t *pMode, uid_t *pUid, gid_t *pGid) {
  struct stat sStat;
  int rc = 0;
  if (0 == ((int (*)(const char *, struct stat *))aSyscall[SQLITE_SYSCALL_STAT].pCurrent)(zFile, &sStat)) {
    *pMode = sStat.st_mode & 0777;
    *pUid = sStat.st_uid;
    *pGid = sStat.st_gid;
  } else {
    rc = (10 | (7 << 8));
  }
  return rc;
}

static int findCreateFileMode(const char *zPath, int flags, mode_t *pMode, uid_t *pUid, gid_t *pGid) {
  int rc = 0;
  *pMode = 0;
  *pUid = 0;
  *pGid = 0;
  if (flags & (0x00080000 | 0x00000800)) {
    char zDb[512 + 1];
    int nDb;

    nDb = sqlite3Strlen30(zPath) - 1;
    while (nDb > 0 && zPath[nDb] != '.') {
      if (zPath[nDb] == '-') {
        memcpy(zDb, zPath, nDb);
        zDb[nDb] = '\0';
        rc = getFileMode(zDb, pMode, pUid, pGid);
        break;
      }
      nDb--;
    }
  } else if (flags & 0x00000008) {
    *pMode = 0600;
  } else if (flags & 0x00000040) {

    const char *z = sqlite3_uri_parameter(zPath, "modeof");
    if (z) {
      rc = getFileMode(z, pMode, pUid, pGid);
    }
  }
  return rc;
}


int sqlite3OsOpen(sqlite3_vfs *pVfs, const char *zPath, sqlite3_file *pFile, int flags, int *pFlagsOut) {
  int rc;
  ;

  rc = pVfs->xOpen(pVfs, zPath, pFile, flags & 0x1087f7f, pFlagsOut);

  return rc;
}

int sqlite3OsDelete(sqlite3_vfs *pVfs, const char *zPath, int dirSync) {
  ;

  return pVfs->xDelete != 0 ? pVfs->xDelete(pVfs, zPath, dirSync) : 0;
}

int sqlite3OsAccess(sqlite3_vfs *pVfs, const char *zPath, int flags, int *pResOut) {
  ;
  return pVfs->xAccess(pVfs, zPath, flags, pResOut);
}

int sqlite3OsFullPathname(sqlite3_vfs *pVfs, const char *zPath, int nPathOut, char *zPathOut) {
  ;
  zPathOut[0] = 0;
  return pVfs->xFullPathname(pVfs, zPath, nPathOut, zPathOut);
}

void *sqlite3OsDlOpen(sqlite3_vfs *pVfs, const char *zPath) { return pVfs->xDlOpen(pVfs, zPath); }

void sqlite3OsDlError(sqlite3_vfs *pVfs, int nByte, char *zBufOut) { pVfs->xDlError(pVfs, nByte, zBufOut); }

void (*sqlite3OsDlSym(sqlite3_vfs *pVfs, void *pHdle, const char *zSym))(void) { return pVfs->xDlSym(pVfs, pHdle, zSym); }

void sqlite3OsDlClose(sqlite3_vfs *pVfs, void *pHandle) { pVfs->xDlClose(pVfs, pHandle); }

int sqlite3OsRandomness(sqlite3_vfs *pVfs, int nByte, char *zBufOut) {
  if (sqlite3Config.iPrngSeed) {
    memset(zBufOut, 0, nByte);
    if ((nByte > (signed)sizeof(unsigned)))
      nByte = sizeof(unsigned int);
    memcpy(zBufOut, &sqlite3Config.iPrngSeed, nByte);
    return 0;
  } else {
    return pVfs->xRandomness(pVfs, nByte, zBufOut);
  }
}

int sqlite3OsSleep(sqlite3_vfs *pVfs, int nMicro) { return pVfs->xSleep(pVfs, nMicro); }

int sqlite3OsGetLastError(sqlite3_vfs *pVfs) { return pVfs->xGetLastError ? pVfs->xGetLastError(pVfs, 0, 0) : 0; }

int sqlite3OsCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *pTimeOut) {
  int rc;

  if (pVfs->iVersion >= 2 && pVfs->xCurrentTimeInt64) {
    rc = pVfs->xCurrentTimeInt64(pVfs, pTimeOut);
  } else {
    double r;
    rc = pVfs->xCurrentTime(pVfs, &r);
    *pTimeOut = sqlite3RealToI64(r * 86400000.0);
  }
  return rc;
}

int sqlite3OsOpenMalloc(sqlite3_vfs *pVfs, const char *zFile, sqlite3_file **ppFile, int flags, int *pOutFlags) {
  int rc;
  sqlite3_file *pFile;
  pFile = (sqlite3_file *)sqlite3MallocZero(pVfs->szOsFile);
  if (pFile) {
    rc = sqlite3OsOpen(pVfs, zFile, pFile, flags, pOutFlags);
    if (rc != 0) {
      sqlite3_free(pFile);
      *ppFile = 0;
    } else {
      *ppFile = pFile;
    }
  } else {
    *ppFile = 0;
    rc = 7;
  }

  return rc;
}

sqlite3_vfs *vfsList = 0;

sqlite3_vfs *sqlite3_vfs_find(const char *zVfs) {
  sqlite3_vfs *pVfs = 0;

  sqlite3_mutex *mutex;

  int rc = sqlite3_initialize();
  if (rc)
    return 0;

  mutex = sqlite3MutexAlloc(2);

  sqlite3_mutex_enter(mutex);
  for (pVfs = vfsList; pVfs; pVfs = pVfs->pNext) {
    if (zVfs == 0)
      break;
    if (strcmp(zVfs, pVfs->zName) == 0)
      break;
  }
  sqlite3_mutex_leave(mutex);
  return pVfs;
}

void vfsUnlink(sqlite3_vfs *pVfs) {

  if (pVfs == 0) {

  } else if (vfsList == pVfs) {
    vfsList = pVfs->pNext;
  } else if (vfsList) {
    sqlite3_vfs *p = vfsList;
    while (p->pNext && p->pNext != pVfs) {
      p = p->pNext;
    }
    if (p->pNext == pVfs) {
      p->pNext = pVfs->pNext;
    }
  }
}

int sqlite3_vfs_register(sqlite3_vfs *pVfs, int makeDflt) {
  sqlite3_mutex *mutex;

  int rc = sqlite3_initialize();
  if (rc)
    return rc;

  mutex = sqlite3MutexAlloc(2);
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  if (makeDflt || vfsList == 0) {
    pVfs->pNext = vfsList;
    vfsList = pVfs;
  } else {
    pVfs->pNext = vfsList->pNext;
    vfsList->pNext = pVfs;
  }

  sqlite3_mutex_leave(mutex);
  return 0;
}

int sqlite3_vfs_unregister(sqlite3_vfs *pVfs) {
  sqlite3_mutex *mutex;

  int rc = sqlite3_initialize();
  if (rc)
    return rc;

  mutex = sqlite3MutexAlloc(2);
  sqlite3_mutex_enter(mutex);
  vfsUnlink(pVfs);
  sqlite3_mutex_leave(mutex);
  return 0;
}

int unixSetSystemCall(sqlite3_vfs *pNotUsed, const char *zName, sqlite3_syscall_ptr pNewFunc) {
  unsigned int i;
  int rc = 12;

  (void)(pNotUsed);
  if (zName == 0) {

    rc = 0;
    for (i = 0; i < sizeof(aSyscall) / sizeof(aSyscall[SQLITE_SYSCALL_OPEN]); i++) {
      if (aSyscall[i].pDefault) {
        aSyscall[i].pCurrent = aSyscall[i].pDefault;
      }
    }
  } else {

    for (i = 0; i < sizeof(aSyscall) / sizeof(aSyscall[SQLITE_SYSCALL_OPEN]); i++) {
      if (strcmp(zName, aSyscall[i].zName) == 0) {
        if (aSyscall[i].pDefault == 0) {
          aSyscall[i].pDefault = aSyscall[i].pCurrent;
        }
        rc = 0;
        if (pNewFunc == 0)
          pNewFunc = aSyscall[i].pDefault;
        aSyscall[i].pCurrent = pNewFunc;
        break;
      }
    }
  }
  return rc;
}

sqlite3_syscall_ptr unixGetSystemCall(sqlite3_vfs *pNotUsed, const char *zName) {
  unsigned int i;

  (void)(pNotUsed);
  for (i = 0; i < sizeof(aSyscall) / sizeof(aSyscall[SQLITE_SYSCALL_OPEN]); i++) {
    if (strcmp(zName, aSyscall[i].zName) == 0)
      return aSyscall[i].pCurrent;
  }
  return 0;
}

const char *unixNextSystemCall(sqlite3_vfs *p, const char *zName) {
  int i = -1;

  (void)(p);
  if (zName) {
    for (i = 0; i < ((int)(sizeof(aSyscall) / sizeof(aSyscall[SQLITE_SYSCALL_OPEN]))) - 1; i++) {
      if (strcmp(zName, aSyscall[i].zName) == 0)
        break;
    }
  }
  for (i++; i < ((int)(sizeof(aSyscall) / sizeof(aSyscall[SQLITE_SYSCALL_OPEN]))); i++) {
    if (aSyscall[i].pCurrent != 0)
      return aSyscall[i].zName;
  }
  return 0;
}

int fillInUnixFile(sqlite3_vfs *pVfs, int h, sqlite3_file *pId, const char *zFilename, int ctrlFlags) {
  const sqlite3_io_methods *pLockingStyle;
  unixFile *pNew = (unixFile *)pId;
  int rc = 0;

  ;
  pNew->h = h;
  pNew->pVfs = pVfs;
  pNew->zPath = zFilename;
  pNew->ctrlFlags = (u8)ctrlFlags;

  pNew->mmapSizeMax = sqlite3Config.szMmap;

  if (sqlite3_uri_boolean(((ctrlFlags & 0x40) ? zFilename : 0), "psow", 1)) {
    pNew->ctrlFlags |= 0x10;
  }
  if (strcmp(pVfs->zName, "unix-excl") == 0) {
    pNew->ctrlFlags |= 0x01;
  }

  if (ctrlFlags & 0x80) {
    pLockingStyle = &nolockIoMethods;
  } else {
    pLockingStyle = (**(finder_type *)pVfs->pAppData)(zFilename, pNew);
  }

  if (pLockingStyle == &posixIoMethods

  ) {
    unixEnterMutex();
    rc = findInodeInfo(pNew, &pNew->pInode);
    if (rc != 0) {

      robust_close(pNew, h, 46355);
      h = -1;
    }
    unixLeaveMutex();
  }

  else if (pLockingStyle == &dotlockIoMethods) {

    char *zLockFile;
    int nFilename;


    nFilename = (int)strlen(zFilename) + 6;
    zLockFile = (char *)sqlite3_malloc64(nFilename);
    if (zLockFile == 0) {
      rc = 7;
    } else {
      sqlite3_snprintf(nFilename, zLockFile,
                       "%s"
                       ".lock",
                       zFilename);
    }
    pNew->lockingContext = zLockFile;
  }

  storeLastErrno(pNew, 0);

  if (rc != 0) {
    if (h >= 0)
      robust_close(pNew, h, 46447);
  } else {
    pId->pMethods = pLockingStyle;
    ;
    verifyDbFile(pNew);
  }
  return rc;
}

int unixOpen(sqlite3_vfs *pVfs, const char *zPath, sqlite3_file *pFile, int flags, int *pOutFlags) {
  unixFile *p = (unixFile *)pFile;
  int fd = -1;
  int openFlags = 0;
  int eType = flags & 0x0FFF00;
  int noLock;
  int rc = 0;
  int ctrlFlags = 0;

  int isExclusive = (flags & 0x00000010);
  int isDelete = (flags & 0x00000008);
  int isCreate = (flags & 0x00000004);
  int isReadonly = (flags & 0x00000001);
  int isReadWrite = (flags & 0x00000002);

  int isNewJrnl = (isCreate && (eType == 0x00004000 || eType == 0x00000800 || eType == 0x00080000));

  char zTmpname[512 + 2];
  const char *zName = zPath;

  if (randomnessPid != (pid_t)getpid()) {
    randomnessPid = (pid_t)getpid();
    sqlite3_randomness(0, 0);
  }
  memset(p, 0, sizeof(unixFile));

  if (eType == 0x00000100) {
    UnixUnusedFd *pUnused;
    pUnused = findReusableFd(zName, flags);
    if (pUnused) {
      fd = pUnused->fd;
    } else {
      pUnused = sqlite3_malloc64(sizeof(*pUnused));
      if (!pUnused) {
        return 7;
      }
    }
    p->pPreallocatedUnused = pUnused;



  } else if (!zName) {


    rc = unixGetTempname(pVfs->mxPathname, zTmpname);
    if (rc != 0) {
      return rc;
    }
    zName = zTmpname;


  }

  if (isReadonly)
    openFlags |=

        00

        ;
  if (isReadWrite)
    openFlags |=

        02

        ;
  if (isCreate)
    openFlags |=

        0100

        ;
  if (isExclusive)
    openFlags |= (

        0200

        |

        0400000

    );
  openFlags |= (

      0

      | 0 |

      0400000

  );

  if (fd < 0) {
    mode_t openMode;
    uid_t uid;
    gid_t gid;
    rc = findCreateFileMode(zName, flags, &openMode, &uid, &gid);
    if (rc != 0) {




      return rc;
    }
    fd = robust_open(zName, openFlags, openMode);
    ;


    if (fd < 0) {
      if (isNewJrnl &&

          (*__errno_location())

              ==

              13

          && ((int (*)(const char *, int))aSyscall[SQLITE_SYSCALL_ACCESS].pCurrent)(zName,

                                                                0

                                                                )) {

        rc = (8 | (6 << 8));
      } else if (

          (*__errno_location())

              !=

              21

          && isReadWrite) {

        UnixUnusedFd *pReadonly = 0;
        flags &= ~(0x00000002 | 0x00000004);
        openFlags &= ~(

            02

            |

            0100

        );
        flags |= 0x00000001;
        openFlags |=

            00

            ;
        isReadonly = 1;
        pReadonly = findReusableFd(zName, flags);
        if (pReadonly) {
          fd = pReadonly->fd;
          sqlite3_free(pReadonly);
        } else {
          fd = robust_open(zName, openFlags, openMode);
        }
      }
    }
    if (fd < 0) {
      int rc2 = unixLogErrorAtLine(sqlite3CantopenError(46904), "open", zName, 46904);
      if (rc == 0)
        rc = rc2;
      goto open_finished;
    }

    if (openMode && (flags & (0x00080000 | 0x00000800)) != 0) {
      robustFchown(fd, uid, gid);
    }
  }

  if (pOutFlags) {
    *pOutFlags = flags;
  }

  if (p->pPreallocatedUnused) {
    p->pPreallocatedUnused->fd = fd;
    p->pPreallocatedUnused->flags = flags & (0x00000001 | 0x00000002);
  }

  if (isDelete) {

    ((int (*)(const char *))aSyscall[SQLITE_SYSCALL_UNLINK].pCurrent)(zName);
  }

  if (isDelete)
    ctrlFlags |= 0x20;
  if (isReadonly)
    ctrlFlags |= 0x02;
  noLock = eType != 0x00000100;
  if (noLock)
    ctrlFlags |= 0x80;
  if (isNewJrnl)
    ctrlFlags |= 0x08;
  if (flags & 0x00000040)
    ctrlFlags |= 0x40;

  rc = fillInUnixFile(pVfs, fd, pFile, zPath, ctrlFlags);

open_finished:
  if (rc != 0) {
    sqlite3_free(p->pPreallocatedUnused);
  }
  return rc;
}

int unixDelete(sqlite3_vfs *NotUsed, const char *zPath, int dirSync) {
  int rc = 0;
  (void)(NotUsed);
  ;
  if (((int (*)(const char *))aSyscall[SQLITE_SYSCALL_UNLINK].pCurrent)(zPath) == (-1)) {
    if (

        (*__errno_location())

        ==

        2

    ) {
      rc = (10 | (23 << 8));
    } else {
      rc = unixLogErrorAtLine((10 | (10 << 8)), "unlink", zPath, 47046);
    }
    return rc;
  }

  if ((dirSync & 1) != 0) {
    int fd;
    rc = ((int (*)(const char *, int *))aSyscall[SQLITE_SYSCALL_OPENDIRECTORY].pCurrent)(zPath, &fd);
    if (rc == 0) {
      if (full_fsync(fd, 0, 0)) {
        rc = unixLogErrorAtLine((10 | (5 << 8)), "fsync", zPath, 47056);
      }
      robust_close(0, fd, 47058);
    } else {


      rc = 0;
    }
  }

  return rc;
}

int unixAccess(sqlite3_vfs *NotUsed, const char *zPath, int flags, int *pResOut) {
  (void)(NotUsed);
  ;

  if (flags == 0) {
    struct stat buf;
    *pResOut = 0 == ((int (*)(const char *, struct stat *))aSyscall[SQLITE_SYSCALL_STAT].pCurrent)(zPath, &buf) && (!

                                                                                                  ((((

                                                                                                        buf.st_mode

                                                                                                        )) &
                                                                                                    0170000) == (0100000))

                                                                                                  || buf.st_size > 0);
  } else {
    *pResOut = ((int (*)(const char *, int))aSyscall[SQLITE_SYSCALL_ACCESS].pCurrent)(zPath,

                                                                  2

                                                                      |

                                                                      4

                                                                  ) == 0;
  }
  return 0;
}

int unixFullPathname(sqlite3_vfs *pVfs, const char *zPath, int nOut, char *zOut) {
  DbPath path;
  (void)(pVfs);
  path.rc = 0;
  path.nUsed = 0;
  path.nSymlink = 0;
  path.nOut = nOut;
  path.zOut = zOut;
  if (zPath[0] != '/') {
    char zPwd[

        4096

        + 2];
    if (((char *(*)(char *, size_t))aSyscall[SQLITE_SYSCALL_GETCWD].pCurrent)(zPwd, sizeof(zPwd) - 2) == 0) {
      return unixLogErrorAtLine(sqlite3CantopenError(47221), "getcwd", zPath, 47221);
    }
    appendAllPathElements(&path, zPwd);
  }
  appendAllPathElements(&path, zPath);
  zOut[path.nUsed] = 0;
  if (path.rc || path.nUsed < 2)
    return sqlite3CantopenError(47227);
  if (path.nSymlink)
    return (0 | (2 << 8));
  return 0;
}

void *unixDlOpen(sqlite3_vfs *NotUsed, const char *zFilename) {
  (void)(NotUsed);
  return dlopen(zFilename,

                0x00002

                    |

                    0x00100

  );
}

void unixDlError(sqlite3_vfs *NotUsed, int nBuf, char *zBufOut) {
  const char *zErr;
  (void)(NotUsed);
  unixEnterMutex();
  zErr = dlerror();
  if (zErr) {
    sqlite3_snprintf(nBuf, zBufOut, "%s", zErr);
  }
  unixLeaveMutex();
}

void (*unixDlSym(sqlite3_vfs *NotUsed, void *p, const char *zSym))(void) {

  void (*(*x)(void *, const char *))(void);
  (void)(NotUsed);
  x = (void (*(*)(void *, const char *))(void))dlsym;
  return (*x)(p, zSym);
}

void unixDlClose(sqlite3_vfs *NotUsed, void *pHandle) {
  (void)(NotUsed);
  dlclose(pHandle);
}

int unixRandomness(sqlite3_vfs *NotUsed, int nBuf, char *zBuf) {
  (void)(NotUsed);

  memset(zBuf, 0, nBuf);
  randomnessPid = (pid_t)getpid();

  {
    int fd, got;
    fd = robust_open("/dev/urandom",

                     00

                     ,
                     0);
    if (fd < 0) {
      time_t t;
      time(&t);
      memcpy(zBuf, &t, sizeof(t));
      memcpy(&zBuf[sizeof(t)], &randomnessPid, sizeof(randomnessPid));


      nBuf = sizeof(t) + sizeof(randomnessPid);
    } else {
      do {
        got = ((ssize_t (*)(int, void *, size_t))aSyscall[SQLITE_SYSCALL_READ].pCurrent)(fd, zBuf, nBuf);
      } while (got < 0 &&

               (*__errno_location())

                   ==

                   4

      );
      robust_close(0, fd, 47328);
    }
  }

  return nBuf;
}

int unixSleep(sqlite3_vfs *NotUsed, int microseconds) {

  struct timespec sp;
  sp.tv_sec = microseconds / 1000000;
  sp.tv_nsec = (microseconds % 1000000) * 1000;

  nanosleep(&sp,

            ((void *)0)

  );

  (void)(NotUsed);
  return microseconds;
}

int unixCurrentTimeInt64(sqlite3_vfs *NotUsed, sqlite3_int64 *piNow) {
  static const sqlite3_int64 unixEpoch = 24405875 * (sqlite3_int64)8640000;
  int rc = 0;

  struct timeval sNow;
  (void)gettimeofday(&sNow, 0);
  *piNow = unixEpoch + 1000 * (sqlite3_int64)sNow.tv_sec + sNow.tv_usec / 1000;

  (void)(NotUsed);
  return rc;
}

int unixCurrentTime(sqlite3_vfs *NotUsed, double *prNow) {
  sqlite3_int64 i = 0;
  int rc;
  (void)(NotUsed);
  rc = unixCurrentTimeInt64(0, &i);
  *prNow = i / 86400000.0;
  return rc;
}

int unixGetLastError(sqlite3_vfs *NotUsed, int NotUsed2, char *NotUsed3) {
  (void)(NotUsed);
  (void)(NotUsed2);
  (void)(NotUsed3);
  return

      (*__errno_location())

          ;
}

sqlite3_vfs memdb_vfs = {
    2, 0, 1024, 0, "memdb", 0, memdbOpen, 0, memdbAccess, memdbFullPathname, memdbDlOpen, memdbDlError, memdbDlSym, memdbDlClose, memdbRandomness, memdbSleep, 0, memdbGetLastError, memdbCurrentTimeInt64, 0, 0, 0,
};

int memdbOpen(sqlite3_vfs *pVfs, const char *zName, sqlite3_file *pFd, int flags, int *pOutFlags) {
  MemFile *pFile = (MemFile *)pFd;
  MemStore *p = 0;
  int szName;
  (void)(pVfs);

  memset(pFile, 0, sizeof(*pFile));
  szName = sqlite3Strlen30(zName);
  if (szName > 1 && (zName[0] == '/' || zName[0] == '\\')) {
    int i;

    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(11);

    sqlite3_mutex_enter(pVfsMutex);
    for (i = 0; i < memdb_g.nMemStore; i++) {
      if (strcmp(memdb_g.apMemStore[i]->zFName, zName) == 0) {
        p = memdb_g.apMemStore[i];
        break;
      }
    }
    if (p == 0) {
      MemStore **apNew;
      p = sqlite3Malloc(sizeof(*p) + (i64)szName + 3);
      if (p == 0) {
        sqlite3_mutex_leave(pVfsMutex);
        return 7;
      }
      apNew = sqlite3Realloc(memdb_g.apMemStore, sizeof(apNew[0]) * (1 + (i64)memdb_g.nMemStore));
      if (apNew == 0) {
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        return 7;
      }
      apNew[memdb_g.nMemStore++] = p;
      memdb_g.apMemStore = apNew;
      memset(p, 0, sizeof(*p));
      p->mFlags = 2 | 1;
      p->szMax = sqlite3Config.mxMemdbSize;
      p->zFName = (char *)&p[1];
      memcpy(p->zFName, zName, szName + 1);
      p->pMutex = sqlite3_mutex_alloc(0);
      if (p->pMutex == 0) {
        memdb_g.nMemStore--;
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        return 7;
      }
      p->nRef = 1;
      memdbEnter(p);
    } else {
      memdbEnter(p);
      p->nRef++;
    }
    sqlite3_mutex_leave(pVfsMutex);
  } else {
    p = sqlite3Malloc(sizeof(*p));
    if (p == 0) {
      return 7;
    }
    memset(p, 0, sizeof(*p));
    p->mFlags = 2 | 1;
    p->szMax = sqlite3Config.mxMemdbSize;
  }
  pFile->pStore = p;
  if (pOutFlags != 0) {
    *pOutFlags = flags | 0x00000080;
  }
  pFd->pMethods = &memdb_io_methods;
  memdbLeave(p);
  return 0;
}

int memdbAccess(sqlite3_vfs *pVfs, const char *zPath, int flags, int *pResOut) {
  (void)(pVfs);
  (void)(zPath);
  (void)(flags);
  *pResOut = 0;
  return 0;
}

int memdbFullPathname(sqlite3_vfs *pVfs, const char *zPath, int nOut, char *zOut) {
  (void)(pVfs);
  sqlite3_snprintf(nOut, zOut, "%s", zPath);
  return 0;
}

void *memdbDlOpen(sqlite3_vfs *pVfs, const char *zPath) { return ((sqlite3_vfs *)((pVfs)->pAppData))->xDlOpen(((sqlite3_vfs *)((pVfs)->pAppData)), zPath); }

void memdbDlError(sqlite3_vfs *pVfs, int nByte, char *zErrMsg) { ((sqlite3_vfs *)((pVfs)->pAppData))->xDlError(((sqlite3_vfs *)((pVfs)->pAppData)), nByte, zErrMsg); }

void (*memdbDlSym(sqlite3_vfs *pVfs, void *p, const char *zSym))(void) { return ((sqlite3_vfs *)((pVfs)->pAppData))->xDlSym(((sqlite3_vfs *)((pVfs)->pAppData)), p, zSym); }

void memdbDlClose(sqlite3_vfs *pVfs, void *pHandle) { ((sqlite3_vfs *)((pVfs)->pAppData))->xDlClose(((sqlite3_vfs *)((pVfs)->pAppData)), pHandle); }

int memdbRandomness(sqlite3_vfs *pVfs, int nByte, char *zBufOut) { return ((sqlite3_vfs *)((pVfs)->pAppData))->xRandomness(((sqlite3_vfs *)((pVfs)->pAppData)), nByte, zBufOut); }

int memdbSleep(sqlite3_vfs *pVfs, int nMicro) { return ((sqlite3_vfs *)((pVfs)->pAppData))->xSleep(((sqlite3_vfs *)((pVfs)->pAppData)), nMicro); }

int memdbGetLastError(sqlite3_vfs *pVfs, int a, char *b) { return ((sqlite3_vfs *)((pVfs)->pAppData))->xGetLastError(((sqlite3_vfs *)((pVfs)->pAppData)), a, b); }

int memdbCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p) { return ((sqlite3_vfs *)((pVfs)->pAppData))->xCurrentTimeInt64(((sqlite3_vfs *)((pVfs)->pAppData)), p); }

int sqlite3IsMemdb(const sqlite3_vfs *pVfs) { return pVfs == &memdb_vfs; }

int sqlite3PagerOpen(sqlite3_vfs *pVfs, Pager **ppPager, const char *zFilename, int nExtra, int flags, int vfsFlags, void (*xReinit)(DbPage *)) {
  u8 *pPtr;
  Pager *pPager = 0;
  int rc = 0;
  int tempFile = 0;
  int memDb = 0;
  int memJM = 0;
  int readOnly = 0;
  int journalFileSize;
  char *zPathname = 0;
  int nPathname = 0;
  int useJournal = (flags & 0x0001) == 0;
  int pcacheSize = sqlite3PcacheSize();
  u32 szPageDflt = 4096;
  const char *zUri = 0;
  int nUriByte = 1;

  journalFileSize = (((sqlite3JournalSize(pVfs)) + 7) & ~7);

  *ppPager = 0;

  if (flags & 0x0002) {
    memDb = 1;
    if (zFilename && zFilename[0]) {
      zPathname = sqlite3DbStrDup(0, zFilename);
      if (zPathname == 0)
        return 7;
      nPathname = sqlite3Strlen30(zPathname);
      zFilename = 0;
    }
  }

  if (zFilename && zFilename[0]) {
    const char *z;
    nPathname = pVfs->mxPathname + 1;
    zPathname = sqlite3DbMallocRaw(0, 2 * (i64)nPathname);
    if (zPathname == 0) {
      return 7;
    }
    zPathname[0] = 0;
    rc = sqlite3OsFullPathname(pVfs, zFilename, nPathname, zPathname);
    if (rc != 0) {
      if (rc == (0 | (2 << 8))) {
        if (vfsFlags & 0x01000000) {
          rc = (14 | (6 << 8));
        } else {
          rc = 0;
        }
      }
    }
    nPathname = sqlite3Strlen30(zPathname);
    z = zUri = &zFilename[sqlite3Strlen30(zFilename) + 1];
    while (*z) {
      z += strlen(z) + 1;
      z += strlen(z) + 1;
    }
    nUriByte = (int)(&z[1] - zUri);


    if (rc == 0 && nPathname + 8 > pVfs->mxPathname) {

      rc = sqlite3CantopenError(64498);
    }
    if (rc != 0) {
      sqlite3DbFree(0, zPathname);
      return rc;
    }
  }

  pPtr = (u8 *)sqlite3MallocZero((((sizeof(*pPager)) + 7) & ~7) + (((pcacheSize) + 7) & ~7) + (((pVfs->szOsFile) + 7) & ~7) + (u64)journalFileSize * 2 + 8 + 4 + (u64)nPathname + 1 + (u64)nUriByte + (u64)nPathname + 8 + 1 +

                                 (u64)nPathname + 4 + 1 +

                                 3);

  if (!pPtr) {
    sqlite3DbFree(0, zPathname);
    return 7;
  }
  pPager = (Pager *)pPtr;
  pPtr += (((sizeof(*pPager)) + 7) & ~7);
  pPager->pPCache = (PCache *)pPtr;
  pPtr += (((pcacheSize) + 7) & ~7);
  pPager->fd = (sqlite3_file *)pPtr;
  pPtr += (((pVfs->szOsFile) + 7) & ~7);
  pPager->sjfd = (sqlite3_file *)pPtr;
  pPtr += journalFileSize;
  pPager->jfd = (sqlite3_file *)pPtr;
  pPtr += journalFileSize;

  memcpy(pPtr, &pPager, 8);
  pPtr += 8;

  pPtr += 4;
  pPager->zFilename = (char *)pPtr;
  if (nPathname > 0) {
    memcpy(pPtr, zPathname, nPathname);
    pPtr += nPathname + 1;
    if (zUri) {
      memcpy(pPtr, zUri, nUriByte);
      pPtr += nUriByte;
    } else {
      pPtr++;
    }
  }

  if (nPathname > 0) {
    pPager->zJournal = (char *)pPtr;
    memcpy(pPtr, zPathname, nPathname);
    pPtr += nPathname;
    memcpy(pPtr, "-journal", 8);
    pPtr += 8 + 1;

  } else {
    pPager->zJournal = 0;
  }

  if (nPathname > 0) {
    pPager->zWal = (char *)pPtr;
    memcpy(pPtr, zPathname, nPathname);
    pPtr += nPathname;
    memcpy(pPtr, "-wal", 4);
    pPtr += 4 + 1;

  } else {
    pPager->zWal = 0;
  }

  (void)pPtr;

  if (nPathname)
    sqlite3DbFree(0, zPathname);
  pPager->pVfs = pVfs;
  pPager->vfsFlags = vfsFlags;

  if (zFilename && zFilename[0]) {
    int fout = 0;
    rc = sqlite3OsOpen(pVfs, pPager->zFilename, pPager->fd, vfsFlags, &fout);


    pPager->memVfs = memJM = (fout & 0x00000080) != 0;
    readOnly = (fout & 0x00000001) != 0;

    if (rc == 0) {
      int iDc = sqlite3OsDeviceCharacteristics(pPager->fd);
      if (!readOnly) {
        setSectorSize(pPager);


        if (szPageDflt < pPager->sectorSize) {
          if (pPager->sectorSize > 8192) {
            szPageDflt = 8192;
          } else {
            szPageDflt = (u32)pPager->sectorSize;
          }
        }
      }
      pPager->noLock = sqlite3_uri_boolean(pPager->zFilename, "nolock", 0);
      if ((iDc & 0x00002000) != 0 || sqlite3_uri_boolean(pPager->zFilename, "immutable", 0)) {
        vfsFlags |= 0x00000001;
        goto act_like_temp_file;
      }
    }
  } else {

  act_like_temp_file:
    tempFile = 1;
    pPager->eState = 1;
    pPager->eLock = 4;
    pPager->noLock = 1;
    readOnly = (vfsFlags & 0x00000001);
  }

  if (rc == 0) {


    rc = sqlite3PagerSetPagesize(pPager, &szPageDflt, -1);
    ;
  }

  if (rc == 0) {
    nExtra = (((nExtra) + 7) & ~7);


    rc = sqlite3PcacheOpen(szPageDflt, nExtra, !memDb, !memDb ? pagerStress : 0, (void *)pPager, pPager->pPCache);
  }

  if (rc != 0) {
    sqlite3OsClose(pPager->fd);
    sqlite3PageFree(pPager->pTmpSpace);
    sqlite3_free(pPager);
    return rc;
  }

  ;

  pPager->useJournal = (u8)useJournal;

  pPager->mxPgno = 0xfffffffe;

  pPager->tempFile = (u8)tempFile;

  pPager->exclusiveMode = (u8)tempFile;
  pPager->changeCountDone = pPager->tempFile;
  pPager->memDb = (u8)memDb;
  pPager->readOnly = (u8)readOnly;

  sqlite3PagerSetFlags(pPager, (2 + 1) | 0x20);

  pPager->nExtra = (u16)nExtra;
  pPager->journalSizeLimit = -1;

  setSectorSize(pPager);
  if (!useJournal) {
    pPager->journalMode = 2;
  } else if (memDb || memJM) {
    pPager->journalMode = 4;
  }

  pPager->xReiniter = xReinit;
  setGetterMethod(pPager);

  *ppPager = pPager;
  return 0;
}

int sqlite3WalOpen(sqlite3_vfs *pVfs, sqlite3_file *pDbFd, const char *zWalName, int bNoShm, i64 mxWalSize, Wal **ppWal) {
  int rc;
  Wal *pRet;
  int flags;

  *ppWal = 0;
  pRet = (Wal *)sqlite3MallocZero(sizeof(Wal) + pVfs->szOsFile);
  if (!pRet) {
    return 7;
  }

  pRet->pVfs = pVfs;
  pRet->pWalFd = (sqlite3_file *)&pRet[1];
  pRet->pDbFd = pDbFd;
  pRet->readLock = -1;
  pRet->mxWalSize = mxWalSize;
  pRet->zWalName = zWalName;
  pRet->syncHeader = 1;
  pRet->padToSectorBoundary = 1;
  pRet->exclusiveMode = (bNoShm ? 2 : 0);

  flags = (0x00000002 | 0x00000004 | 0x00080000);
  rc = sqlite3OsOpen(pVfs, zWalName, pRet->pWalFd, flags, &flags);
  if (rc == 0 && flags & 0x00000001) {
    pRet->readOnly = 1;
  }

  if (rc != 0) {
    walIndexClose(pRet, 0);
    sqlite3OsClose(pRet->pWalFd);
    sqlite3_free(pRet);
  } else {
    int iDC = sqlite3OsDeviceCharacteristics(pDbFd);
    if (iDC & 0x00000400) {
      pRet->syncHeader = 0;
    }
    if (iDC & 0x00001000) {
      pRet->padToSectorBoundary = 0;
    }
    *ppWal = pRet;
    ;
  }
  return rc;
}

int sqlite3BtreeOpen(sqlite3_vfs *pVfs, const char *zFilename, sqlite3 *db, Btree **ppBtree, int flags, int vfsFlags) {
  BtShared *pBt = 0;
  Btree *p;
  sqlite3_mutex *mutexOpen = 0;
  int rc = 0;
  u8 nReserve;
  unsigned char zDbHeader[100];

  const int isTempDb = zFilename == 0 || zFilename[0] == 0;

  const int isMemdb = (zFilename && strcmp(zFilename, ":memory:") == 0) || (isTempDb && sqlite3TempInMemory(db)) || (vfsFlags & 0x00000080) != 0;

  if (isMemdb) {
    flags |= 2;
  }
  if ((vfsFlags & 0x00000100) != 0 && (isMemdb || isTempDb)) {
    vfsFlags = (vfsFlags & ~0x00000100) | 0x00000200;
  }
  p = sqlite3MallocZero(sizeof(Btree));
  if (!p) {
    return 7;
  }
  p->inTrans = 0;
  p->db = db;

  p->lock.pBtree = p;
  p->lock.iTable = 1;

  if (isTempDb == 0 && (isMemdb == 0 || (vfsFlags & 0x00000040) != 0)) {
    if (vfsFlags & 0x00020000) {
      int nFilename = sqlite3Strlen30(zFilename) + 1;
      int nFullPathname = pVfs->mxPathname + 1;
      char *zFullPathname = sqlite3Malloc(((nFullPathname) > (nFilename) ? (nFullPathname) : (nFilename)));
      sqlite3_mutex *mutexShared;

      p->sharable = 1;
      if (!zFullPathname) {
        sqlite3_free(p);
        return 7;
      }
      if (isMemdb) {
        memcpy(zFullPathname, zFilename, nFilename);
      } else {
        rc = sqlite3OsFullPathname(pVfs, zFilename, nFullPathname, zFullPathname);
        if (rc) {
          if (rc == (0 | (2 << 8))) {
            rc = 0;
          } else {
            sqlite3_free(zFullPathname);
            sqlite3_free(p);
            return rc;
          }
        }
      }

      mutexOpen = sqlite3MutexAlloc(4);
      sqlite3_mutex_enter(mutexOpen);
      mutexShared = sqlite3MutexAlloc(2);
      sqlite3_mutex_enter(mutexShared);

      for (pBt = sqlite3SharedCacheList; pBt; pBt = pBt->pNext) {


        if (0 == strcmp(zFullPathname, sqlite3PagerFilename(pBt->pPager, 0)) && sqlite3PagerVfs(pBt->pPager) == pVfs) {
          int iDb;
          for (iDb = db->nDb - 1; iDb >= 0; iDb--) {
            Btree *pExisting = db->aDb[iDb].pBt;
            if (pExisting && pExisting->pBt == pBt) {
              sqlite3_mutex_leave(mutexShared);
              sqlite3_mutex_leave(mutexOpen);
              sqlite3_free(zFullPathname);
              sqlite3_free(p);
              return 19;
            }
          }
          p->pBt = pBt;
          pBt->nRef++;
          break;
        }
      }
      sqlite3_mutex_leave(mutexShared);
      sqlite3_free(zFullPathname);
    }
  }

  if (pBt == 0) {











    memset(&zDbHeader[16], 0, 8);

    pBt = sqlite3MallocZero(sizeof(*pBt));
    if (pBt == 0) {
      rc = 7;
      goto btree_open_out;
    }
    rc = sqlite3PagerOpen(pVfs, &pBt->pPager, zFilename, sizeof(MemPage), flags, vfsFlags, pageReinit);
    if (rc == 0) {
      sqlite3PagerSetMmapLimit(pBt->pPager, db->szMmap);
      rc = sqlite3PagerReadFileheader(pBt->pPager, sizeof(zDbHeader), zDbHeader);
    }
    if (rc != 0) {
      goto btree_open_out;
    }
    pBt->openFlags = (u8)flags;
    pBt->db = db;
    sqlite3PagerSetBusyHandler(pBt->pPager, btreeInvokeBusyHandler, pBt);
    p->pBt = pBt;

    pBt->pCursor = 0;
    pBt->pPage1 = 0;
    if (sqlite3PagerIsreadonly(pBt->pPager))
      pBt->btsFlags |= 0x0001;

    pBt->pageSize = (zDbHeader[16] << 8) | (zDbHeader[17] << 16);
    if (pBt->pageSize < 512 || pBt->pageSize > 65536 || ((pBt->pageSize - 1) & pBt->pageSize) != 0) {
      pBt->pageSize = 0;

      if (zFilename && !isMemdb) {
        pBt->autoVacuum = (0 ? 1 : 0);
        pBt->incrVacuum = (0 == 2 ? 1 : 0);
      }

      nReserve = 0;
    } else {

      nReserve = zDbHeader[20];
      pBt->btsFlags |= 0x0002;

      pBt->autoVacuum = (sqlite3Get4byte(&zDbHeader[36 + 4 * 4]) ? 1 : 0);
      pBt->incrVacuum = (sqlite3Get4byte(&zDbHeader[36 + 7 * 4]) ? 1 : 0);
    }
    rc = sqlite3PagerSetPagesize(pBt->pPager, &pBt->pageSize, nReserve);
    if (rc)
      goto btree_open_out;
    pBt->usableSize = pBt->pageSize - nReserve;



    pBt->nRef = 1;
    if (p->sharable) {
      sqlite3_mutex *mutexShared;
      mutexShared = sqlite3MutexAlloc(2);
      if (1 && sqlite3Config.bCoreMutex) {
        pBt->mutex = sqlite3MutexAlloc(0);
        if (pBt->mutex == 0) {
          rc = 7;
          goto btree_open_out;
        }
      }
      sqlite3_mutex_enter(mutexShared);
      pBt->pNext = sqlite3SharedCacheList;
      sqlite3SharedCacheList = pBt;
      sqlite3_mutex_leave(mutexShared);
    }
  }

  if (p->sharable) {
    int i;
    Btree *pSib;
    for (i = 0; i < db->nDb; i++) {
      if ((pSib = db->aDb[i].pBt) != 0 && pSib->sharable) {
        while (pSib->pPrev) {
          pSib = pSib->pPrev;
        }
        if ((uptr)p->pBt < (uptr)pSib->pBt) {
          p->pNext = pSib;
          p->pPrev = 0;
          pSib->pPrev = p;
        } else {
          while (pSib->pNext && (uptr)pSib->pNext->pBt < (uptr)p->pBt) {
            pSib = pSib->pNext;
          }
          p->pNext = pSib->pNext;
          p->pPrev = pSib;
          if (p->pNext) {
            p->pNext->pPrev = p;
          }
          pSib->pNext = p;
        }
        break;
      }
    }
  }

  *ppBtree = p;

btree_open_out:
  if (rc != 0) {
    if (pBt && pBt->pPager) {
      sqlite3PagerClose(pBt->pPager, 0);
    }
    sqlite3_free(pBt);
    sqlite3_free(p);
    *ppBtree = 0;
  } else {
    sqlite3_file *pFile;

    if (sqlite3BtreeSchema(p, 0, 0) == 0) {
      sqlite3BtreeSetCacheSize(p, -2000);
    }

    pFile = sqlite3PagerFile(pBt->pPager);
    if (pFile->pMethods) {
      sqlite3OsFileControlHint(pFile, 30, (void *)&pBt->db);
    }
  }
  if (mutexOpen) {


    sqlite3_mutex_leave(mutexOpen);
  }

  return rc;
}

int sqlite3JournalOpen(sqlite3_vfs *pVfs, const char *zName, sqlite3_file *pJfd, int flags, int nSpill) {
  MemJournal *p = (MemJournal *)pJfd;

  memset(p, 0, sizeof(MemJournal));
  if (nSpill == 0) {
    return sqlite3OsOpen(pVfs, zName, pJfd, flags, 0);
  }

  if (nSpill > 0) {
    p->nChunkSize = nSpill;
  } else {
    p->nChunkSize = 8 + 1024 - sizeof(FileChunk);


  }

  pJfd->pMethods = (const sqlite3_io_methods *)&MemJournalMethods;
  p->nSpill = nSpill;
  p->flags = flags;
  p->zJournal = zName;
  p->pVfs = pVfs;
  return 0;
}

int sqlite3JournalSize(sqlite3_vfs *pVfs) { return ((pVfs->szOsFile) > ((int)sizeof(MemJournal)) ? (pVfs->szOsFile) : ((int)sizeof(MemJournal))); }