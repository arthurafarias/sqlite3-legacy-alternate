
#pragma once

#include "sqlite/BitMask.h"
#include "sqlite/WhereClause.h"
  struct WhereOrInfo {
    WhereClause wc;
    Bitmask indexable;
  };


