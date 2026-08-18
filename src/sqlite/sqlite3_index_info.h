
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/sqlite3_index_constraint.h"
#include "sqlite/sqlite3_index_constraint_usage.h"
#include "sqlite/sqlite3_index_orderby.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_uint64.h"
#include "sqlite/sqlite3_value.h"
  typedef struct sqlite3_index_info sqlite3_index_info;

  struct sqlite3_index_info {
    int nConstraint;
    sqlite3_index_constraint *aConstraint;
    int nOrderBy;
    sqlite3_index_orderby *aOrderBy;

    sqlite3_index_constraint_usage *aConstraintUsage;
    int idxNum;
    char *idxStr;
    int needToFreeIdxStr;
    int orderByConsumed;
    double estimatedCost;

    sqlite3_int64 estimatedRows;

    int idxFlags;

    sqlite3_uint64 colUsed;
  };

  const char *sqlite3_vtab_collation(sqlite3_index_info *, int);
  int sqlite3_vtab_distinct(sqlite3_index_info *);
  int sqlite3_vtab_in(sqlite3_index_info *, int iCons, int bHandle);
  int sqlite3_vtab_rhs_value(sqlite3_index_info *, int, sqlite3_value **ppVal);

  void freeIdxStr(sqlite3_index_info * pIdxInfo);

#ifdef __cplusplus
}
#endif
