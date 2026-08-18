
#pragma once

#include "sqlite/i64.h"
#include "sqlite/sqlite3_int64.h"
  typedef struct UnixUnusedFd UnixUnusedFd;
  typedef struct sqlite3_io_methods sqlite3_io_methods;
  typedef struct sqlite3_vfs sqlite3_vfs;
  typedef struct unixInodeInfo unixInodeInfo;
  typedef struct unixShm unixShm;
  typedef struct unixShmNode unixShmNode;
  typedef struct unixFile unixFile;
  struct flock;
  struct unixFile {
    sqlite3_io_methods const *pMethod;
    sqlite3_vfs *pVfs;
    unixInodeInfo *pInode;
    int h;
    unsigned char eFileLock;
    unsigned short int ctrlFlags;
    int lastErrno;
    void *lockingContext;
    UnixUnusedFd *pPreallocatedUnused;
    const char *zPath;
    unixShm *pShm;
    int szChunk;

    int nFetchOut;
    sqlite3_int64 mmapSize;
    sqlite3_int64 mmapSizeActual;
    sqlite3_int64 mmapSizeMax;
    void *pMapRegion;

    int sectorSize;
    int deviceCharacteristics;
  };

  void robust_close(unixFile * pFile, int h, int lineno);
  void storeLastErrno(unixFile * pFile, int error);
  void closePendingFds(unixFile * pFile);
  void releaseInodeInfo(unixFile * pFile);
  int findInodeInfo(unixFile * pFile, unixInodeInfo * *ppInode);
  int fileHasMoved(unixFile * pFile);
  void verifyDbFile(unixFile * pFile);
  int unixFileLock(unixFile * pFile, struct flock * pLock);
  int unixIsSharingShmNode(unixFile *);
  void setPendingFd(unixFile * pFile);
  int unixMapfile(unixFile * pFd, i64 nByte);
  void unixUnmapfile(unixFile * pFd);
  int seekAndRead(unixFile * id, sqlite3_int64 offset, void *pBuf, int cnt);
  int seekAndWrite(unixFile * id, i64 offset, const void *pBuf, int cnt);
  int fcntlSizeHint(unixFile * pFile, i64 nByte);
  void unixModeBit(unixFile * pFile, unsigned char mask, int *pArg);
  int unixFcntlExternalReader(unixFile *, int *);
  void setDeviceCharacteristics(unixFile * pFd);
  int unixShmSystemLock(unixFile * pFile, int lockType, int ofst, int n);
  void unixShmPurge(unixFile * pFd);
  int unixLockSharedMemory(unixFile * pDbFd, unixShmNode * pShmNode);
  int unixOpenSharedMemory(unixFile * pDbFd);
  void unixRemapfile(unixFile * pFd, i64 nNew);


