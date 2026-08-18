
#pragma once

#include "sqlite/FileChunk.h"
#include "sqlite/sqlite3_int64.h"
  struct FilePoint;

  struct FilePoint {
    sqlite3_int64 iOffset;
    FileChunk *pChunk;
  };


