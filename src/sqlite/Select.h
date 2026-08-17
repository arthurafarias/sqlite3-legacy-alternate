
#pragma once
#include "sqlite/i16.h"
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/ExprList.h"
#include "sqlite/LogEst.h"
#include "sqlite/With.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
  typedef struct AggInfo AggInfo;
  typedef struct Expr Expr;
  typedef struct SrcItem SrcItem;
  typedef struct SrcList SrcList;
  typedef struct Table Table;
  typedef struct Window Window;

  typedef i16 LogEst;
  typedef struct Select Select;

  struct Select {
    u8 op;
    LogEst nSelectRow;
    u32 selFlags;
    int iLimit, iOffset;
    u32 selId;
    ExprList *pEList;
    SrcList *pSrc;
    Expr *pWhere;
    ExprList *pGroupBy;
    Expr *pHaving;
    ExprList *pOrderBy;
    Select *pPrior;
    Select *pNext;
    Expr *pLimit;
    With *pWith;

    Window *pWin;
    Window *pWinDefn;
  };

  const char *sqlite3SelectOpName(int);
  int sqlite3SelectExprHeight(const Select *);
  void windowRemoveExprFromSelect(Select * pSelect, Expr * pExpr);
  void heightOfSelect(const Select *pSelect, int *pnHeight);
  void gatherSelectWindows(Select * p);
  Select *findRightmost(Select * p);
  int hasAnchor(Select * p);
  void recomputeColumnsUsed(Select * pSelect, SrcItem * pSrcItem);
  ExprList *findLeftmostExprlist(Select * pSel);
  int compoundHasDifferentAffinities(Select * p);
  Table *isSimpleCount(Select * p, AggInfo * pAggInfo);

#ifdef __cplusplus
}
#endif
