
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/i64.h"
typedef struct CallCount CallCount;

struct CallCount {
  i64 nValue;
  i64 nStep;
  i64 nTotal;
};

#ifdef __cplusplus
}
#endif
