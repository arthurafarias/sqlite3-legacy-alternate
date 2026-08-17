
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/i64.h"
  typedef struct CountCtx CountCtx;
  struct CountCtx {
    i64 n;
  };

#ifdef __cplusplus
}
#endif
