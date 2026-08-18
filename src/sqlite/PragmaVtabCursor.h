
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/sqlite3_vtab_cursor.h"
#include "sqlite/sqlite_int64.h"
  typedef struct sqlite3_stmt sqlite3_stmt;

  typedef struct PragmaVtabCursor PragmaVtabCursor;
  struct PragmaVtabCursor {
    sqlite3_vtab_cursor base;
    sqlite3_stmt *pPragma;
    sqlite_int64 iRowid;
    char *azArg[2];
  };

  void pragmaVtabCursorClear(PragmaVtabCursor * pCsr);

#ifdef __cplusplus
}
#endif
