
#pragma once

#include "sqlite/u8.h"
  typedef struct Btree Btree;
  typedef struct Schema Schema;

  typedef struct Db Db;

  struct Db {
    char *zDbSName;
    Btree *pBt;
    u8 safety_level;
    u8 bSyncSet;
    Schema *pSchema;
  };


