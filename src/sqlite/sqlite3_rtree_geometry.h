
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/sqlite3_rtree_dbl.h"
  typedef struct sqlite3_rtree_geometry sqlite3_rtree_geometry;

  struct sqlite3_rtree_geometry {
    void *pContext;
    int nParam;
    sqlite3_rtree_dbl *aParam;
    void *pUser;
    void (*xDelUser)(void *);
  };

#ifdef __cplusplus
}
#endif
