
#pragma once
#include "sqlite/Select.h"
typedef struct Trigger Trigger;
#include "sqlite/u8.h"
typedef struct Expr Expr;
typedef struct ExprList ExprList;
typedef struct IdList IdList;
typedef struct SrcList SrcList;
typedef struct Upsert Upsert;
#ifdef __cplusplus
extern C {
#endif
  typedef struct TriggerStep TriggerStep;

  struct TriggerStep {
    u8 op;

    u8 orconf;
    Trigger *pTrig;
    Select *pSelect;
    SrcList *pSrc;
    Expr *pWhere;
    ExprList *pExprList;
    IdList *pIdList;
    Upsert *pUpsert;
    char *zSpan;
    TriggerStep *pNext;
    TriggerStep *pLast;
  };

#ifdef __cplusplus
}
#endif
