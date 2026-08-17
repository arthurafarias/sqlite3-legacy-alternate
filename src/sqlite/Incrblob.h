
#pragma once
#ifdef __cplusplus
extern C {
#endif

#include "sqlite/sqlite3_int64.h"
#include "sqlite/u16.h"
  typedef struct BtCursor BtCursor;
  typedef struct Table Table;
  typedef struct sqlite3 sqlite3;
  typedef struct sqlite3_stmt sqlite3_stmt;
  typedef struct Incrblob Incrblob;
  struct Incrblob {
    int nByte;
    int iOffset;
    u16 iCol;
    BtCursor *pCsr;
    sqlite3_stmt *pStmt;
    sqlite3 *db;
    char *zDb;
    Table *pTab;
  };

  int blobSeekToRow(Incrblob * p, sqlite3_int64 iRow, char **pzErr);

#ifdef __cplusplus
}
#endif
