
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/Pgno.h"
#include "sqlite/u8.h"
  typedef struct TableLock TableLock;
  struct TableLock {
    int iDb;
    Pgno iTab;
    u8 isWriteLock;
    const char *zLockName;
  };

#ifdef __cplusplus
}
#endif
