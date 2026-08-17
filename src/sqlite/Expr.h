
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/AggInfo.h"
#include "sqlite/BitMask.h"
#include "sqlite/i16.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
#include "sqlite/ynVar.h"
  typedef struct Parse Parse;
  typedef struct Table Table;
  typedef struct WhereClause WhereClause;

  typedef struct ExprList ExprList;
  typedef struct Select Select;

  typedef struct Expr Expr;

  typedef struct Window Window;
  typedef struct Index Index;
  typedef struct SrcList SrcList;

  struct Expr {
    u8 op;
    char affExpr;
    u8 op2;

    u32 flags;
    union {
      char *zToken;
      int iValue;
    } u;

    Expr *pLeft;
    Expr *pRight;
    union {
      ExprList *pList;
      Select *pSelect;
    } x;

    int nHeight;

    int iTable;

    ynVar iColumn;

    i16 iAgg;
    union {
      int iJoin;
      int iOfst;
    } w;
    AggInfo *pAggInfo;
    union {
      Table *pTab;

      Window *pWin;
      int nReg;
      struct {
        int iAddr;
        int regReturn;
      } sub;
    } y;
  };

  void sqlite3DequoteExpr(Expr *);
  Expr *sqlite3ExprSimplifiedAndOr(Expr *);
  void sqlite3ExprToRegister(Expr * pExpr, int iReg);
  int sqlite3ExprCompareSkip(Expr *, Expr *, int);
  int sqlite3ExprImpliesNonNullRow(Expr *, int, int);
  int sqlite3ExprCoveredByIndex(Expr *, int iCur, Index *pIdx);
  int sqlite3ExprIdToTrueFalse(Expr *);
  int sqlite3ExprIsConstantOrFunction(Expr *, u8);
  int sqlite3ExprIsSingleTableConstraint(Expr *, const SrcList *, int, int);
  int sqlite3ExprReferencesUpdatedColumn(Expr *, int *, int);
  void sqlite3SetJoinExpr(Expr *, int, u32);
  Expr *sqlite3ExprSkipCollate(Expr *);
  Expr *sqlite3ExprSkipCollateAndLikely(Expr *);

  int sqlite3ExprTruthValue(const Expr *);
  int sqlite3ExprIsInteger(const Expr *, int *, Parse *);
  int sqlite3ExprCanBeNull(const Expr *);
  int sqlite3ExprNeedsNoAffinityChange(const Expr *, char);
  int sqlite3ExprIsLikeOperator(const Expr *);
  char sqlite3CompareAffinity(const Expr *pExpr, char aff2);
  int sqlite3IndexAffinityOk(const Expr *pExpr, char idx_affinity);
  char sqlite3ExprAffinity(const Expr *pExpr);
  int sqlite3ExprDataType(const Expr *pExpr);
  Bitmask sqlite3ExprColUsed(Expr *);
  void sqlite3ExprSetErrorOffset(Expr *, int);
  int sqlite3ExprVectorSize(const Expr *pExpr);
  int sqlite3ExprIsVector(const Expr *pExpr);
  Expr *sqlite3VectorFieldSubexpr(Expr *, int);
  void incrAggFunctionDepth(Expr * pExpr, int N);
  int exprProbability(Expr * p);
  char comparisonAffinity(const Expr *pExpr);
  u8 binaryCompareP5(const Expr *pExpr1, const Expr *pExpr2, int jumpIfNull);
  void heightOfExpr(const Expr *p, int *pnHeight);
  void exprSetHeight(Expr * p);
  int exprStructSize(const Expr *p);
  int dupedExprStructSize(const Expr *p, int flags);
  int dupedExprNodeSize(const Expr *p, int flags);
  int dupedExprSize(const Expr *p);
  int exprEvalRhsFirst(Expr * pExpr);
  int sqlite3ExprIsTableConstant(Expr * p, int iCur, int bAllowSubq);
  Select *isCandidateForInOpt(const Expr *pX);
  int sqlite3ExprIsNotTrue(Expr * pExpr);
  void sqlite3StringToId(Expr * p);
  void unsetJoinExpr(Expr * p, int iTable, int nullable);
  void updateRangeAffinityStr(Expr * pRight, int n, char *zAff);
  void whereApplyPartialIndexConstraints(Expr * pTruth, int iTabCur, WhereClause *pWC);
  void transferJoinMarkings(Expr * pDerived, Expr * pBase);
  Expr *whereRightSubexprIsColumn(Expr * p);
  int estLikePatternLength(Expr * p, u16 eCode);
  int exprIsCoveredByIndex(const Expr *pExpr, const Index *pIdx, int iTabCur);
  int exprIsDeterministic(Expr * p);

#ifdef __cplusplus
}
#endif
