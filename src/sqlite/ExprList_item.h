#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/u16.h"
#include "sqlite/u8.h"
typedef struct Expr Expr;

typedef struct ExprList_item ExprList_item;
struct ExprList_item {
  Expr *pExpr;
  char *zEName;
  struct {
    u8 sortFlags;
    unsigned eEName : 2;
    unsigned done : 1;
    unsigned reusable : 1;
    unsigned bSorterRef : 1;
    unsigned bNulls : 1;
    unsigned bUsed : 1;
    unsigned bUsingTerm : 1;
    unsigned bNoExpand : 1;

  } fg;
  union {
    struct {
      u16 iOrderByCol;
      u16 iAlias;
    } x;
    int iConstExprReg;

  } u;
};

#ifdef __cplusplus
}
#endif
