
#pragma once
#include "sqlite/Select.h"
struct Trigger;
#include "sqlite/u8.h"
struct IdList;
struct Upsert;

  struct TriggerStep;

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


