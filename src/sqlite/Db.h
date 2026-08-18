
#pragma once

#include "sqlite/u8.h"
  struct Btree;
  struct Schema;

  struct Db;

  struct Db {
    char *zDbSName;
    Btree *pBt;
    u8 safety_level;
    u8 bSyncSet;
    Schema *pSchema;
  };


