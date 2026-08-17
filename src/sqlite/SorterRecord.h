
#pragma once
#ifdef __cplusplus
extern C {
#endif

  typedef struct SorterRecord SorterRecord;

  struct SorterRecord {
    int nVal;
    union {
      SorterRecord *pNext;
      int iNext;
    } u;
  };

#ifdef __cplusplus
}
#endif
