#define _GNU_SOURCE 1

#include "sqlite/WalIterator.h"

#include "sqlite/ht_slot.h"
#include "sqlite/sqlite3.h"
#include "sqlite/u32.h"
int walIteratorNext(WalIterator *p, u32 *piPage, u32 *piFrame) {
  u32 iMin;
  u32 iRet = 0xFFFFFFFF;
  int i;

  iMin = p->iPrior;

  for (i = p->nSegment - 1; i >= 0; i--) {
    struct WalSegment *pSegment = &p->aSegment[i];
    while (pSegment->iNext < pSegment->nEntry) {
      u32 iPg = pSegment->aPgno[pSegment->aIndex[pSegment->iNext]];
      if (iPg > iMin) {
        if (iPg < iRet) {
          iRet = iPg;
          *piFrame = pSegment->iZero + pSegment->aIndex[pSegment->iNext];
        }
        break;
      }
      pSegment->iNext++;
    }
  }

  *piPage = p->iPrior = iRet;
  return (iRet == 0xFFFFFFFF);
}

void walIteratorFree(WalIterator *p) { sqlite3_free(p); }
