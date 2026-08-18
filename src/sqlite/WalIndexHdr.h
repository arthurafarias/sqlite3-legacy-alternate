
#pragma once

#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
  struct WalIndexHdr;
  struct WalIndexHdr {
    u32 iVersion;
    u32 unused;
    u32 iChange;
    u8 isInit;
    u8 bigEndCksum;
    u16 szPage;
    u32 mxFrame;
    u32 nPage;
    u32 aFrameCksum[2];
    u32 aSalt[2];
    u32 aCksum[2];
  };


