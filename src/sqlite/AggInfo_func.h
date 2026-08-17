#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/u8.h"
typedef struct Expr Expr;
typedef struct FuncDef FuncDef;

typedef struct AggInfo_func AggInfo_func;
struct AggInfo_func {
  Expr *pFExpr;
  FuncDef *pFunc;
  int iDistinct;
  int iDistAddr;
  int iOBTab;
  u8 bOBPayload;
  u8 bOBUnique;
  u8 bUseSubtype;
};

#ifdef __cplusplus
}
#endif
