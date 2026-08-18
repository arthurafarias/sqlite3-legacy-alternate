
#pragma once

#include "sqlite/PragmaName.h"
#include "sqlite/sqlite3_vtab.h"
#include "sqlite/u8.h"
  struct sqlite3;

  struct PragmaVtab;
  struct PragmaVtab {
    sqlite3_vtab base;
    sqlite3 *db;
    const PragmaName *pName;
    u8 nHidden;
    u8 iHidden;
  };


