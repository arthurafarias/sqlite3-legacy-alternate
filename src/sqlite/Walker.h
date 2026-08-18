
#pragma once

#include "sqlite/Mem.h"
#include "sqlite/u16.h"
  struct CheckOnCtx;
  struct CoveringIndexCheck;
  struct DbFixer;
  struct Expr;
  struct ExprList;
  struct IdxCover;
  struct NameContext;
  struct Parse;
  struct RefSrcList;
  struct RenameCtx;
  struct Select;
  struct SrcItem;
  struct SrcList;
  struct Table;
  struct Trigger;
  struct WhereConst;
  struct Window;
  struct WindowRewrite;

  struct Walker;

  struct Walker {
    Parse *pParse;
    int (*xExprCallback)(Walker *, Expr *);
    int (*xSelectCallback)(Walker *, Select *);
    void (*xSelectCallback2)(Walker *, Select *);
    int walkerDepth;
    u16 eCode;
    u16 mWFlags;
    union {
      NameContext *pNC;
      int n;
      int iCur;
      int sz;
      SrcList *pSrcList;
      struct CCurHint *pCCurHint;
      struct RefSrcList *pRefSrcList;
      int *aiCol;
      struct IdxCover *pIdxCover;
      ExprList *pGroupBy;
      Select *pSelect;
      struct WindowRewrite *pRewrite;
      struct WhereConst *pConst;
      struct RenameCtx *pRename;
      struct Table *pTab;
      struct CoveringIndexCheck *pCovIdxCk;
      SrcItem *pSrcItem;
      DbFixer *pFix;
      Mem *aMem;
      struct CheckOnCtx *pCheckOnCtx;
    } u;
  };

  int sqlite3WalkExpr(Walker *, Expr *);
  int sqlite3WalkExprNN(Walker *, Expr *);
  int sqlite3WalkExprList(Walker *, ExprList *);
  int sqlite3WalkSelect(Walker *, Select *);
  int sqlite3WalkSelectExpr(Walker *, Select *);
  int sqlite3WalkSelectFrom(Walker *, Select *);
  int sqlite3ExprWalkNoop(Walker *, Expr *);
  int sqlite3SelectWalkNoop(Walker *, Select *);
  int sqlite3SelectWalkFail(Walker *, Select *);
  int sqlite3WalkerDepthIncrease(Walker *, Select *);
  void sqlite3WalkerDepthDecrease(Walker *, Select *);
  void sqlite3WalkWinDefnDummyCallback(Walker *, Select *);
  void sqlite3SelectPopWith(Walker *, Select *);

  void sqlite3AggInfoPersistWalkerInit(Walker *, Parse *);
  int walkWindowList(Walker * pWalker, Window * pList, int bOneOnly);
  int incrAggDepth(Walker * pWalker, Expr * pExpr);
  int resolveExprStep(Walker * pWalker, Expr * pExpr);
  int resolveRemoveWindowsCb(Walker * pWalker, Expr * pExpr);
  int resolveSelectStep(Walker * pWalker, Select * p);
  int gatherSelectWindowsCallback(Walker * pWalker, Expr * pExpr);
  int gatherSelectWindowsSelectCallback(Walker * pWalker, Select * p);
  __attribute__((noinline)) int exprNodeIsConstantFunction(Walker * pWalker, Expr * pExpr);
  int exprNodeIsConstant(Walker * pWalker, Expr * pExpr);
  int exprSelectWalkTableConstant(Walker * pWalker, Select * pSelect);
  int exprNodeIsConstantOrGroupBy(Walker * pWalker, Expr * pExpr);
  int exprNodeCanReturnSubtype(Walker * pWalker, Expr * pExpr);
  void bothImplyNotNullRow(Walker * pWalker, Expr * pE1, Expr * pE2);
  int impliesNotNullRow(Walker * pWalker, Expr * pExpr);
  int exprIdxCover(Walker * pWalker, Expr * pExpr);
  int selectRefEnter(Walker * pWalker, Select * pSelect);
  void selectRefLeave(Walker * pWalker, Select * pSelect);
  int exprRefToSrcList(Walker * pWalker, Expr * pExpr);
  int agginfoPersistExprCb(Walker * pWalker, Expr * pExpr);
  int analyzeAggregate(Walker * pWalker, Expr * pExpr);
  int renameUnmapExprCb(Walker * pWalker, Expr * pExpr);
  void renameWalkWith(Walker * pWalker, Select * pSelect);
  int renameUnmapSelectCb(Walker * pWalker, Select * p);
  int renameColumnSelectCb(Walker * pWalker, Select * p);
  int renameColumnExprCb(Walker * pWalker, Expr * pExpr);
  void renameWalkTrigger(Walker * pWalker, Trigger * pTrigger);
  int renameTableExprCb(Walker * pWalker, Expr * pExpr);
  int renameTableSelectCb(Walker * pWalker, Select * pSelect);
  int renameQuotefixExprCb(Walker * pWalker, Expr * pExpr);
  int fixExprCb(Walker * p, Expr * pExpr);
  int fixSelectCb(Walker * p, Select * pSelect);
  int exprColumnFlagUnion(Walker * pWalker, Expr * pExpr);
  int checkConstraintExprNode(Walker * pWalker, Expr * pExpr);
  int recomputeColumnsUsedExpr(Walker * pWalker, Expr * pExpr);
  void renumberCursorDoMapping(Walker * pWalker, int *piCursor);
  int renumberCursorsCb(Walker * pWalker, Expr * pExpr);
  int propagateConstantExprRewrite(Walker * pWalker, Expr * pExpr);
  int convertCompoundSelectToSubquery(Walker * pWalker, Select * p);
  int selectExpander(Walker * pWalker, Select * p);
  void selectAddSubqueryTypeInfo(Walker * pWalker, Select * p);
  int aggregateIdxEprRefToColCallback(Walker * pWalker, Expr * pExpr);
  int havingToWhereExprCb(Walker * pWalker, Expr * pExpr);
  int selectCheckOnClausesExpr(Walker * pWalker, Expr * pExpr);
  int selectCheckOnClausesSelect(Walker * pWalker, Select * pSelect);
  int sqlite3ReturningSubqueryVarSelect(Walker * NotUsed, Expr * pExpr);
  int sqlite3ReturningSubqueryCorrelated(Walker * pWalker, Select * pSelect);
  int exprNodePatternLengthEst(Walker * pWalker, Expr * pExpr);
  int whereIsCoveringIndexWalkCallback(Walker * pWalk, Expr * pExpr);
  int exprNodeIsDeterministic(Walker * pWalker, Expr * pExpr);
  int selectWindowRewriteExprCb(Walker * pWalker, Expr * pExpr);
  int selectWindowRewriteSelectCb(Walker * pWalker, Select * pSelect);
  int sqlite3WindowExtraAggFuncDepth(Walker * pWalker, Expr * pExpr);
  int disallowAggregatesInOrderByCb(Walker * pWalker, Expr * pExpr);


