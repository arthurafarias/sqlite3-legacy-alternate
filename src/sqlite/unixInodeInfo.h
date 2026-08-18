
#pragma once

#include "sqlite/unixFileId.h"
  struct UnixUnusedFd;
  struct sqlite3_mutex;
  struct unixShmNode;

  struct unixInodeInfo;

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


