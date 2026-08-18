#pragma once

#include "sqlite/Pgno.h"
#include "sqlite/u32.h"
struct sqlite3;
struct InitData;

struct InitData {
  sqlite3 *db;
  char **pzErrMsg;
  int iDb;
  int rc;
  u32 mInitFlags;
  u32 nInitRow;
  Pgno mxPage;
};

void corruptSchema(InitData *pData, char **azObj, const char *zExtra);

