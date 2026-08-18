
#pragma once

#include "sqlite/FilePoint.h"
  struct sqlite3_io_methods;
  struct sqlite3_vfs;

  struct MemJournal;

  struct MemJournal {
    const sqlite3_io_methods *pMethod;
    int nChunkSize;

    int nSpill;
    FileChunk *pFirst;
    FilePoint endpoint;
    FilePoint readpoint;

    int flags;
    sqlite3_vfs *pVfs;
    const char *zJournal;
  };

  int memjrnlCreateFile(MemJournal * p);


