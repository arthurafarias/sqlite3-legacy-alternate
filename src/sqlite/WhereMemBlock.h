
#pragma once

#include "sqlite/u64.h"
  typedef struct WhereMemBlock WhereMemBlock;
  struct WhereMemBlock {
    WhereMemBlock *pNext;
    u64 sz;
  };


