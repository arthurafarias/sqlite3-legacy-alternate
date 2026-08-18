
#pragma once

#include "sqlite/u32.h"
#include "sqlite/u8.h"
  struct WalCkptInfo;
  struct WalCkptInfo {
    u32 nBackfill;
    u32 aReadMark[(8 - 3)];
    u8 aLock[8];
    u32 nBackfillAttempted;
    u32 notUsed0;
  };


