
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/BitMask.h"
#include "sqlite/LogEst.h"
#include "sqlite/Pgno.h"
#include "sqlite/i16.h"
#include "sqlite/u16.h"
#include "sqlite/u8.h"
  typedef struct Expr Expr;
  typedef struct ExprList ExprList;
  typedef struct Schema Schema;
  typedef struct Table Table;

  typedef struct Index Index;

  struct Index {
    char *zName;
    i16 *aiColumn;
    LogEst *aiRowLogEst;
    Table *pTable;
    char *zColAff;
    Index *pNext;
    Schema *pSchema;
    u8 *aSortOrder;
    const char **azColl;
    Expr *pPartIdxWhere;
    ExprList *aColExpr;
    Pgno tnum;
    LogEst szIdxRow;
    u16 nKeyCol;
    u16 nColumn;
    u8 onError;
    unsigned idxType : 2;
    unsigned bUnordered : 1;
    unsigned uniqNotNull : 1;
    unsigned isResized : 1;
    unsigned isCovering : 1;
    unsigned noSkipScan : 1;
    unsigned hasStat1 : 1;
    unsigned bNoQuery : 1;
    unsigned bAscKeyBug : 1;
    unsigned bHasVCol : 1;
    unsigned bHasExpr : 1;
    Bitmask colNotIdxed;
  };

  int sqlite3IndexHasDuplicateRootPage(Index *);
  int sqlite3TableColumnToIndex(Index *, int);
  void sqlite3DefaultRowEst(Index *);
  void estimateIndexWidth(Index * pIdx);
  int isDupColumn(Index * pIdx, int nKey, Index *pPk, int iCol);
  void recomputeColumnsNotIndexed(Index * pIdx);
  int xferCompatibleIndex(Index * pDest, Index * pSrc);
  int indexColumnIsBeingUpdated(Index * pIdx, int iCol, int *aXRef, int chngRowid);
  int indexWhereClauseMightChange(Index * pIdx, int *aXRef, int chngRowid);
  const char *explainIndexColumnName(Index * pIdx, int i);
  int indexColumnNotNull(Index * pIdx, int iCol);

#ifdef __cplusplus
}
#endif
