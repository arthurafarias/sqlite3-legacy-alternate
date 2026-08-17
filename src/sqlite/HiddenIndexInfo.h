
#pragma once
#ifdef __cplusplus
extern C {
#endif

#include "sqlite/u32.h"
  typedef struct Parse Parse;
  typedef struct WhereClause WhereClause;
  typedef struct sqlite3_value sqlite3_value;

  typedef struct HiddenIndexInfo HiddenIndexInfo;
  struct HiddenIndexInfo {
    WhereClause *pWC;
    Parse *pParse;
    int eDistinct;
    u32 mIn;
    u32 mHandleIn;
    sqlite3_value *aRhs[];
  };

#ifdef __cplusplus
}
#endif
