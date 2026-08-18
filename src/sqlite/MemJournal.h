
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/FilePoint.h"
  typedef struct FileChunk FileChunk;
  typedef struct sqlite3_io_methods sqlite3_io_methods;
  typedef struct sqlite3_vfs sqlite3_vfs;

  typedef struct MemJournal MemJournal;

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

#ifdef __cplusplus
}
#endif
