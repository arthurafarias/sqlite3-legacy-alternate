
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif
