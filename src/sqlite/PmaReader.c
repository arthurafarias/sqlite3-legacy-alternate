#define _GNU_SOURCE 1

#include <string.h>

#include "sqlite/PmaReader.h"

#include "sqlite/IncrMerger.h"
#include "sqlite/MergeEngine.h"
#include "sqlite/SortSubtask.h"
#include "sqlite/SorterFile.h"
#include "sqlite/Vdbe.h"
#include "sqlite/VdbeSorter.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_file.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
void vdbePmaReaderClear(PmaReader *pReadr) {
  sqlite3_free(pReadr->aAlloc);
  sqlite3_free(pReadr->aBuffer);
  if (pReadr->aMap)
    sqlite3OsUnfetch(pReadr->pFd, 0, pReadr->aMap);
  vdbeIncrFree(pReadr->pIncr);
  memset(pReadr, 0, sizeof(PmaReader));
}

int vdbePmaReadBlob(PmaReader *p, int nByte, u8 **ppOut) {
  int iBuf;
  int nAvail;

  if (p->aMap) {
    *ppOut = &p->aMap[p->iReadOff];
    p->iReadOff += nByte;
    return 0;
  }

  iBuf = p->iReadOff % p->nBuffer;
  if (iBuf == 0) {
    int nRead;
    int rc;

    if ((p->iEof - p->iReadOff) > (i64)p->nBuffer) {
      nRead = p->nBuffer;
    } else {
      nRead = (int)(p->iEof - p->iReadOff);
    }

    ((void)(0))

        ;

    rc = sqlite3OsRead(p->pFd, p->aBuffer, nRead, p->iReadOff);

    ((void)(0))

        ;
    if (rc != 0)
      return rc;
  }
  nAvail = p->nBuffer - iBuf;

  if (nByte <= nAvail) {

    *ppOut = &p->aBuffer[iBuf];
    p->iReadOff += nByte;
  } else {

    int nRem;

    if (p->nAlloc < nByte) {
      u8 *aNew;
      sqlite3_int64 nNew = ((128) > (2 * (sqlite3_int64)p->nAlloc) ? (128) : (2 * (sqlite3_int64)p->nAlloc));
      while (nByte > nNew)
        nNew = nNew * 2;
      aNew = sqlite3Realloc(p->aAlloc, nNew);
      if (!aNew)
        return 7;
      p->nAlloc = nNew;
      p->aAlloc = aNew;
    }

    memcpy(p->aAlloc, &p->aBuffer[iBuf], nAvail);
    p->iReadOff += nAvail;
    nRem = nByte - nAvail;

    while (nRem > 0) {
      int rc;
      int nCopy;
      u8 *aNext = 0;

      nCopy = nRem;
      if (nRem > p->nBuffer)
        nCopy = p->nBuffer;
      rc = vdbePmaReadBlob(p, nCopy, &aNext);
      if (rc != 0)
        return rc;

      ((void)(0))

          ;

      ((void)(0))

          ;
      memcpy(&p->aAlloc[nByte - nRem], aNext, nCopy);
      nRem -= nCopy;
    }

    *ppOut = p->aAlloc;
  }

  return 0;
}

int vdbePmaReadVarint(PmaReader *p, u64 *pnOut) {
  int iBuf;

  if (p->aMap) {
    p->iReadOff += sqlite3GetVarint(&p->aMap[p->iReadOff], pnOut);
  } else {
    iBuf = p->iReadOff % p->nBuffer;
    if (iBuf && (p->nBuffer - iBuf) >= 9) {
      p->iReadOff += sqlite3GetVarint(&p->aBuffer[iBuf], pnOut);
    } else {
      u8 aVarint[16], *a;
      int i = 0, rc;
      do {
        rc = vdbePmaReadBlob(p, 1, &a);
        if (rc)
          return rc;
        aVarint[(i++) & 0xf] = a[0];
      } while ((a[0] & 0x80) != 0);
      sqlite3GetVarint(aVarint, pnOut);
    }
  }

  return 0;
}

int vdbePmaReaderNext(PmaReader *pReadr) {
  int rc = 0;
  u64 nRec = 0;

  if (pReadr->iReadOff >= pReadr->iEof) {
    IncrMerger *pIncr = pReadr->pIncr;
    int bEof = 1;
    if (pIncr) {
      rc = vdbeIncrSwap(pIncr);
      if (rc == 0 && pIncr->bEof == 0) {
        rc = vdbePmaReaderSeek(pIncr->pTask, pReadr, &pIncr->aFile[0], pIncr->iStartOff);
        bEof = 0;
      }
    }

    if (bEof) {

      vdbePmaReaderClear(pReadr);
      ;
      return rc;
    }
  }

  if (rc == 0) {
    rc = vdbePmaReadVarint(pReadr, &nRec);
  }
  if (rc == 0) {
    pReadr->nKey = (int)nRec;
    rc = vdbePmaReadBlob(pReadr, (int)nRec, &pReadr->aKey);
    ;
  }

  return rc;
}

int vdbePmaReaderIncrMergeInit(PmaReader *pReadr, int eMode) {
  int rc = 0;
  IncrMerger *pIncr = pReadr->pIncr;
  SortSubtask *pTask = pIncr->pTask;
  sqlite3 *db = pTask->pSorter->db;

  rc = vdbeMergeEngineInit(pTask, pIncr->pMerger, eMode);

  if (rc == 0) {
    int mxSz = pIncr->mxSz;

    if (pIncr->bUseThread) {
      rc = vdbeSorterOpenTempFile(db, mxSz, &pIncr->aFile[0].pFd);
      if (rc == 0) {
        rc = vdbeSorterOpenTempFile(db, mxSz, &pIncr->aFile[1].pFd);
      }
    } else

    {
      if (pTask->file2.pFd == 0) {

        ((void)(0))

            ;
        rc = vdbeSorterOpenTempFile(db, pTask->file2.iEof, &pTask->file2.pFd);
        pTask->file2.iEof = 0;
      }
      if (rc == 0) {
        pIncr->aFile[1].pFd = pTask->file2.pFd;
        pIncr->iStartOff = pTask->file2.iEof;
        pTask->file2.iEof += mxSz;
      }
    }
  }

  if (rc == 0 && pIncr->bUseThread) {

    ((void)(0))

        ;
    rc = vdbeIncrPopulate(pIncr);
  }

  if (rc == 0 && (8 == 0 || eMode != 1)) {
    rc = vdbePmaReaderNext(pReadr);
  }

  return rc;
}

int vdbePmaReaderIncrInit(PmaReader *pReadr, int eMode) {
  IncrMerger *pIncr = pReadr->pIncr;
  int rc = 0;
  if (pIncr) {

    ((void)(0))

        ;
    if (pIncr->bUseThread) {
      void *pCtx = (void *)pReadr;
      rc = vdbeSorterCreateThread(pIncr->pTask, vdbePmaReaderBgIncrInit, pCtx);
    } else

    {
      rc = vdbePmaReaderIncrMergeInit(pReadr, eMode);
    }
  }
  return rc;
}
