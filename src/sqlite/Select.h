
#pragma once
#include "sqlite/i16.h"

#include "sqlite/ExprList.h"
#include "sqlite/LogEst.h"
#include "sqlite/With.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
  struct AggInfo;
  struct SrcList;
  struct Window;

  typedef i16 LogEst;

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


