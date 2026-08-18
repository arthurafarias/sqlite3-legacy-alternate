
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/BitMask.h"
#include "sqlite/LogEst.h"
#include "sqlite/u16.h"
#include "sqlite/u8.h"
  typedef struct Expr Expr;
  typedef struct SrcItem SrcItem;
  typedef struct WhereAndInfo WhereAndInfo;
  typedef struct WhereClause WhereClause;
  typedef struct WhereOrInfo WhereOrInfo;

  typedef struct WhereTerm WhereTerm;
  struct WhereTerm {
    Expr *pExpr;
    WhereClause *pWC;
    LogEst truthProb;
    u16 wtFlags;
    u16 eOperator;
    u8 nChild;
    u8 eMatchOp;
    int iParent;
    int leftCursor;

    union {
      struct {
        int leftColumn;
        int iField;
      } x;
      WhereOrInfo *pOrInfo;
      WhereAndInfo *pAndInfo;
    } u;
    Bitmask prereqRight;
    Bitmask prereqAll;
  };

  WhereTerm *whereNthSubterm(WhereTerm * pTerm, int N);
  int constraintCompatibleWithOuterJoin(const WhereTerm *pTerm, const SrcItem *pSrc);
  int termCanDriveIndex(const WhereTerm *pTerm, const SrcItem *pSrc, const Bitmask notReady);
  LogEst whereRangeAdjust(WhereTerm * pTerm, LogEst nNew);
  int isLimitTerm(WhereTerm * pTerm);

#ifdef __cplusplus
}
#endif
