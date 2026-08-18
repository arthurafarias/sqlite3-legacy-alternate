
#pragma once

#include "sqlite/u32.h"
struct TabResult;

struct TabResult {
  char **azResult;
  char *zErrMsg;
  u32 nAlloc;
  u32 nRow;
  u32 nColumn;
  u32 nData;
  int rc;
};


