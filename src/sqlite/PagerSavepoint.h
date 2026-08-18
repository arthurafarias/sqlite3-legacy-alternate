
#pragma once

#include "sqlite/Pgno.h"
#include "sqlite/i64.h"
#include "sqlite/u32.h"
  typedef struct Bitvec Bitvec;

  typedef struct PagerSavepoint PagerSavepoint;
  struct PagerSavepoint {
    i64 iOffset;
    i64 iHdrOffset;
    Bitvec *pInSavepoint;
    Pgno nOrig;
    Pgno iSubRec;
    int bTruncateOnRelease;

    u32 aWalData[4];
  };


