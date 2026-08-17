
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/TriggerStep.h"
#include "sqlite/u8.h"
  typedef struct Table Table;

  typedef struct Expr Expr;
  typedef struct IdList IdList;
  typedef struct Schema Schema;
  typedef struct Trigger Trigger;

  struct Trigger {
    char *zName;
    char *table;
    u8 op;
    u8 tr_tm;
    u8 bReturning;
    Expr *pWhen;
    IdList *pColumns;

    Schema *pSchema;
    Schema *pTabSchema;
    TriggerStep *step_list;
    Trigger *pNext;
  };

  Table *tableOfTrigger(Trigger * pTrigger);

#ifdef __cplusplus
}
#endif
