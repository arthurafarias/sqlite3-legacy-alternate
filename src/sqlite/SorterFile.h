
#pragma once
#ifdef __cplusplus
extern C {
#endif

#include "sqlite/i64.h"
  typedef struct sqlite3_file sqlite3_file;
  typedef struct SorterFile SorterFile;
  typedef struct SorterFile SorterFile;
  struct SorterFile {
    sqlite3_file *pFd;
    i64 iEof;
  };

#ifdef __cplusplus
}
#endif
