#define _GNU_SOURCE 1
#include <string.h>
#include "sqlite/WhereOrSet.h"
#include "sqlite/LogEst.h"
#include "sqlite/WhereOrCost.h"
#include "sqlite/u16.h"
void whereOrMove(WhereOrSet *pDest, WhereOrSet *pSrc) {
  pDest->n = pSrc->n;
  memcpy(pDest->a, pSrc->a, pDest->n * sizeof(pDest->a[0]));
}

int whereOrInsert(WhereOrSet *pSet, Bitmask prereq, LogEst rRun, LogEst nOut) {
  u16 i;
  WhereOrCost *p;
  for (i = pSet->n, p = pSet->a; i > 0; i--, p++) {
    if (rRun <= p->rRun && (prereq & p->prereq) == prereq) {
      goto whereOrInsert_done;
    }
    if (p->rRun <= rRun && (p->prereq & prereq) == p->prereq) {
      return 0;
    }
  }
  if (pSet->n < 3) {
    p = &pSet->a[pSet->n++];
    p->nOut = nOut;
  } else {
    p = pSet->a;
    for (i = 1; i < pSet->n; i++) {
      if (p->rRun > pSet->a[i].rRun)
        p = pSet->a + i;
    }
    if (p->rRun <= rRun)
      return 0;
  }
whereOrInsert_done:
  p->prereq = prereq;
  p->rRun = rRun;
  if (p->nOut > nOut)
    p->nOut = nOut;
  return 1;
}
