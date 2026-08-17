
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_rtree_dbl.h"
#include "sqlite/sqlite3_value.h"
  typedef struct sqlite3_rtree_query_info sqlite3_rtree_query_info;

  struct sqlite3_rtree_query_info {
    void *pContext;
    int nParam;
    sqlite3_rtree_dbl *aParam;
    void *pUser;
    void (*xDelUser)(void *);
    sqlite3_rtree_dbl *aCoord;
    unsigned int *anQueue;
    int nCoord;
    int iLevel;
    int mxLevel;
    sqlite3_int64 iRowid;
    sqlite3_rtree_dbl rParentScore;
    int eParentWithin;
    int eWithin;
    sqlite3_rtree_dbl rScore;

    sqlite3_value **apSqlParam;
  };

#ifdef __cplusplus
}
#endif
