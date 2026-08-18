
#pragma once

#include "sqlite/BitMask.h"
#include "sqlite/WhereClause.h"
  typedef struct WhereOrInfo WhereOrInfo;
  struct WhereOrInfo {
    WhereClause wc;
    Bitmask indexable;
  };


