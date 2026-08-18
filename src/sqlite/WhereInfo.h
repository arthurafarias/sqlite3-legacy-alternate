
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/BitMask.h"
#include "sqlite/LogEst.h"
#include "sqlite/WhereClause.h"
#include "sqlite/WhereLevel.h"
#include "sqlite/WhereMaskSet.h"
#include "sqlite/i8.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
  typedef struct ExprList ExprList;
  typedef struct Index Index;
  typedef struct Parse Parse;
  typedef struct Select Select;
  typedef struct SrcItem SrcItem;
  typedef struct SrcList SrcList;
  typedef struct WhereLoop WhereLoop;
  typedef struct WhereMemBlock WhereMemBlock;
  typedef struct WherePath WherePath;
  typedef struct sqlite3_index_info sqlite3_index_info;

  typedef struct WhereInfo WhereInfo;

  struct WhereInfo {
    Parse *pParse;
    SrcList *pTabList;
    ExprList *pOrderBy;
    ExprList *pResultSet;

    Select *pSelect;
    int aiCurOnePass[2];
    int iContinue;
    int iBreak;
    int savedNQueryLoop;
    u16 wctrlFlags;
    LogEst iLimit;
    u8 nLevel;
    i8 nOBSat;
    u8 eOnePass;
    u8 eDistinct;
    unsigned bDeferredSeek : 1;
    unsigned untestedTerms : 1;
    unsigned bOrderedInnerLoop : 1;
    unsigned sorted : 1;
    unsigned bStarDone : 1;
    unsigned bStarUsed : 1;
    LogEst nRowOut;

    int iTop;
    int iEndWhere;
    WhereLoop *pLoops;
    WhereMemBlock *pMemToFree;
    Bitmask revMask;
    WhereClause sWC;
    WhereMaskSet sMaskSet;
    WhereLevel a[];
  };

  void sqlite3WhereEnd(WhereInfo *);
  LogEst sqlite3WhereOutputRowCount(WhereInfo *);
  int sqlite3WhereIsDistinct(WhereInfo *);
  int sqlite3WhereIsOrdered(WhereInfo *);
  int sqlite3WhereOrderByLimitOptLabel(WhereInfo *);
  int sqlite3WhereIsSorted(WhereInfo *);
  int sqlite3WhereContinueLabel(WhereInfo *);
  int sqlite3WhereBreakLabel(WhereInfo *);
  int sqlite3WhereOkOnePass(WhereInfo *, int *);
  int sqlite3WhereUsesDeferredSeek(WhereInfo *);
  void *sqlite3WhereMalloc(WhereInfo * pWInfo, u64 nByte);
  void *sqlite3WhereRealloc(WhereInfo * pWInfo, void *pOld, u64 nByte);
  __attribute__((noinline)) void sqlite3WhereRightJoinLoop(WhereInfo * pWInfo, int iLevel, WhereLevel *pLevel);
  void codeDeferredSeek(WhereInfo * pWInfo, Index * pIdx, int iCur, int iIdxCur);
  __attribute__((noinline)) void sqlite3ConstructBloomFilter(WhereInfo * pWInfo, int iLevel, WhereLevel *pLevel,
                                                             Bitmask notReady);
  sqlite3_index_info *allocateIndexInfo(WhereInfo * pWInfo, WhereClause * pWC, Bitmask mUnusable, SrcItem * pSrc,
                                        u16 * pmNoOmit);
  __attribute__((noinline)) u32 whereIsCoveringIndex(WhereInfo * pWInfo, Index * pIdx, int iTabCur);
  __attribute__((noinline)) int wherePathMatchSubqueryOB(WhereInfo * pWInfo, WhereLoop * pLoop, int iLoop, int iCur,
                                                         ExprList *pOrderBy, Bitmask *pRevMask, Bitmask *pOBSat);
  i8 wherePathSatisfiesOrderBy(WhereInfo * pWInfo, ExprList * pOrderBy, WherePath * pPath, u16 wctrlFlags, u16 nLoop,
                               WhereLoop * pLast, Bitmask * pRevMask);
  LogEst whereSortingCost(WhereInfo * pWInfo, LogEst nRow, int nOrderBy, int nSorted);
  int computeMxChoice(WhereInfo * pWInfo);
  int wherePathSolver(WhereInfo * pWInfo, LogEst nRowEst);
  __attribute__((noinline)) void whereInterstageHeuristic(WhereInfo * pWInfo);
  __attribute__((noinline)) Bitmask whereOmitNoopJoin(WhereInfo * pWInfo, Bitmask notReady);
  __attribute__((noinline)) void whereCheckIfBloomFilterIsUseful(const WhereInfo *pWInfo);
  __attribute__((noinline)) void whereReverseScanOrder(WhereInfo * pWInfo);

  LogEst estLog(LogEst N);

#ifdef __cplusplus
}
#endif
