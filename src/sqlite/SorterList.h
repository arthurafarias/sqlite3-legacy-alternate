
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/i64.h"
#include "sqlite/u8.h"
  typedef struct SorterRecord SorterRecord;
  typedef struct SorterList SorterList;

  struct SorterList {
    SorterRecord *pList;
    u8 *aMemory;
    i64 szPMA;
  };

#ifdef __cplusplus
}
#endif
