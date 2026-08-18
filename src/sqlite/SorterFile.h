
#pragma once

#include "sqlite/i64.h"
  struct sqlite3_file;
  struct SorterFile;
  struct SorterFile {
    sqlite3_file *pFd;
    i64 iEof;
  };


