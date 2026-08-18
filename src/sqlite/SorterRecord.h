
#pragma once

  typedef struct SorterRecord SorterRecord;

  struct SorterRecord {
    int nVal;
    union {
      SorterRecord *pNext;
      int iNext;
    } u;
  };


