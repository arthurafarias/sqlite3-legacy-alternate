
#pragma once

#include "sqlite/SrcList.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
typedef struct RefSrcList RefSrcList;

struct RefSrcList {
  sqlite3 *db;
  SrcList *pRef;
  i64 nExclude;
  int *aiExclude;
};


