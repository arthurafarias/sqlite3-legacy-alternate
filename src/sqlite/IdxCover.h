
#pragma once

#include "sqlite/Index.h"
typedef struct IdxCover IdxCover;

struct IdxCover {
  Index *pIdx;
  int iCur;
};


