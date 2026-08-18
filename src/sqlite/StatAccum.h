
#pragma once

#include "sqlite/StatSample.h"
#include "sqlite/tRowcnt.h"
#include "sqlite/u8.h"
  struct sqlite3;

  struct StatAccum;

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


