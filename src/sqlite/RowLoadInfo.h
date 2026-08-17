
#pragma once
#ifdef __cplusplus
extern C {
#endif

#include "sqlite/u8.h"
  typedef struct RowLoadInfo RowLoadInfo;

  struct RowLoadInfo {
    int regResult;
    u8 ecelFlags;
  };

#ifdef __cplusplus
}
#endif
