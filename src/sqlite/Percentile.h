
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/i64.h"
#include "sqlite/u64.h"
typedef struct Percentile Percentile;

struct Percentile {
  u64 nAlloc;
  u64 nUsed;
  char bSorted;
  char bKeepSorted;
  char bPctValid;
  double rPct;
  double *a;
};

i64 percentBinarySearch(Percentile *p, double y, int bExact);

#ifdef __cplusplus
}
#endif
