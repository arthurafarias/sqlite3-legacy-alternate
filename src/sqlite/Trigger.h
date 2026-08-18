
#pragma once

#include "sqlite/TriggerStep.h"
#include "sqlite/u8.h"

  struct Schema;

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


