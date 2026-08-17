
#pragma once
#ifdef __cplusplus
extern C {
#endif

#include "sqlite/u32.h"
#include "sqlite/u8.h"

  typedef struct NameContext NameContext;
  typedef struct AggInfo AggInfo;
  typedef struct ExprList ExprList;
  typedef struct Table Table;
  typedef struct Expr Expr;
  typedef struct FuncDef FuncDef;

  struct AggInfo {
    u8 directMode;

    u8 useSortingIdx;

    u32 nSortingColumn;
    int sortingIdx;
    int sortingIdxPTab;
    int iFirstReg;
    ExprList *pGroupBy;
    struct AggInfo_col {
      Table *pTab;
      Expr *pCExpr;
      int iTable;
      int iColumn;
      int iSorterColumn;
    } *aCol;
    int nColumn;
    int nAccumulator;

    struct AggInfo_func {
      Expr *pFExpr;
      FuncDef *pFunc;
      int iDistinct;
      int iDistAddr;
      int iOBTab;
      u8 bOBPayload;
      u8 bOBUnique;
      u8 bUseSubtype;
    } *aFunc;
    int nFunc;
    u32 selId;
  };

  void analyzeAggFuncArgs(AggInfo * pAggInfo, NameContext * pNC);
  void aggregateConvertIndexedExprRefToColumn(AggInfo * pAggInfo);

#ifdef __cplusplus
}
#endif
