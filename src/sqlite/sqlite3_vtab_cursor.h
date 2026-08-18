
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/sqlite_int64.h"
  typedef struct sqlite3_context sqlite3_context;
  typedef struct sqlite3_value sqlite3_value;

  typedef struct sqlite3_vtab_cursor sqlite3_vtab_cursor;
  typedef struct sqlite3_vtab sqlite3_vtab;

  struct sqlite3_vtab_cursor {
    sqlite3_vtab *pVtab;
  };

  int pragmaVtabClose(sqlite3_vtab_cursor * cur);
  int pragmaVtabNext(sqlite3_vtab_cursor * pVtabCursor);
  int pragmaVtabFilter(sqlite3_vtab_cursor * pVtabCursor, int idxNum, const char *idxStr, int argc,
                       sqlite3_value **argv);
  int pragmaVtabEof(sqlite3_vtab_cursor * pVtabCursor);
  int pragmaVtabColumn(sqlite3_vtab_cursor * pVtabCursor, sqlite3_context * ctx, int i);
  int pragmaVtabRowid(sqlite3_vtab_cursor * pVtabCursor, sqlite_int64 * p);
  int jsonEachClose(sqlite3_vtab_cursor * cur);
  int jsonEachEof(sqlite3_vtab_cursor * cur);
  int jsonEachNext(sqlite3_vtab_cursor * cur);
  int jsonEachColumn(sqlite3_vtab_cursor * cur, sqlite3_context * ctx, int iColumn);
  int jsonEachRowid(sqlite3_vtab_cursor * cur, sqlite_int64 * pRowid);
  int jsonEachFilter(sqlite3_vtab_cursor * cur, int idxNum, const char *idxStr, int argc, sqlite3_value **argv);

#ifdef __cplusplus
}
#endif
