
#pragma once

#include "sqlite/MemStore.h"
#include "sqlite/sqlite3_file.h"
  typedef struct MemFile MemFile;

  struct MemFile {
    sqlite3_file base;
    MemStore *pStore;
    int eLock;
  };


