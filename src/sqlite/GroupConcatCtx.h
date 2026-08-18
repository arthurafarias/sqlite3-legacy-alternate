
#pragma once

#include "sqlite/StrAccum.h"
typedef struct {
  StrAccum str;

  int nAccum;
  int nFirstSepLength;

  int *pnSepLengths;

} GroupConcatCtx;


