
#pragma once

#include "sqlite/u32.h"
  struct SubProgram;
  struct Trigger;
  struct TriggerPrg;

  struct TriggerPrg {
    Trigger *pTrigger;
    TriggerPrg *pNext;
    SubProgram *pProgram;
    int orconf;
    u32 aColmask[2];
  };


