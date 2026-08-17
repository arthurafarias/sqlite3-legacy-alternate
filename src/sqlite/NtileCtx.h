
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/i64.h"
typedef struct NtileCtx NtileCtx;

struct NtileCtx {
  i64 nTotal;
  i64 nParam;
  i64 iRow;
};

#ifdef __cplusplus
}
#endif
