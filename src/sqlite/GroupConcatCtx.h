
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/StrAccum.h"
typedef struct {
  StrAccum str;

  int nAccum;
  int nFirstSepLength;

  int *pnSepLengths;

} GroupConcatCtx;

#ifdef __cplusplus
}
#endif
