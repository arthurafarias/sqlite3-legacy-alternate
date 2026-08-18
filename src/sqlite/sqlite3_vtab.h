
#pragma once

  typedef struct sqlite3_index_info sqlite3_index_info;
  typedef struct sqlite3_vtab_cursor sqlite3_vtab_cursor;
  typedef struct sqlite3_vtab sqlite3_vtab;
  typedef struct sqlite3_module sqlite3_module;

  struct sqlite3_vtab {
    const sqlite3_module *pModule;
    int nRef;
    char *zErrMsg;
  };

  int pragmaVtabDisconnect(sqlite3_vtab * pVtab);
  int pragmaVtabBestIndex(sqlite3_vtab * tab, sqlite3_index_info * pIdxInfo);
  int pragmaVtabOpen(sqlite3_vtab * pVtab, sqlite3_vtab_cursor * *ppCursor);
  int jsonEachDisconnect(sqlite3_vtab * pVtab);
  int jsonEachOpen(sqlite3_vtab * p, sqlite3_vtab_cursor * *ppCursor);
  int jsonEachBestIndex(sqlite3_vtab * tab, sqlite3_index_info * pIdxInfo);


