
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/u8.h"
  typedef struct Expr Expr;
  typedef struct ExprList ExprList;
  typedef struct Index Index;
  typedef struct SrcList SrcList;
  typedef struct Upsert Upsert;

  struct Upsert {
    ExprList *pUpsertTarget;
    Expr *pUpsertTargetWhere;
    ExprList *pUpsertSet;
    Expr *pUpsertWhere;
    Upsert *pNextUpsert;
    u8 isDoUpdate;
    u8 isDup;

    void *pToFree;

    Index *pUpsertIdx;
    SrcList *pUpsertSrc;
    int regData;
    int iDataCur;
    int iIdxCur;
  };

  Upsert *sqlite3UpsertOfIndex(Upsert *, Index *);
  int sqlite3UpsertNextIsIPK(Upsert *);

#ifdef __cplusplus
}
#endif
