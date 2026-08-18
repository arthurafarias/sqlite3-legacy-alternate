
#pragma once

#include "sqlite/i64.h"
#include "sqlite/u32.h"
  typedef struct JsonParent JsonParent;
  struct JsonParent {
    u32 iHead;
    u32 iValue;
    u32 iEnd;
    u32 nPath;
    i64 iKey;
  };


