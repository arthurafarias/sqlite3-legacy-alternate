#pragma once

#include "sqlite/ht_slot.h"
#include "sqlite/u32.h"
typedef struct WalSegment WalSegment;
struct WalSegment {
  int iNext;
  ht_slot *aIndex;
  u32 *aPgno;
  int nEntry;
  int iZero;
};


