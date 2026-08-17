
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/u32.h"
  typedef struct LookasideSlot LookasideSlot;

  struct LookasideSlot {
    LookasideSlot *pNext;
  };

  u32 countLookasideSlots(LookasideSlot * p);

#ifdef __cplusplus
}
#endif
