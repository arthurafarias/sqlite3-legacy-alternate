
#pragma once
#ifdef __cplusplus
extern C {
#endif

#include "sqlite/u32.h"
  typedef struct SubProgram SubProgram;
  typedef struct Trigger Trigger;
  typedef struct TriggerPrg TriggerPrg;

  struct TriggerPrg {
    Trigger *pTrigger;
    TriggerPrg *pNext;
    SubProgram *pProgram;
    int orconf;
    u32 aColmask[2];
  };

#ifdef __cplusplus
}
#endif
