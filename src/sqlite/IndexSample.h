
#pragma once

#include "sqlite/tRowcnt.h"
  typedef struct IndexSample IndexSample;

  struct IndexSample {
    void *p;
    int n;
    tRowcnt *anEq;
    tRowcnt *anLt;
    tRowcnt *anDLt;
  };


