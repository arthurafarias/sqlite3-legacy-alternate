
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "sqlite/WalSegment.h"
#include "sqlite/u32.h"
  typedef struct WalIterator WalIterator;
  struct WalIterator {
    u32 iPrior;
    int nSegment;
    WalSegment aSegment[];
  };

  int walIteratorNext(WalIterator * p, u32 * piPage, u32 * piFrame);
  void walIteratorFree(WalIterator * p);

#ifdef __cplusplus
}
#endif
