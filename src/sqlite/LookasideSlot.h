
#pragma once

#include "sqlite/u32.h"
  struct LookasideSlot;

  struct LookasideSlot {
    LookasideSlot *pNext;
  };

  u32 countLookasideSlots(LookasideSlot * p);


