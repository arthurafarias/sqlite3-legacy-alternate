#define _GNU_SOURCE 1
#include <stdint.h>
#include "sqlite/VdbeSorter.h"
#include "sqlite/IncrMerger.h"
#include "sqlite/MergeEngine.h"
#include "sqlite/PmaReader.h"
#include "sqlite/SQLiteThread.h"
#include "sqlite/SortSubtask.h"
#include "sqlite/SorterCompare.h"
#include "sqlite/SorterList.h"
#include "sqlite/SorterRecord.h"
#include "sqlite/Vdbe.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/SqliteResultCode.h"
int vdbeSorterJoinAll(VdbeSorter *pSorter, int rcin) {
  int rc = rcin;
  int i;

  for (i = pSorter->nTask - 1; i >= 0; i--) {
    SortSubtask *pTask = &pSorter->aTask[i];
    int rc2 = vdbeSorterJoinThread(pTask);
    if (rc == SQLITE_OK)
      rc = rc2;
  }
  return rc;
}

SorterCompare vdbeSorterGetCompare(VdbeSorter *p) {
  if (p->typeMask == 0x01) {
    return vdbeSorterCompareInt;
  } else if (p->typeMask == 0x02) {
    return vdbeSorterCompareText;
  }
  return vdbeSorterCompare;
}

void *vdbeSorterFlushThread(void *pCtx) {
  SortSubtask *pTask = (SortSubtask *)pCtx;
  int rc;

  rc = vdbeSorterListToPMA(pTask, &pTask->list);
  pTask->bDone = 1;
  return ((void *)(intptr_t)(rc));
}

int vdbeSorterFlushPMA(VdbeSorter *pSorter) {
  int rc = SQLITE_OK;
  int i;
  SortSubtask *pTask = 0;
  int nWorker = (pSorter->nTask - 1);

  pSorter->bUsePMA = 1;

  for (i = 0; i < nWorker; i++) {
    int iTest = (pSorter->iPrev + i + 1) % nWorker;
    pTask = &pSorter->aTask[iTest];
    if (pTask->bDone) {
      rc = vdbeSorterJoinThread(pTask);
    }
    if (rc != SQLITE_OK || pTask->pThread == 0)
      break;
  }

  if (rc == SQLITE_OK) {
    if (i == nWorker) {
      rc = vdbeSorterListToPMA(&pSorter->aTask[nWorker], &pSorter->list);
    } else {
      u8 *aMem;
      void *pCtx;

      aMem = pTask->list.aMemory;
      pCtx = (void *)pTask;
      pSorter->iPrev = (u8)(pTask - pSorter->aTask);
      pTask->list = pSorter->list;
      pSorter->list.pList = 0;
      pSorter->list.szPMA = 0;
      if (aMem) {
        pSorter->list.aMemory = aMem;
        pSorter->nMemory = sqlite3MallocSize(aMem);
      } else if (pSorter->list.aMemory) {
        pSorter->list.aMemory = (u8*)(sqlite3Malloc(pSorter->nMemory));
        if (!pSorter->list.aMemory)
          return 7;
      }

      rc = vdbeSorterCreateThread(pTask, vdbeSorterFlushThread, pCtx);
    }
  }

  return rc;
}

static int vdbeSorterTreeDepth(int nPMA) {
  int nDepth = 0;
  i64 nDiv = 16;
  while (nDiv < (i64)nPMA) {
    nDiv = nDiv * 16;
    nDepth++;
  }
  return nDepth;
}

int vdbeSorterMergeTreeBuild(VdbeSorter *pSorter, MergeEngine **ppOut) {
  MergeEngine *pMain = 0;
  int rc = SQLITE_OK;
  int iTask;

  if (pSorter->nTask > 1) {
    pMain = vdbeMergeEngineNew(pSorter->nTask);
    if (pMain == 0)
      rc = 7;
  }

  for (iTask = 0; rc == SQLITE_OK && iTask < pSorter->nTask; iTask++) {
    SortSubtask *pTask = &pSorter->aTask[iTask];

    if (8 == 0 || pTask->nPMA) {
      MergeEngine *pRoot = 0;
      int nDepth = vdbeSorterTreeDepth(pTask->nPMA);
      i64 iReadOff = 0;

      if (pTask->nPMA <= 16) {
        rc = vdbeMergeEngineLevel0(pTask, pTask->nPMA, &iReadOff, &pRoot);
      } else {
        int i;
        int iSeq = 0;
        pRoot = vdbeMergeEngineNew(16);
        if (pRoot == 0)
          rc = 7;
        for (i = 0; i < pTask->nPMA && rc == SQLITE_OK; i += 16) {
          MergeEngine *pMerger = 0;
          int nReader;

          nReader = ((pTask->nPMA - i) < (16) ? (pTask->nPMA - i) : (16));
          rc = vdbeMergeEngineLevel0(pTask, nReader, &iReadOff, &pMerger);
          if (rc == SQLITE_OK) {
            rc = vdbeSorterAddToTree(pTask, nDepth, iSeq++, pRoot, pMerger);
          }
        }
      }

      if (rc == SQLITE_OK) {
        if (pMain != 0) {
          rc = vdbeIncrMergerNew(pTask, pRoot, &pMain->aReadr[iTask].pIncr);
        } else {
          pMain = pRoot;
        }
      } else {
        vdbeMergeEngineFree(pRoot);
      }
    }
  }

  if (rc != SQLITE_OK) {
    vdbeMergeEngineFree(pMain);
    pMain = 0;
  }
  *ppOut = pMain;
  return rc;
}

int vdbeSorterSetupMerge(VdbeSorter *pSorter) {
  int rc;
  SortSubtask *pTask0 = &pSorter->aTask[0];
  MergeEngine *pMain = 0;

  sqlite3 *db = pTask0->pSorter->db;
  int i;
  SorterCompare xCompare = vdbeSorterGetCompare(pSorter);
  for (i = 0; i < pSorter->nTask; i++) {
    pSorter->aTask[i].xCompare = xCompare;
  }

  rc = vdbeSorterMergeTreeBuild(pSorter, &pMain);
  if (rc == SQLITE_OK) {
    if (pSorter->bUseThreads) {
      int iTask;
      PmaReader *pReadr = 0;
      SortSubtask *pLast = &pSorter->aTask[pSorter->nTask - 1];
      rc = vdbeSortAllocUnpacked(pLast);
      if (rc == SQLITE_OK) {
        pReadr = (PmaReader *)sqlite3DbMallocZero(db, sizeof(PmaReader));
        pSorter->pReader = pReadr;
        if (pReadr == 0)
          rc = 7;
      }
      if (rc == SQLITE_OK) {
        rc = vdbeIncrMergerNew(pLast, pMain, &pReadr->pIncr);
        if (rc == SQLITE_OK) {
          vdbeIncrMergerSetThreads(pReadr->pIncr);
          for (iTask = 0; iTask < (pSorter->nTask - 1); iTask++) {
            IncrMerger *pIncr;
            if ((pIncr = pMain->aReadr[iTask].pIncr)) {
              vdbeIncrMergerSetThreads(pIncr);
            }
          }
          for (iTask = 0; rc == SQLITE_OK && iTask < pSorter->nTask; iTask++) {
            PmaReader *p = &pMain->aReadr[iTask];

            rc = vdbePmaReaderIncrInit(p, 1);
          }
        }
        pMain = 0;
      }
      if (rc == SQLITE_OK) {
        rc = vdbePmaReaderIncrMergeInit(pReadr, 2);
      }
    } else {
      rc = vdbeMergeEngineInit(pTask0, pMain, 0);
      pSorter->pMerger = pMain;
      pMain = 0;
    }
  }

  if (rc != SQLITE_OK) {
    vdbeMergeEngineFree(pMain);
  }
  return rc;
}

void *vdbeSorterRowkey(const VdbeSorter *pSorter, int *pnKey) {
  void *pKey;
  if (pSorter->bUsePMA) {
    PmaReader *pReader;

    if (pSorter->bUseThreads) {
      pReader = pSorter->pReader;
    } else {
      pReader = &pSorter->pMerger->aReadr[pSorter->pMerger->aTree[1]];
    }
    *pnKey = pReader->nKey;
    pKey = pReader->aKey;
  } else {
    *pnKey = pSorter->list.pList->nVal;
    pKey = ((void *)((SorterRecord *)(pSorter->list.pList) + 1));
  }
  return pKey;
}
