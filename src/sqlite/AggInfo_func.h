#pragma once

#include "sqlite/u8.h"
struct Expr;
struct FuncDef;

struct AggInfo_func;
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


