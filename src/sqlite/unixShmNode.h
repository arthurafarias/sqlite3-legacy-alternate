
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/u16.h"
#include "sqlite/u8.h"
  typedef struct sqlite3_mutex sqlite3_mutex;
  typedef struct unixInodeInfo unixInodeInfo;
  typedef struct unixShm unixShm;
  typedef struct unixShmNode unixShmNode;

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

#ifdef __cplusplus
}
#endif
