
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/tRowcnt.h"
  typedef struct IndexSample IndexSample;

  struct IndexSample {
    void *p;
    int n;
    tRowcnt *anEq;
    tRowcnt *anLt;
    tRowcnt *anDLt;
  };

#ifdef __cplusplus
}
#endif
