
#pragma once

#include "sqlite/Pgno.h"
#include "sqlite/u8.h"
  typedef struct BtLock BtLock;
  typedef struct Btree Btree;

  struct BtLock {
    Btree *pBtree;
    Pgno iTable;
    u8 eLock;
    BtLock *pNext;
  };


