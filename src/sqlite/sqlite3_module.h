
#pragma once

#include "sqlite/sqlite3_int64.h"
  struct sqlite3_module;
  struct sqlite3;
  struct sqlite3_vtab;
  struct sqlite3_index_info;
  struct sqlite3_vtab_cursor;
  struct sqlite3_context;
  struct sqlite3_value;

  struct sqlite3_module {
    int iVersion;
    int (*xCreate)(sqlite3 *, void *pAux, int argc, const char *const *argv, sqlite3_vtab **ppVTab, char **);
    int (*xConnect)(sqlite3 *, void *pAux, int argc, const char *const *argv, sqlite3_vtab **ppVTab, char **);
    int (*xBestIndex)(sqlite3_vtab *pVTab, sqlite3_index_info *);
    int (*xDisconnect)(sqlite3_vtab *pVTab);
    int (*xDestroy)(sqlite3_vtab *pVTab);
    int (*xOpen)(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor);
    int (*xClose)(sqlite3_vtab_cursor *);
    int (*xFilter)(sqlite3_vtab_cursor *, int idxNum, const char *idxStr, int argc, sqlite3_value **argv);
    int (*xNext)(sqlite3_vtab_cursor *);
    int (*xEof)(sqlite3_vtab_cursor *);
    int (*xColumn)(sqlite3_vtab_cursor *, sqlite3_context *, int);
    int (*xRowid)(sqlite3_vtab_cursor *, sqlite3_int64 *pRowid);
    int (*xUpdate)(sqlite3_vtab *, int, sqlite3_value **, sqlite3_int64 *);
    int (*xBegin)(sqlite3_vtab *pVTab);
    int (*xSync)(sqlite3_vtab *pVTab);
    int (*xCommit)(sqlite3_vtab *pVTab);
    int (*xRollback)(sqlite3_vtab *pVTab);
    int (*xFindFunction)(sqlite3_vtab *pVtab, int nArg, const char *zName,
                         void (**pxFunc)(sqlite3_context *, int, sqlite3_value **), void **ppArg);
    int (*xRename)(sqlite3_vtab *pVtab, const char *zNew);

    int (*xSavepoint)(sqlite3_vtab *pVTab, int);
    int (*xRelease)(sqlite3_vtab *pVTab, int);
    int (*xRollbackTo)(sqlite3_vtab *pVTab, int);

    int (*xShadowName)(const char *);

    int (*xIntegrity)(sqlite3_vtab *pVTab, const char *zSchema, const char *zTabName, int mFlags, char **pzErr);
  };

  extern const sqlite3_module pragmaVtabModule;
  extern sqlite3_module jsonEachModule;


