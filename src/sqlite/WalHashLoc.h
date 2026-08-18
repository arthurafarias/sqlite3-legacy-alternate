
#pragma once

#include "sqlite/ht_slot.h"
#include "sqlite/u32.h"
  struct WalHashLoc;
  struct WalHashLoc {
    volatile ht_slot *aHash;
    volatile u32 *aPgno;
    u32 iZero;
  };


