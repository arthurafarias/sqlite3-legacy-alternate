
#pragma once

#include "sqlite/WalSegment.h"
#include "sqlite/u32.h"
struct WalIterator;
struct WalIterator {
  u32 iPrior;
  int nSegment;
  WalSegment aSegment[1];
};

int walIteratorNext(WalIterator *p, u32 *piPage, u32 *piFrame);
void walIteratorFree(WalIterator *p);


