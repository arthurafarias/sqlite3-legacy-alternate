
#pragma once
#ifdef __cplusplus
extern C {
#endif

#include "sqlite/ht_slot.h"
#include "sqlite/u32.h"
  typedef struct WalIterator WalIterator;
  struct WalIterator {
    u32 iPrior;
    int nSegment;
    struct WalSegment {
      int iNext;
      ht_slot *aIndex;
      u32 *aPgno;
      int nEntry;
      int iZero;
    } aSegment[];
  };

  int walIteratorNext(WalIterator * p, u32 * piPage, u32 * piFrame);
  void walIteratorFree(WalIterator * p);

#ifdef __cplusplus
}
#endif
