#define _GNU_SOURCE 1

#include "sqlite/IndexIterator.h"

#include "sqlite/Index.h"
#include "sqlite/IndexListTerm.h"
Index *indexIteratorFirst(IndexIterator *pIter, int *pIx) {

  if (pIter->eType) {
    *pIx = pIter->u.ax.aIdx[0].ix;
    return pIter->u.ax.aIdx[0].p;
  } else {
    *pIx = 0;
    return pIter->u.lx.pIdx;
  }
}

Index *indexIteratorNext(IndexIterator *pIter, int *pIx) {
  if (pIter->eType) {
    int i = ++pIter->i;
    if (i >= pIter->u.ax.nIdx) {
      *pIx = i;
      return 0;
    }
    *pIx = pIter->u.ax.aIdx[i].ix;
    return pIter->u.ax.aIdx[i].p;
  } else {
    ++(*pIx);
    pIter->u.lx.pIdx = pIter->u.lx.pIdx->pNext;
    return pIter->u.lx.pIdx;
  }
}
