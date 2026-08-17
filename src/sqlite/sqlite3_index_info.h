
#pragma once
#ifdef __cplusplus
extern C {
#endif

#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_uint64.h"
#include "sqlite/sqlite3_value.h"

  typedef struct sqlite3_index_info sqlite3_index_info;

  struct sqlite3_index_info {

    int nConstraint;
    struct sqlite3_index_constraint {
      int iColumn;
      unsigned char op;
      unsigned char usable;
      int iTermOffset;
    } *aConstraint;
    int nOrderBy;
    struct sqlite3_index_orderby {
      int iColumn;
      unsigned char desc;
    } *aOrderBy;

    struct sqlite3_index_constraint_usage {
      int argvIndex;
      unsigned char omit;
    } *aConstraintUsage;
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
