
#pragma once

  struct SorterRecord;

  struct SorterRecord {
    int nVal;
    union {
      SorterRecord *pNext;
      int iNext;
    } u;
  };


