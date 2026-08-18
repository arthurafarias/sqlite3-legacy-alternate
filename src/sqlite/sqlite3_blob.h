
#pragma once

#include "sqlite/sqlite3_int64.h"
#include "sqlite/u32.h"
  struct BtCursor;

  struct sqlite3_blob;

  int sqlite3_blob_reopen(sqlite3_blob *, sqlite3_int64);
  int sqlite3_blob_close(sqlite3_blob *);
  int sqlite3_blob_bytes(sqlite3_blob *);
  int sqlite3_blob_read(sqlite3_blob *, void *Z, int N, int iOffset);
  int sqlite3_blob_write(sqlite3_blob *, const void *z, int n, int iOffset);

  int blobReadWrite(sqlite3_blob * pBlob, void *z, int n, int iOffset, int (*xCall)(BtCursor *, u32, u32, void *));


