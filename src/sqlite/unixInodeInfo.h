
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/unixFileId.h"
  typedef struct UnixUnusedFd UnixUnusedFd;
  typedef struct sqlite3_mutex sqlite3_mutex;
  typedef struct unixShmNode unixShmNode;

  typedef struct unixInodeInfo unixInodeInfo;

  struct unixInodeInfo {
    struct unixFileId fileId;
    sqlite3_mutex *pLockMutex;
    int nShared;
    int nLock;
    unsigned char eFileLock;
    unsigned char bProcessLock;
    UnixUnusedFd *pUnused;
    int nRef;
    unixShmNode *pShmNode;
    unixInodeInfo *pNext;
    unixInodeInfo *pPrev;
  };

  extern unixInodeInfo *inodeList;

#ifdef __cplusplus
}
#endif
