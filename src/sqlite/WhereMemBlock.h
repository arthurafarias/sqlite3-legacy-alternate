
#pragma once

#include "sqlite/u64.h"
  struct WhereMemBlock;
  struct WhereMemBlock {
    WhereMemBlock *pNext;
    u64 sz;
  };


