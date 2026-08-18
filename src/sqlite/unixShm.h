
#pragma once

#include "sqlite/u16.h"
#include "sqlite/u8.h"
  typedef struct unixShmNode unixShmNode;
  typedef struct unixShm unixShm;
  struct unixShm {
    unixShmNode *pShmNode;
    unixShm *pNext;
    u8 hasMutex;
    u8 id;
    u16 sharedMask;
    u16 exclMask;
  };

  int unixShmRegionPerMap(void);


