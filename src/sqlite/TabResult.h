
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "sqlite/u32.h"

typedef struct TabResult TabResult;

struct TabResult {
  char **azResult;
  char *zErrMsg;
  u32 nAlloc;
  u32 nRow;
  u32 nColumn;
  u32 nData;
  int rc;
};

#ifdef __cplusplus
}
#endif
