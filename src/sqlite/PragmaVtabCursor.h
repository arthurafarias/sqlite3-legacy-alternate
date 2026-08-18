
#pragma once

#include "sqlite/sqlite3_vtab_cursor.h"
#include "sqlite/sqlite_int64.h"
  struct sqlite3_stmt;

  struct PragmaVtabCursor;
  struct PragmaVtabCursor {
    sqlite3_vtab_cursor base;
    sqlite3_stmt *pPragma;
    sqlite_int64 iRowid;
    char *azArg[2];
  };

  void pragmaVtabCursorClear(PragmaVtabCursor * pCsr);


