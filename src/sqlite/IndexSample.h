
#pragma once

#include "sqlite/tRowcnt.h"
  struct IndexSample;

  struct IndexSample {
    void *p;
    int n;
    tRowcnt *anEq;
    tRowcnt *anLt;
    tRowcnt *anDLt;
  };


