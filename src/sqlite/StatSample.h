
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/tRowcnt.h"
  typedef struct StatSample StatSample;
  struct StatSample {
    tRowcnt *anDLt;
  };

#ifdef __cplusplus
}
#endif
