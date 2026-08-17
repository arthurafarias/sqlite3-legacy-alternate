#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "sqlite/i64.h"

typedef struct FuncDef FuncDef;

typedef union MemValue MemValue;
union MemValue {
  double r;
  i64 i;
  int nZero;
  const char *zPType;
  FuncDef *pDef;
};

#ifdef __cplusplus
}
#endif
