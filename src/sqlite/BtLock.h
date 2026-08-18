
#pragma once

#include "sqlite/Pgno.h"
#include "sqlite/u8.h"
  struct BtLock;
  struct Btree;

  struct BtLock {
    Btree *pBtree;
    Pgno iTable;
    u8 eLock;
    BtLock *pNext;
  };


