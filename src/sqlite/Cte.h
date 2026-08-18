
#pragma once

#include "sqlite/u8.h"
  struct CteUse;
  struct ExprList;
  struct Select;

  struct Cte;

  struct Cte {
    char *zName;
    ExprList *pCols;
    Select *pSelect;
    const char *zCteErr;
    CteUse *pUse;
    u8 eM10d;
  };


