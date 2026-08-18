
#pragma once

#include "sqlite/u8.h"
  typedef struct ExprList ExprList;
  typedef struct SelectDest SelectDest;

  struct SelectDest {
    u8 eDest;
    int iSDParm;
    int iSDParm2;
    int iSdst;
    int nSdst;
    char *zAffSdst;
    ExprList *pOrderBy;
  };

  void sqlite3SelectDestInit(SelectDest *, int, int);


