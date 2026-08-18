
#pragma once

#include "sqlite/Pgno.h"
#include "sqlite/u8.h"
  struct TableLock;
  struct TableLock {
    int iDb;
    Pgno iTab;
    u8 isWriteLock;
    const char *zLockName;
  };


