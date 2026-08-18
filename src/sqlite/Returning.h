
#pragma once

  struct Returning;

#include "sqlite/Trigger.h"
#include "sqlite/TriggerStep.h"
  struct Parse;

  struct Returning {
    Parse *pParse;
    ExprList *pReturnEL;
    Trigger retTrig;
    TriggerStep retTStep;
    int iRetCur;
    int nRetCol;
    int iRetReg;
    char zName[40];
  };


