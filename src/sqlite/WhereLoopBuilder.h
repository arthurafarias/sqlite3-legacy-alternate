
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/BitMask.h"
#include "sqlite/LogEst.h"
#include "sqlite/u16.h"
  typedef struct Index Index;
  typedef struct SrcItem SrcItem;
  typedef struct WhereClause WhereClause;
  typedef struct WhereInfo WhereInfo;
  typedef struct WhereLoop WhereLoop;
  typedef struct WhereOrSet WhereOrSet;
  typedef struct sqlite3_index_info sqlite3_index_info;
  typedef struct WhereLoopBuilder WhereLoopBuilder;

  struct WhereLoopBuilder {
    WhereInfo *pWInfo;
    WhereClause *pWC;
    WhereLoop *pNew;
    WhereOrSet *pOrSet;

    unsigned char bldFlags1;
    unsigned char bldFlags2;
    unsigned int iPlanLimit;
  };

  int whereLoopInsert(WhereLoopBuilder * pBuilder, WhereLoop * pTemplate);
  int whereLoopAddBtreeIndex(WhereLoopBuilder * pBuilder, SrcItem * pSrc, Index * pProbe, LogEst nInMul);
  int indexMightHelpWithOrderBy(WhereLoopBuilder * pBuilder, Index * pIndex, int iCursor);
  int whereLoopAddBtree(WhereLoopBuilder * pBuilder, Bitmask mPrereq);
  int whereLoopAddVirtualOne(WhereLoopBuilder * pBuilder, Bitmask mPrereq, Bitmask mUsable, u16 mExclude,
                             sqlite3_index_info * pIdxInfo, u16 mNoOmit, int *pbIn, int *pbRetryLimit);
  int whereLoopAddVirtual(WhereLoopBuilder * pBuilder, Bitmask mPrereq, Bitmask mUnusable);
  int whereLoopAddOr(WhereLoopBuilder * pBuilder, Bitmask mPrereq, Bitmask mUnusable);
  int whereLoopAddAll(WhereLoopBuilder * pBuilder);
  int whereShortCut(WhereLoopBuilder * pBuilder);

#ifdef __cplusplus
}
#endif
