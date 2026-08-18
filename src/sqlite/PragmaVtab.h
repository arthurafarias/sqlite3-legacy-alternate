
#pragma once

#include "sqlite/PragmaName.h"
#include "sqlite/sqlite3_vtab.h"
#include "sqlite/u8.h"
  typedef struct sqlite3 sqlite3;

  typedef struct PragmaVtab PragmaVtab;
  struct PragmaVtab {
    sqlite3_vtab base;
    sqlite3 *db;
    const PragmaName *pName;
    u8 nHidden;
    u8 iHidden;
  };


