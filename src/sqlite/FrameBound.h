#pragma once

#include "Expr.h"
typedef struct Expr Expr;

typedef struct FrameBound FrameBound;

struct FrameBound {
  int eType;
  Expr *pExpr;
};


