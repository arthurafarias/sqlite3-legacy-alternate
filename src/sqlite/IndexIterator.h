
#pragma once
#ifdef __cplusplus
extern C {
#endif

  typedef struct Index Index;
  typedef struct IndexListTerm IndexListTerm;
  typedef struct IndexIterator IndexIterator;
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

#ifdef __cplusplus
}
#endif
