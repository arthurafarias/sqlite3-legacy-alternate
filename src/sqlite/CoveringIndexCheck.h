
#pragma once

#include "sqlite/u8.h"
  struct Index;

  struct CoveringIndexCheck;
  struct CoveringIndexCheck {
    Index *pIdx;
    int iTabCur;
    u8 bExpr;
    u8 bUnidx;
  };


