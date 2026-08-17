
#pragma once
#ifdef __cplusplus
extern C {
#endif

  typedef struct Returning Returning;

#include "sqlite/Trigger.h"
#include "sqlite/TriggerStep.h"
  typedef struct ExprList ExprList;
  typedef struct Parse Parse;

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

#ifdef __cplusplus
}
#endif
