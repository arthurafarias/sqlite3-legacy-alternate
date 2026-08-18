
#pragma once

#include "sqlite/i64.h"
typedef struct CallCount CallCount;

struct CallCount {
  i64 nValue;
  i64 nStep;
  i64 nTotal;
};


