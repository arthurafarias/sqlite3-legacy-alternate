#pragma once

#include "sqlite/i64.h"
struct FuncDef;

typedef union MemValue MemValue;
union MemValue {
  double r;
  i64 i;
  int nZero;
  const char *zPType;
  FuncDef *pDef;
};


