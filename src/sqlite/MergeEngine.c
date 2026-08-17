#define _GNU_SOURCE 1

#include "sqlite/MergeEngine.h"

#include "sqlite/PmaReader.h"
#include "sqlite/SortSubtask.h"
#include "sqlite/SorterCompare.h"
#include "sqlite/UnpackedRecord.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_file.h"
#include "sqlite/u8.h"
void vdbeMergeEngineFree(MergeEngine *pMerger) {
  int i;
  if (pMerger) {
    for (i = 0; i < pMerger->nTree; i++) {
      vdbePmaReaderClear(&pMerger->aReadr[i]);
    }
  }
  sqlite3_free(pMerger);
}

int vdbeMergeEngineStep(MergeEngine *pMerger, int *pbEof) {
  int rc;
  int iPrev = pMerger->aTree[1];
  SortSubtask *pTask = pMerger->pTask;

  rc = vdbePmaReaderNext(&pMerger->aReadr[iPrev]);

  if (rc == 0) {
    int i;
    PmaReader *pReadr1;
    PmaReader *pReadr2;
    int bCached = 0;

    pReadr1 = &pMerger->aReadr[(iPrev & 0xFFFE)];
    pReadr2 = &pMerger->aReadr[(iPrev | 0x0001)];

    for (i = (pMerger->nTree + iPrev) / 2; i > 0; i = i / 2) {

      int iRes;
      if (pReadr1->pFd == 0) {
        iRes = +1;
      } else if (pReadr2->pFd == 0) {
        iRes = -1;
      } else {
        iRes = pTask->xCompare(pTask, &bCached, pReadr1->aKey, pReadr1->nKey, pReadr2->aKey, pReadr2->nKey);
      }

      if (iRes < 0 || (iRes == 0 && pReadr1 < pReadr2)) {
        pMerger->aTree[i] = (int)(pReadr1 - pMerger->aReadr);
        pReadr2 = &pMerger->aReadr[pMerger->aTree[i ^ 0x0001]];
        bCached = 0;
      } else {
        if (pReadr1->pFd)
          bCached = 0;
        pMerger->aTree[i] = (int)(pReadr2 - pMerger->aReadr);
        pReadr1 = &pMerger->aReadr[pMerger->aTree[i ^ 0x0001]];
      }
    }
    *pbEof = (pMerger->aReadr[pMerger->aTree[1]].pFd == 0);
  }

  return (rc == 0 ? pTask->pUnpacked->errCode : rc);
}

void vdbeMergeEngineCompare(MergeEngine *pMerger, int iOut) {
  int i1;
  int i2;
  int iRes;
  PmaReader *p1;
  PmaReader *p2;

  if (iOut >= (pMerger->nTree / 2)) {
    i1 = (iOut - pMerger->nTree / 2) * 2;
    i2 = i1 + 1;
  } else {
    i1 = pMerger->aTree[iOut * 2];
    i2 = pMerger->aTree[iOut * 2 + 1];
  }

  p1 = &pMerger->aReadr[i1];
  p2 = &pMerger->aReadr[i2];

  if (p1->pFd == 0) {
    iRes = i2;
  } else if (p2->pFd == 0) {
    iRes = i1;
  } else {
    SortSubtask *pTask = pMerger->pTask;
    int bCached = 0;
    int res;

    ((void)(0))

        ;
    res = pTask->xCompare(pTask, &bCached, p1->aKey, p1->nKey, p2->aKey, p2->nKey);
    if (res <= 0) {
      iRes = i1;
    } else {
      iRes = i2;
    }
  }

  pMerger->aTree[iOut] = iRes;
}
