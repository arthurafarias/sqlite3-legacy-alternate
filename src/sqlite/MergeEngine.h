
#pragma once

  struct PmaReader;
  struct SortSubtask;
  struct MergeEngine;
  struct MergeEngine {
    int nTree;
    SortSubtask *pTask;
    int *aTree;
    PmaReader *aReadr;
  };

  void vdbeMergeEngineFree(MergeEngine * pMerger);
  int vdbeMergeEngineStep(MergeEngine * pMerger, int *pbEof);
  void vdbeMergeEngineCompare(MergeEngine * pMerger, int iOut);


