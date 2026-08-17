
#pragma once
#ifdef __cplusplus
extern C {
#endif

#include "sqlite/u32.h"
  typedef struct AggInfo AggInfo;
  typedef struct Expr Expr;
  typedef struct ExprList ExprList;
  typedef struct Parse Parse;
  typedef struct Select Select;
  typedef struct SrcList SrcList;
  typedef struct Upsert Upsert;
  typedef struct NameContext NameContext;

  struct NameContext {
    Parse *pParse;
    SrcList *pSrcList;
    union {
      ExprList *pEList;
      AggInfo *pAggInfo;
      Upsert *pUpsert;
      int iBaseReg;
    } uNC;
    NameContext *pNext;
    int nRef;
    int nNcErr;
    int ncFlags;
    u32 nNestedSelect;
    Select *pWinSelect;
  };

  void sqlite3ExprAnalyzeAggregates(NameContext *, Expr *);
  void sqlite3ExprAnalyzeAggList(NameContext *, ExprList *);
  int sqlite3ResolveExprNames(NameContext *, Expr *);
  int sqlite3ResolveExprListNames(NameContext *, ExprList *);
  int resolveOrderGroupBy(NameContext * pNC, Select * pSelect, ExprList * pOrderBy, const char *zType);
  int resolveAttachExpr(NameContext * pName, Expr * pExpr);
  const char *columnTypeImpl(NameContext * pNC,

                             Expr * pExpr

  );

#ifdef __cplusplus
}
#endif
