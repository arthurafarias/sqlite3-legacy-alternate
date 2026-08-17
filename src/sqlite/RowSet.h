
#pragma once
#ifdef __cplusplus
extern C {
#endif

#include "sqlite/i64.h"
#include "sqlite/u16.h"
  typedef struct RowSetChunk RowSetChunk;
  typedef struct RowSetEntry RowSetEntry;
  typedef struct sqlite3 sqlite3;
  typedef struct RowSet RowSet;

  struct RowSet {
    struct RowSetChunk *pChunk;
    sqlite3 *db;
    struct RowSetEntry *pEntry;
    struct RowSetEntry *pLast;
    struct RowSetEntry *pFresh;
    struct RowSetEntry *pForest;
    u16 nFresh;
    u16 rsFlags;
    int iBatch;
  };

  void sqlite3RowSetDelete(void *);
  void sqlite3RowSetClear(void *);
  void sqlite3RowSetInsert(RowSet *, i64);
  int sqlite3RowSetTest(RowSet *, int iBatch, i64);
  int sqlite3RowSetNext(RowSet *, i64 *);
  struct RowSetEntry *rowSetEntryAlloc(RowSet * p);

#ifdef __cplusplus
}
#endif
