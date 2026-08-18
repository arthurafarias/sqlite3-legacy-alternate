
#pragma once

#include "sqlite/FileChunk.h"
#include "sqlite/sqlite3_int64.h"
  typedef struct FilePoint FilePoint;

  struct FilePoint {
    sqlite3_int64 iOffset;
    FileChunk *pChunk;
  };


