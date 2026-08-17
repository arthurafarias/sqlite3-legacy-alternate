
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/Index.h"
typedef struct IdxCover IdxCover;

struct IdxCover {
  Index *pIdx;
  int iCur;
};

#ifdef __cplusplus
}
#endif
