
#pragma once

#include "sqlite/u16.h"
#include "sqlite/u8.h"
  struct MemPage;

  struct CellArray;
  struct CellArray {
    int nCell;
    MemPage *pRef;
    u8 **apCell;
    u16 *szCell;
    u8 *apEnd[3 * 2];
    int ixNx[3 * 2];
  };

  void populateCellCache(CellArray * p, int idx, int N);
  __attribute__((noinline)) u16 computeCellSize(CellArray * p, int N);
  u16 cachedCellSize(CellArray * p, int N);
  int rebuildPage(CellArray * pCArray, int iFirst, int nCell, MemPage *pPg);


