
#pragma once

  typedef struct PmaReader PmaReader;
  typedef struct SortSubtask SortSubtask;
  typedef struct MergeEngine MergeEngine;
  struct MergeEngine {
    int nTree;
    SortSubtask *pTask;
    int *aTree;
    PmaReader *aReadr;
  };

  void vdbeMergeEngineFree(MergeEngine * pMerger);
  int vdbeMergeEngineStep(MergeEngine * pMerger, int *pbEof);
  void vdbeMergeEngineCompare(MergeEngine * pMerger, int iOut);


