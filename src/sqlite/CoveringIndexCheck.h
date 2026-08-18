
#pragma once

#include "sqlite/u8.h"
  typedef struct Index Index;

  typedef struct CoveringIndexCheck CoveringIndexCheck;
  struct CoveringIndexCheck {
    Index *pIdx;
    int iTabCur;
    u8 bExpr;
    u8 bUnidx;
  };


