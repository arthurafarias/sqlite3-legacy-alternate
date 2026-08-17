
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "sqlite/AggInfo_col.h"
#include "sqlite/AggInfo_func.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"

  typedef struct NameContext NameContext;
  typedef struct AggInfo AggInfo;
  typedef struct ExprList ExprList;

  struct AggInfo {
    u8 directMode;

    u8 useSortingIdx;

    u32 nSortingColumn;
    int sortingIdx;
    int sortingIdxPTab;
    int iFirstReg;
    ExprList *pGroupBy;
    AggInfo_col *aCol;
    int nColumn;
    int nAccumulator;

    AggInfo_func *aFunc;
    int nFunc;
    u32 selId;
  };

  void analyzeAggFuncArgs(AggInfo * pAggInfo, NameContext * pNC);
  void aggregateConvertIndexedExprRefToColumn(AggInfo * pAggInfo);

#ifdef __cplusplus
}
#endif
