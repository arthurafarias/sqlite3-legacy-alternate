
#pragma once

#include "sqlite/SorterCompare.h"
#include "sqlite/SorterFile.h"
#include "sqlite/SorterList.h"
#include "sqlite/i64.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
  struct IncrMerger;
  struct MergeEngine;
  struct PmaReader;
  struct SQLiteThread;
  struct UnpackedRecord;
  struct VdbeSorter;

  struct SortSubtask {
    SQLiteThread *pThread;
    int bDone;
    int nPMA;
    VdbeSorter *pSorter;
    UnpackedRecord *pUnpacked;
    SorterList list;
    SorterCompare xCompare;
    SorterFile file;
    SorterFile file2;
    u64 nSpill;
  };

  int vdbeSorterMapFile(SortSubtask * pTask, SorterFile * pFile, u8 * *pp);
  int vdbePmaReaderSeek(SortSubtask * pTask, PmaReader * pReadr, SorterFile * pFile, i64 iOff);
  int vdbePmaReaderInit(SortSubtask * pTask, SorterFile * pFile, i64 iStart, PmaReader * pReadr, i64 * pnByte);
  int vdbeSorterCompareTail(SortSubtask * pTask, int *pbKey2Cached, const void *pKey1, int nKey1, const void *pKey2,
                            int nKey2);
  int vdbeSorterCompare(SortSubtask * pTask, int *pbKey2Cached, const void *pKey1, int nKey1, const void *pKey2,
                        int nKey2);
  int vdbeSorterCompareText(SortSubtask * pTask, int *pbKey2Cached, const void *pKey1, int nKey1, const void *pKey2,
                            int nKey2);
  int vdbeSorterCompareInt(SortSubtask * pTask, int *pbKey2Cached, const void *pKey1, int nKey1, const void *pKey2,
                           int nKey2);
  int vdbeSorterJoinThread(SortSubtask * pTask);
  int vdbeSorterCreateThread(SortSubtask * pTask, void *(*xTask)(void *), void *pIn);
  int vdbeSortAllocUnpacked(SortSubtask * pTask);
  SorterRecord *vdbeSorterMerge(SortSubtask * pTask, SorterRecord * p1, SorterRecord * p2);
  int vdbeSorterSort(SortSubtask * pTask, SorterList * pList);
  int vdbeSorterListToPMA(SortSubtask * pTask, SorterList * pList);
  int vdbeIncrMergerNew(SortSubtask * pTask, MergeEngine * pMerger, IncrMerger * *ppOut);
  int vdbeMergeEngineInit(SortSubtask * pTask, MergeEngine * pMerger, int eMode);
  int vdbeMergeEngineLevel0(SortSubtask * pTask, int nPMA, i64 *piOffset, MergeEngine **ppOut);
  int vdbeSorterAddToTree(SortSubtask * pTask, int nDepth, int iSeq, MergeEngine *pRoot, MergeEngine *pLeaf);


