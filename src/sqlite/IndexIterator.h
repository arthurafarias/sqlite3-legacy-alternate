
#pragma once

  struct Index;
  struct IndexListTerm;
  struct IndexIterator;
  struct IndexIterator {
    int eType;
    int i;
    union {
      struct {
        Index *pIdx;
      } lx;
      struct {
        int nIdx;
        IndexListTerm *aIdx;
      } ax;
    } u;
  };

  Index *indexIteratorFirst(IndexIterator * pIter, int *pIx);
  Index *indexIteratorNext(IndexIterator * pIter, int *pIx);


