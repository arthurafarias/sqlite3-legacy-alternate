#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "Expr.h"
typedef struct Expr Expr;

typedef struct FrameBound FrameBound;

struct FrameBound {
  int eType;
  Expr *pExpr;
};

#ifdef __cplusplus
}
#endif
