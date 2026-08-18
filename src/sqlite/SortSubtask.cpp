#define _GNU_SOURCE 1
#include <stdint.h>
#include <string.h>
#include "sqlite/SortSubtask.h"
#include "sqlite/IncrMerger.h"
#include "sqlite/KeyInfo.h"
#include "sqlite/MergeEngine.h"
#include "sqlite/PmaReader.h"
#include "sqlite/PmaWriter.h"
#include "sqlite/SQLiteThread.h"
#include "sqlite/SorterCompare.h"
#include "sqlite/SorterFile.h"
#include "sqlite/SorterList.h"
#include "sqlite/SorterRecord.h"
#include "sqlite/UnpackedRecord.h"
#include "sqlite/Vdbe.h"
#include "sqlite/VdbeSorter.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_file.h"
#include "sqlite/sqlite3_io_methods.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/SqliteResultCode.h"
int vdbeSorterMapFile(SortSubtask *pTask, SorterFile *pFile, u8 **pp) {
  int rc = SQLITE_OK;
  if (pFile->iEof <= (i64)(pTask->pSorter->db->nMaxSorterMmap)) {
    sqlite3_file *pFd = pFile->pFd;
    if (pFd->pMethods->iVersion >= 3) {
      rc = sqlite3OsFetch(pFd, 0, (int)pFile->iEof, (void **)pp);
    }
  }
  return rc;
}

int vdbePmaReaderSeek(SortSubtask *pTask, PmaReader *pReadr, SorterFile *pFile, i64 iOff) {
  int rc = 0;

  if (sqlite3FaultSim(201))
    return (10 | (1 << 8));
  if (pReadr->aMap) {
    sqlite3OsUnfetch(pReadr->pFd, 0, pReadr->aMap);
    pReadr->aMap = 0;
  }
  pReadr->iReadOff = iOff;
  pReadr->iEof = pFile->iEof;
  pReadr->pFd = pFile->pFd;

  rc = vdbeSorterMapFile(pTask, pFile, &pReadr->aMap);
  if (rc == SQLITE_OK && pReadr->aMap == 0) {
    int pgsz = pTask->pSorter->pgsz;
    int iBuf = pReadr->iReadOff % pgsz;
    if (pReadr->aBuffer == 0) {
      pReadr->aBuffer = (u8 *)sqlite3Malloc(pgsz);
      if (pReadr->aBuffer == 0)
        rc = 7;
      pReadr->nBuffer = pgsz;
    }
    if (rc == SQLITE_OK && iBuf) {
      int nRead = pgsz - iBuf;
      if ((pReadr->iReadOff + nRead) > pReadr->iEof) {
        nRead = (int)(pReadr->iEof - pReadr->iReadOff);
      }
      rc = sqlite3OsRead(pReadr->pFd, &pReadr->aBuffer[iBuf], nRead, pReadr->iReadOff);
    }
  }

  return rc;
}

int vdbePmaReaderInit(SortSubtask *pTask, SorterFile *pFile, i64 iStart, PmaReader *pReadr, i64 *pnByte) {
  int rc;

  rc = vdbePmaReaderSeek(pTask, pReadr, pFile, iStart);
  if (rc == SQLITE_OK) {
    u64 nByte = 0;
    rc = vdbePmaReadVarint(pReadr, &nByte);
    pReadr->iEof = pReadr->iReadOff + nByte;
    *pnByte += nByte;
  }

  if (rc == SQLITE_OK) {
    rc = vdbePmaReaderNext(pReadr);
  }
  return rc;
}

int vdbeSorterCompareTail(SortSubtask *pTask, int *pbKey2Cached, const void *pKey1, int nKey1, const void *pKey2,
                          int nKey2) {
  UnpackedRecord *r2 = pTask->pUnpacked;
  if (*pbKey2Cached == 0) {
    sqlite3VdbeRecordUnpack(nKey2, pKey2, r2);
    *pbKey2Cached = 1;
  }
  return sqlite3VdbeRecordCompareWithSkip(nKey1, pKey1, r2, 1);
}

int vdbeSorterCompare(SortSubtask *pTask, int *pbKey2Cached, const void *pKey1, int nKey1, const void *pKey2,
                      int nKey2) {
  UnpackedRecord *r2 = pTask->pUnpacked;
  if (!*pbKey2Cached) {
    sqlite3VdbeRecordUnpack(nKey2, pKey2, r2);
    *pbKey2Cached = 1;
  }
  return sqlite3VdbeRecordCompare(nKey1, pKey1, r2);
}

int vdbeSorterCompareText(SortSubtask *pTask, int *pbKey2Cached, const void *pKey1, int nKey1, const void *pKey2,
                          int nKey2) {
  const u8 *const p1 = (const u8 *const)pKey1;
  const u8 *const p2 = (const u8 *const)pKey2;
  const u8 *const v1 = &p1[p1[0]];
  const u8 *const v2 = &p2[p2[0]];

  int n1;
  int n2;
  int res;

  n1 = (u32) * (&p1[1]);
  if (n1 >= 0x80)
    sqlite3GetVarint32((&p1[1]), (u32 *)&(n1));
  n2 = (u32) * (&p2[1]);
  if (n2 >= 0x80)
    sqlite3GetVarint32((&p2[1]), (u32 *)&(n2));
  res = memcmp(v1, v2, (((n1) < (n2) ? (n1) : (n2)) - 13) / 2);
  if (res == 0) {
    res = n1 - n2;
  }

  if (res == 0) {
    if (pTask->pSorter->pKeyInfo->nKeyField > 1) {
      res = vdbeSorterCompareTail(pTask, pbKey2Cached, pKey1, nKey1, pKey2, nKey2);
    }
  } else {
    if (pTask->pSorter->pKeyInfo->aSortFlags[0]) {
      res = res * -1;
    }
  }

  return res;
}

int vdbeSorterCompareInt(SortSubtask *pTask, int *pbKey2Cached, const void *pKey1, int nKey1, const void *pKey2,
                         int nKey2) {
  const u8 *const p1 = (const u8 *const)pKey1;
  const u8 *const p2 = (const u8 *const)pKey2;
  const int s1 = p1[1];
  const int s2 = p2[1];
  const u8 *const v1 = &p1[p1[0]];
  const u8 *const v2 = &p2[p2[0]];
  int res;

  if (s1 == s2) {
    static const u8 aLen[] = {0, 1, 2, 3, 4, 6, 8, 0, 0, 0};
    const u8 n = aLen[s1];
    int i;
    res = 0;
    for (i = 0; i < n; i++) {
      if ((res = v1[i] - v2[i]) != 0) {
        if (((v1[0] ^ v2[0]) & 0x80) != 0) {
          res = v1[0] & 0x80 ? -1 : +1;
        }
        break;
      }
    }
  } else if (s1 > 7 && s2 > 7) {
    res = s1 - s2;
  } else {
    if (s2 > 7) {
      res = +1;
    } else if (s1 > 7) {
      res = -1;
    } else {
      res = s1 - s2;
    }

    if (res > 0) {
      if (*v1 & 0x80)
        res = -1;
    } else {
      if (*v2 & 0x80)
        res = +1;
    }
  }

  if (res == 0) {
    if (pTask->pSorter->pKeyInfo->nKeyField > 1) {
      res = vdbeSorterCompareTail(pTask, pbKey2Cached, pKey1, nKey1, pKey2, nKey2);
    }
  } else if (pTask->pSorter->pKeyInfo->aSortFlags[0]) {
    res = res * -1;
  }

  return res;
}

int vdbeSorterJoinThread(SortSubtask *pTask) {
  int rc = SQLITE_OK;
  if (pTask->pThread) {
    void *pRet = ((void *)(intptr_t)(1));
    (void)sqlite3ThreadJoin(pTask->pThread, &pRet);
    rc = ((int)(intptr_t)(pRet));

    pTask->bDone = 0;
    pTask->pThread = 0;
  }
  return rc;
}

int vdbeSorterCreateThread(SortSubtask *pTask, void *(*xTask)(void *), void *pIn) {
  return sqlite3ThreadCreate(&pTask->pThread, xTask, pIn);
}

int vdbeSortAllocUnpacked(SortSubtask *pTask) {
  if (pTask->pUnpacked == 0) {
    pTask->pUnpacked = sqlite3VdbeAllocUnpackedRecord(pTask->pSorter->pKeyInfo);
    if (pTask->pUnpacked == 0)
      return 7;
    pTask->pUnpacked->nField = pTask->pSorter->pKeyInfo->nKeyField;
    pTask->pUnpacked->errCode = 0;
  }
  return SQLITE_OK;
}

SorterRecord *vdbeSorterMerge(SortSubtask *pTask, SorterRecord *p1, SorterRecord *p2) {
  SorterRecord *pFinal = 0;
  SorterRecord **pp = &pFinal;
  int bCached = 0;

  for (;;) {
    int res;
    res = pTask->xCompare(pTask, &bCached, ((void *)((SorterRecord *)(p1) + 1)), p1->nVal,
                          ((void *)((SorterRecord *)(p2) + 1)), p2->nVal);

    if (res <= 0) {
      *pp = p1;
      pp = &p1->u.pNext;
      p1 = p1->u.pNext;
      if (p1 == 0) {
        *pp = p2;
        break;
      }
    } else {
      *pp = p2;
      pp = &p2->u.pNext;
      p2 = p2->u.pNext;
      bCached = 0;
      if (p2 == 0) {
        *pp = p1;
        break;
      }
    }
  }
  return pFinal;
}

int vdbeSorterSort(SortSubtask *pTask, SorterList *pList) {
  int i;
  SorterRecord *p;
  int rc;
  SorterRecord *aSlot[64];

  rc = vdbeSortAllocUnpacked(pTask);
  if (rc != SQLITE_OK)
    return rc;

  p = pList->pList;
  pTask->xCompare = vdbeSorterGetCompare(pTask->pSorter);
  memset(aSlot, 0, sizeof(aSlot));

  while (p) {
    SorterRecord *pNext;
    if (pList->aMemory) {
      if ((u8 *)p == pList->aMemory) {
        pNext = 0;
      } else {
        pNext = (SorterRecord *)&pList->aMemory[p->u.iNext];
      }
    } else {
      pNext = p->u.pNext;
    }

    p->u.pNext = 0;
    for (i = 0; aSlot[i]; i++) {
      p = vdbeSorterMerge(pTask, p, aSlot[i]);

      aSlot[i] = 0;
    }
    aSlot[i] = p;
    p = pNext;
  }

  p = 0;
  for (i = 0; i < ((int)(sizeof(aSlot) / sizeof(aSlot[0]))); i++) {
    if (aSlot[i] == 0)
      continue;
    p = p ? vdbeSorterMerge(pTask, p, aSlot[i]) : aSlot[i];
  }
  pList->pList = p;

  return pTask->pUnpacked->errCode;
}

int vdbeSorterListToPMA(SortSubtask *pTask, SorterList *pList) {
  sqlite3 *db = pTask->pSorter->db;
  int rc = SQLITE_OK;
  PmaWriter writer;

  memset(&writer, 0, sizeof(PmaWriter));

  if (pTask->file.pFd == 0) {
    rc = vdbeSorterOpenTempFile(db, 0, &pTask->file.pFd);
  }

  if (rc == SQLITE_OK) {
    vdbeSorterExtendFile(db, pTask->file.pFd, pTask->file.iEof + pList->szPMA + 9);
  }

  if (rc == SQLITE_OK) {
    rc = vdbeSorterSort(pTask, pList);
  }

  if (rc == SQLITE_OK) {
    SorterRecord *p;
    SorterRecord *pNext = 0;

    vdbePmaWriterInit(pTask->file.pFd, &writer, pTask->pSorter->pgsz, pTask->file.iEof);
    pTask->nPMA++;
    vdbePmaWriteVarint(&writer, pList->szPMA);
    for (p = pList->pList; p; p = pNext) {
      pNext = p->u.pNext;
      vdbePmaWriteVarint(&writer, p->nVal);
      vdbePmaWriteBlob(&writer, (u8*)(((void *)((SorterRecord *)(p) + 1))), p->nVal);
      if (pList->aMemory == 0)
        sqlite3_free(p);
    }
    pList->pList = p;
    rc = vdbePmaWriterFinish(&writer, &pTask->file.iEof, &pTask->nSpill);
  }

  return rc;
}

int vdbeIncrMergerNew(SortSubtask *pTask, MergeEngine *pMerger, IncrMerger **ppOut) {
  int rc = SQLITE_OK;
  IncrMerger *pIncr = *ppOut = (IncrMerger *)(sqlite3FaultSim(100) ? 0 : sqlite3MallocZero(sizeof(*pIncr)));
  if (pIncr) {
    pIncr->pMerger = pMerger;
    pIncr->pTask = pTask;
    pIncr->mxSz = ((pTask->pSorter->mxKeysize + 9) > (pTask->pSorter->mxPmaSize / 2) ? (pTask->pSorter->mxKeysize + 9)
                                                                                     : (pTask->pSorter->mxPmaSize / 2));
    pTask->file2.iEof += pIncr->mxSz;
  } else {
    vdbeMergeEngineFree(pMerger);
    rc = 7;
  }

  return rc;
}

int vdbeMergeEngineInit(SortSubtask *pTask, MergeEngine *pMerger, int eMode) {
  int rc = SQLITE_OK;
  int i;
  int nTree;

  pMerger->pTask = pTask;

  nTree = pMerger->nTree;
  for (i = 0; i < nTree; i++) {
    if (8 > 0 && eMode == 2) {
      rc = vdbePmaReaderNext(&pMerger->aReadr[nTree - i - 1]);
    } else {
      rc = vdbePmaReaderIncrInit(&pMerger->aReadr[i], 0);
    }
    if (rc != SQLITE_OK)
      return rc;
  }

  for (i = pMerger->nTree - 1; i > 0; i--) {
    vdbeMergeEngineCompare(pMerger, i);
  }
  return pTask->pUnpacked->errCode;
}

int vdbeMergeEngineLevel0(SortSubtask *pTask, int nPMA, i64 *piOffset, MergeEngine **ppOut) {
  MergeEngine *pNew;
  i64 iOff = *piOffset;
  int i;
  int rc = SQLITE_OK;

  *ppOut = pNew = vdbeMergeEngineNew(nPMA);
  if (pNew == 0)
    rc = 7;

  for (i = 0; i < nPMA && rc == SQLITE_OK; i++) {
    i64 nDummy = 0;
    PmaReader *pReadr = &pNew->aReadr[i];
    rc = vdbePmaReaderInit(pTask, &pTask->file, iOff, pReadr, &nDummy);
    iOff = pReadr->iEof;
  }

  if (rc != SQLITE_OK) {
    vdbeMergeEngineFree(pNew);
    *ppOut = 0;
  }
  *piOffset = iOff;
  return rc;
}

int vdbeSorterAddToTree(SortSubtask *pTask, int nDepth, int iSeq, MergeEngine *pRoot, MergeEngine *pLeaf) {
  int rc = SQLITE_OK;
  int nDiv = 1;
  int i;
  MergeEngine *p = pRoot;
  IncrMerger *pIncr;

  rc = vdbeIncrMergerNew(pTask, pLeaf, &pIncr);

  for (i = 1; i < nDepth; i++) {
    nDiv = nDiv * 16;
  }

  for (i = 1; i < nDepth && rc == SQLITE_OK; i++) {
    int iIter = (iSeq / nDiv) % 16;
    PmaReader *pReadr = &p->aReadr[iIter];

    if (pReadr->pIncr == 0) {
      MergeEngine *pNew = vdbeMergeEngineNew(16);
      if (pNew == 0) {
        rc = 7;
      } else {
        rc = vdbeIncrMergerNew(pTask, pNew, &pReadr->pIncr);
      }
    }
    if (rc == SQLITE_OK) {
      p = pReadr->pIncr->pMerger;
      nDiv = nDiv / 16;
    }
  }

  if (rc == SQLITE_OK) {
    p->aReadr[iSeq % 16].pIncr = pIncr;
  } else {
    vdbeIncrFree(pIncr);
  }
  return rc;
}
