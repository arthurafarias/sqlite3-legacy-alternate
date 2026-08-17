
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/SrcItem.h"
#include "sqlite/u32.h"
  typedef struct Expr Expr;
  typedef struct WhereClause WhereClause;
  typedef struct WhereTerm WhereTerm;

  typedef struct SrcList SrcList;

  struct SrcList {
    int nSrc;
    u32 nAlloc;
    SrcItem a[];
  };

  int tableAndColumnIndex(SrcList * pSrc, int iStart, int iEnd, const char *zCol, int *piTab, int *piCol,
                          int bIgnoreHidden);
  SrcItem *isSelfJoinView(SrcList * pTabList, SrcItem * pThis, int iFirst, int iEnd);
  void sqlite3WhereExprAnalyze(SrcList *, WhereClause *);
  void exprAnalyze(SrcList *, WhereClause *, int);
  void whereCombineDisjuncts(SrcList * pSrc, WhereClause * pWC, WhereTerm * pOne, WhereTerm * pTwo);
  void exprAnalyzeOrTerm(SrcList * pSrc, WhereClause * pWC, int idxTerm);
  __attribute__((noinline)) int exprMightBeIndexed2(SrcList * pFrom, int *aiCurCol, Expr *pExpr, int j);
  int exprMightBeIndexed(SrcList * pFrom, int *aiCurCol, Expr *pExpr, int op);

#ifdef __cplusplus
}
#endif
