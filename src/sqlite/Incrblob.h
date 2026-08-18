
#pragma once

#include "sqlite/sqlite3_int64.h"
#include "sqlite/u16.h"
  struct BtCursor;
  struct Table;
  struct sqlite3;
  struct sqlite3_stmt;
  struct Incrblob;
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


