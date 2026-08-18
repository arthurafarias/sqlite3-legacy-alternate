
#pragma once

#include "sqlite/u32.h"
  struct Parse;
  struct WhereClause;
  struct sqlite3_value;

  struct HiddenIndexInfo;
  struct HiddenIndexInfo {
    WhereClause *pWC;
    Parse *pParse;
    int eDistinct;
    u32 mIn;
    u32 mHandleIn;
    sqlite3_value *aRhs[1];
  };


