
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/SortSubtask.h"
#include "sqlite/SorterCompare.h"
#include "sqlite/SorterList.h"
#include "sqlite/u8.h"
  typedef struct KeyInfo KeyInfo;
  typedef struct MergeEngine MergeEngine;
  typedef struct PmaReader PmaReader;
  typedef struct UnpackedRecord UnpackedRecord;
  typedef struct sqlite3 sqlite3;

  struct VdbeSorter {
    int mnPmaSize;
    int mxPmaSize;
    int mxKeysize;
    int pgsz;
    PmaReader *pReader;
    MergeEngine *pMerger;
    sqlite3 *db;
    KeyInfo *pKeyInfo;
    UnpackedRecord *pUnpacked;
    SorterList list;
    int iMemory;
    int nMemory;
    u8 bUsePMA;
    u8 bUseThreads;
    u8 iPrev;
    u8 nTask;
    u8 typeMask;
    SortSubtask aTask[];
  };

  int vdbeSorterJoinAll(VdbeSorter * pSorter, int rcin);
  SorterCompare vdbeSorterGetCompare(VdbeSorter * p);
  void *vdbeSorterFlushThread(void *pCtx);
  int vdbeSorterFlushPMA(VdbeSorter * pSorter);
  int vdbeSorterMergeTreeBuild(VdbeSorter * pSorter, MergeEngine * *ppOut);
  int vdbeSorterSetupMerge(VdbeSorter * pSorter);
  void *vdbeSorterRowkey(const VdbeSorter *pSorter, int *pnKey);

#ifdef __cplusplus
}
#endif
