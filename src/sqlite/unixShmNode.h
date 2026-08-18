
#pragma once

#include "sqlite/u16.h"
#include "sqlite/u8.h"
  struct sqlite3_mutex;
  struct unixInodeInfo;
  struct unixShm;
  struct unixShmNode;

  struct unixShmNode {
    unixInodeInfo *pInode;
    sqlite3_mutex *pShmMutex;
    char *zFilename;
    int hShm;
    int szRegion;
    u16 nRegion;
    u8 isReadonly;
    u8 isUnlocked;
    char **apRegion;
    int nRef;
    unixShm *pFirst;

    int aLock[8];
  };


