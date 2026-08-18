
#pragma once

#include "sqlite/Index.h"
struct IdxCover;

struct IdxCover {
  Index *pIdx;
  int iCur;
};


