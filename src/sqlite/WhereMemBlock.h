
#pragma once
#ifdef __cplusplus
extern C {
#endif

#include "sqlite/u64.h"
  typedef struct WhereMemBlock WhereMemBlock;
  struct WhereMemBlock {
    WhereMemBlock *pNext;
    u64 sz;
  };

#ifdef __cplusplus
}
#endif
