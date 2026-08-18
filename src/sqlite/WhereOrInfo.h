
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/BitMask.h"
#include "sqlite/WhereClause.h"
  typedef struct WhereOrInfo WhereOrInfo;
  struct WhereOrInfo {
    WhereClause wc;
    Bitmask indexable;
  };

#ifdef __cplusplus
}
#endif
