
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/StatSample.h"
#include "sqlite/tRowcnt.h"
#include "sqlite/u8.h"
  typedef struct sqlite3 sqlite3;

  typedef struct StatAccum StatAccum;

  typedef struct StatAccum StatAccum;
  struct StatAccum {
    sqlite3 *db;
    tRowcnt nEst;
    tRowcnt nRow;
    int nLimit;
    int nCol;
    int nKeyCol;
    u8 nSkipAhead;
    StatSample current;
  };

  void statAccumDestructor(void *pOld);

#ifdef __cplusplus
}
#endif
