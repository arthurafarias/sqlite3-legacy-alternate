#define _GNU_SOURCE 1

#include "sqlite/IncrMerger.h"

#include "sqlite/MergeEngine.h"
#include "sqlite/PmaReader.h"
#include "sqlite/PmaWriter.h"
#include "sqlite/SortSubtask.h"
#include "sqlite/SorterFile.h"
#include "sqlite/Vdbe.h"
#include "sqlite/VdbeSorter.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_file.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
void vdbeIncrFree(IncrMerger *pIncr) {
  if (pIncr) {

    if (pIncr->bUseThread) {
      vdbeSorterJoinThread(pIncr->pTask);
      if (pIncr->aFile[0].pFd)
        sqlite3OsCloseFree(pIncr->aFile[0].pFd);
      if (pIncr->aFile[1].pFd)
        sqlite3OsCloseFree(pIncr->aFile[1].pFd);
    }

    vdbeMergeEngineFree(pIncr->pMerger);
    sqlite3_free(pIncr);
  }
}

int vdbeIncrPopulate(IncrMerger *pIncr) {
  int rc = 0;
  int rc2;
  i64 iStart = pIncr->iStartOff;
  SorterFile *pOut = &pIncr->aFile[1];
  SortSubtask *pTask = pIncr->pTask;
  MergeEngine *pMerger = pIncr->pMerger;
  PmaWriter writer;

  ;

  vdbePmaWriterInit(pOut->pFd, &writer, pTask->pSorter->pgsz, iStart);
  while (rc == 0) {
    int dummy;
    PmaReader *pReader = &pMerger->aReadr[pMerger->aTree[1]];
    int nKey = pReader->nKey;
    i64 iEof = writer.iWriteOff + writer.iBufEnd;

    if (pReader->pFd == 0)
      break;
    if ((iEof + nKey + sqlite3VarintLen(nKey)) > (iStart + pIncr->mxSz))
      break;

    vdbePmaWriteVarint(&writer, nKey);
    vdbePmaWriteBlob(&writer, pReader->aKey, nKey);

    ((void)(0))

        ;
    rc = vdbeMergeEngineStep(pIncr->pMerger, &dummy);
  }

  rc2 = vdbePmaWriterFinish(&writer, &pOut->iEof, &pTask->nSpill);
  if (rc == 0)
    rc = rc2;
  ;
  return rc;
}

int vdbeIncrBgPopulate(IncrMerger *pIncr) {
  void *p = (void *)pIncr;

  return vdbeSorterCreateThread(pIncr->pTask, vdbeIncrPopulateThread, p);
}

int vdbeIncrSwap(IncrMerger *pIncr) {
  int rc = 0;

  if (pIncr->bUseThread) {
    rc = vdbeSorterJoinThread(pIncr->pTask);

    if (rc == 0) {
      SorterFile f0 = pIncr->aFile[0];
      pIncr->aFile[0] = pIncr->aFile[1];
      pIncr->aFile[1] = f0;
    }

    if (rc == 0) {
      if (pIncr->aFile[0].iEof == pIncr->iStartOff) {
        pIncr->bEof = 1;
      } else {
        rc = vdbeIncrBgPopulate(pIncr);
      }
    }
  } else

  {
    rc = vdbeIncrPopulate(pIncr);
    pIncr->aFile[0] = pIncr->aFile[1];
    if (pIncr->aFile[0].iEof == pIncr->iStartOff) {
      pIncr->bEof = 1;
    }
  }

  return rc;
}

void vdbeIncrMergerSetThreads(IncrMerger *pIncr) {
  pIncr->bUseThread = 1;
  pIncr->pTask->file2.iEof -= pIncr->mxSz;
}
