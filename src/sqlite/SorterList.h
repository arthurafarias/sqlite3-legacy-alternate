
#pragma once

#include "sqlite/i64.h"
#include "sqlite/u8.h"
  struct SorterRecord;
  struct SorterList;

  struct SorterList {
    SorterRecord *pList;
    u8 *aMemory;
    i64 szPMA;
  };


