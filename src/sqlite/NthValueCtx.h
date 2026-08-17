
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/i64.h"
#include "sqlite/sqlite3_value.h"
typedef struct NthValueCtx NthValueCtx;

struct NthValueCtx {
  i64 nStep;
  sqlite3_value *pValue;
};

#ifdef __cplusplus
}
#endif
