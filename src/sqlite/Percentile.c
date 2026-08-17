#define _GNU_SOURCE 1
#include "sqlite/Percentile.h"
#include "sqlite/i64.h"
#include "sqlite/u64.h"
i64 percentBinarySearch(Percentile *p, double y, int bExact) {
  i64 iFirst = 0;
  i64 iLast = (i64)p->nUsed - 1;
  while (iLast >= iFirst) {
    i64 iMid = (iFirst + iLast) / 2;
    double x = p->a[iMid];
    if (x < y) {
      iFirst = iMid + 1;
    } else if (x > y) {
      iLast = iMid - 1;
    } else {
      return iMid;
    }
  }
  if (bExact)
    return -1;
  return iFirst;
}
