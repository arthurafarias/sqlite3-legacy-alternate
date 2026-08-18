#define _GNU_SOURCE 1
#include "sqlite/PragmaVtabCursor.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_stmt.h"
#include "sqlite/sqlite_int64.h"
void pragmaVtabCursorClear(PragmaVtabCursor *pCsr) {
  int i;
  sqlite3_finalize(pCsr->pPragma);
  pCsr->pPragma = 0;
  pCsr->iRowid = 0;
  for (i = 0; i < ((int)(sizeof(pCsr->azArg) / sizeof(pCsr->azArg[0]))); i++) {
    sqlite3_free(pCsr->azArg[i]);
    pCsr->azArg[i] = 0;
  }
}
