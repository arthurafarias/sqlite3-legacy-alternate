#define _GNU_SOURCE 1

#include <stdint.h>
#include <string.h>

#include "sqlite/Wal.h"

#include "sqlite/PgHdr.h"
#include "sqlite/Pgno.h"
#include "sqlite/WalCkptInfo.h"
#include "sqlite/WalHashLoc.h"
#include "sqlite/WalIndexHdr.h"
#include "sqlite/WalIterator.h"
#include "sqlite/WalWriter.h"
#include "sqlite/ht_slot.h"
#include "sqlite/i16.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_file.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_io_methods.h"
#include "sqlite/sqlite3_uint64.h"
#include "sqlite/sqlite3_vfs.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
__attribute__((noinline)) int walIndexPageRealloc(Wal *pWal, int iPage, volatile u32 **ppPage) {
  int rc = 0;

  if (pWal->nWiData <= iPage) {
    sqlite3_int64 nByte = sizeof(u32 *) * (1 + (i64)iPage);
    volatile u32 **apNew;
    apNew = (volatile u32 **)sqlite3Realloc((void *)pWal->apWiData, nByte);
    if (!apNew) {
      *ppPage = 0;
      return 7;
    }
    memset((void *)&apNew[pWal->nWiData], 0, sizeof(u32 *) * (iPage + 1 - pWal->nWiData));
    pWal->apWiData = apNew;
    pWal->nWiData = iPage + 1;
  }

  if (pWal->exclusiveMode == 2) {
    pWal->apWiData[iPage] = (u32 volatile *)sqlite3MallocZero((sizeof(ht_slot) * (4096 * 2) + 4096 * sizeof(u32)));
    if (!pWal->apWiData[iPage])
      rc = 7;
  } else {
    rc = sqlite3OsShmMap(pWal->pDbFd, iPage, (sizeof(ht_slot) * (4096 * 2) + 4096 * sizeof(u32)), pWal->writeLock, (void volatile **)&pWal->apWiData[iPage]);


    ;
    if (rc == 0) {
      if (iPage > 0 && sqlite3FaultSim(600))
        rc = 7;
    } else if ((rc & 0xff) == 8) {
      pWal->readOnly |= 2;
      if (rc == 8) {
        rc = 0;
      }
    }
  }

  *ppPage = pWal->apWiData[iPage];

  return rc;
}

int walIndexPage(Wal *pWal, int iPage, volatile u32 **ppPage) {

  ;
  if (pWal->nWiData <= iPage || (*ppPage = pWal->apWiData[iPage]) == 0) {
    return walIndexPageRealloc(pWal, iPage, ppPage);
  }
  return 0;
}

volatile WalCkptInfo *walCkptInfo(Wal *pWal) {

  ;
  return (volatile WalCkptInfo *)&(pWal->apWiData[0][sizeof(WalIndexHdr) / 2]);
}

volatile WalIndexHdr *walIndexHdr(Wal *pWal) {

  ;
  return (volatile WalIndexHdr *)pWal->apWiData[0];
}

static void walChecksumBytes(int nativeCksum, u8 *a, int nByte, const u32 *aIn, u32 *aOut) {
  u32 s1, s2;
  u32 *aData = (u32 *)a;
  u32 *aEnd = (u32 *)&a[nByte];

  if (aIn) {
    s1 = aIn[0];
    s2 = aIn[1];
  } else {
    s1 = s2 = 0;
  }

  if (!nativeCksum) {
    do {
      s1 += ((((aData[0]) & 0x000000FF) << 24) + (((aData[0]) & 0x0000FF00) << 8) + (((aData[0]) & 0x00FF0000) >> 8) + (((aData[0]) & 0xFF000000) >> 24)) + s2;
      s2 += ((((aData[1]) & 0x000000FF) << 24) + (((aData[1]) & 0x0000FF00) << 8) + (((aData[1]) & 0x00FF0000) >> 8) + (((aData[1]) & 0xFF000000) >> 24)) + s1;
      aData += 2;
    } while (aData < aEnd);
  } else if (nByte % 64 == 0) {
    do {
      s1 += *aData++ + s2;
      s2 += *aData++ + s1;
      s1 += *aData++ + s2;
      s2 += *aData++ + s1;
      s1 += *aData++ + s2;
      s2 += *aData++ + s1;
      s1 += *aData++ + s2;
      s2 += *aData++ + s1;
      s1 += *aData++ + s2;
      s2 += *aData++ + s1;
      s1 += *aData++ + s2;
      s2 += *aData++ + s1;
      s1 += *aData++ + s2;
      s2 += *aData++ + s1;
      s1 += *aData++ + s2;
      s2 += *aData++ + s1;
    } while (aData < aEnd);
  } else {
    do {
      s1 += *aData++ + s2;
      s2 += *aData++ + s1;
    } while (aData < aEnd);
  }

  aOut[0] = s1;
  aOut[1] = s2;
}

void walShmBarrier(Wal *pWal) {
  if (pWal->exclusiveMode != 2) {
    sqlite3OsShmBarrier(pWal->pDbFd);
  }
}

void walIndexWriteHdr(Wal *pWal) {
  volatile WalIndexHdr *aHdr = walIndexHdr(pWal);
  const int nCksum =

      __builtin_offsetof(

          WalIndexHdr

          ,

          aCksum

      )

      ;

  pWal->hdr.isInit = 1;
  pWal->hdr.iVersion = 3007000;
  walChecksumBytes(1, (u8 *)&pWal->hdr, nCksum, 0, pWal->hdr.aCksum);

  memcpy((void *)&aHdr[1], (const void *)&pWal->hdr, sizeof(WalIndexHdr));
  walShmBarrier(pWal);
  memcpy((void *)&aHdr[0], (const void *)&pWal->hdr, sizeof(WalIndexHdr));
}

void walEncodeFrame(Wal *pWal, u32 iPage, u32 nTruncate, u8 *aData, u8 *aFrame) {
  int nativeCksum;
  u32 *aCksum = pWal->hdr.aFrameCksum;

  sqlite3Put4byte(&aFrame[0], iPage);
  sqlite3Put4byte(&aFrame[4], nTruncate);
  if (pWal->iReCksum == 0) {
    memcpy(&aFrame[8], pWal->hdr.aSalt, 8);

    nativeCksum = (pWal->hdr.bigEndCksum == 0);
    walChecksumBytes(nativeCksum, aFrame, 8, aCksum, aCksum);
    walChecksumBytes(nativeCksum, aData, pWal->szPage, aCksum, aCksum);

    sqlite3Put4byte(&aFrame[16], aCksum[0]);
    sqlite3Put4byte(&aFrame[20], aCksum[1]);
  } else {
    memset(&aFrame[8], 0, 16);
  }
}

int walDecodeFrame(Wal *pWal, u32 *piPage, u32 *pnTruncate, u8 *aData, u8 *aFrame) {
  int nativeCksum;
  u32 *aCksum = pWal->hdr.aFrameCksum;
  u32 pgno;

  if (memcmp(&pWal->hdr.aSalt, &aFrame[8], 8) != 0) {
    return 0;
  }

  pgno = sqlite3Get4byte(&aFrame[0]);
  if (pgno == 0) {
    return 0;
  }

  if (!pWal->szPage) {
    return 0;
  }

  nativeCksum = (pWal->hdr.bigEndCksum == 0);
  walChecksumBytes(nativeCksum, aFrame, 8, aCksum, aCksum);
  walChecksumBytes(nativeCksum, aData, pWal->szPage, aCksum, aCksum);
  if (aCksum[0] != sqlite3Get4byte(&aFrame[16]) || aCksum[1] != sqlite3Get4byte(&aFrame[20])) {

    return 0;
  }

  *piPage = pgno;
  *pnTruncate = sqlite3Get4byte(&aFrame[4]);
  return 1;
}

int walLockShared(Wal *pWal, int lockIdx) {
  int rc;
  if (pWal->exclusiveMode)
    return 0;
  rc = sqlite3OsShmLock(pWal->pDbFd, lockIdx, 1, 2 | 4);

  ;

  return rc;
}

void walUnlockShared(Wal *pWal, int lockIdx) {
  if (pWal->exclusiveMode)
    return;
  (void)sqlite3OsShmLock(pWal->pDbFd, lockIdx, 1, 1 | 4);

  ;
}

int walLockExclusive(Wal *pWal, int lockIdx, int n) {
  int rc;
  if (pWal->exclusiveMode)
    return 0;
  rc = sqlite3OsShmLock(pWal->pDbFd, lockIdx, n, 2 | 8);

  ;

  return rc;
}

void walUnlockExclusive(Wal *pWal, int lockIdx, int n) {
  if (pWal->exclusiveMode)
    return;
  (void)sqlite3OsShmLock(pWal->pDbFd, lockIdx, n, 1 | 8);

  ;
}

static int walHash(u32 iPage) { return (iPage * 383) & ((4096 * 2) - 1); }

static int walNextHash(int iPriorHash) { return (iPriorHash + 1) & ((4096 * 2) - 1); }

int walHashGet(Wal *pWal, int iHash, WalHashLoc *pLoc) {
  int rc;

  rc = walIndexPage(pWal, iHash, &pLoc->aPgno);

  if (pLoc->aPgno) {
    pLoc->aHash = (volatile ht_slot *)&pLoc->aPgno[4096];
    if (iHash == 0) {
      pLoc->aPgno = &pLoc->aPgno[(sizeof(WalIndexHdr) * 2 + sizeof(WalCkptInfo)) / sizeof(u32)];
      pLoc->iZero = 0;
    } else {
      pLoc->iZero = (4096 - ((sizeof(WalIndexHdr) * 2 + sizeof(WalCkptInfo)) / sizeof(u32))) + (iHash - 1) * 4096;
    }
  } else if ((rc == 0)) {
    rc = 1;
  }
  return rc;
}

static int walFramePage(u32 iFrame) {
  int iHash = (iFrame + 4096 - (4096 - ((sizeof(WalIndexHdr) * 2 + sizeof(WalCkptInfo)) / sizeof(u32))) - 1) / 4096;

  return iHash;
}

u32 walFramePgno(Wal *pWal, u32 iFrame) {
  int iHash = walFramePage(iFrame);

  ;
  if (iHash == 0) {
    return pWal->apWiData[0][(sizeof(WalIndexHdr) * 2 + sizeof(WalCkptInfo)) / sizeof(u32) + iFrame - 1];
  }
  return pWal->apWiData[iHash][(iFrame - 1 - (4096 - ((sizeof(WalIndexHdr) * 2 + sizeof(WalCkptInfo)) / sizeof(u32)))) % 4096];
}

void walCleanupHash(Wal *pWal) {
  WalHashLoc sLoc;
  int iLimit = 0;
  int nByte;
  int i;

  ;
  ;
  ;

  if (pWal->hdr.mxFrame == 0)
    return;

  i = walHashGet(pWal, walFramePage(pWal->hdr.mxFrame), &sLoc);
  if ((i))
    return;

  iLimit = pWal->hdr.mxFrame - sLoc.iZero;

  for (i = 0; i < (4096 * 2); i++) {
    if (sLoc.aHash[i] > iLimit) {
      sLoc.aHash[i] = 0;
    }
  }

  nByte = (int)((char *)sLoc.aHash - (char *)&sLoc.aPgno[iLimit]);

  memset((void *)&sLoc.aPgno[iLimit], 0, nByte);
}

int walIndexAppend(Wal *pWal, u32 iFrame, u32 iPage) {
  int rc;
  WalHashLoc sLoc;

  rc = walHashGet(pWal, walFramePage(iFrame), &sLoc);

  if (rc == 0) {
    int iKey;
    int idx;
    int nCollide;

    idx = iFrame - sLoc.iZero;



    if (idx == 1) {
      int nByte = (int)((u8 *)&sLoc.aHash[(4096 * 2)] - (u8 *)sLoc.aPgno);


      memset((void *)sLoc.aPgno, 0, nByte);
    }

    if (sLoc.aPgno[idx - 1]) {
      walCleanupHash(pWal);


    }

    nCollide = idx;
    for (iKey = walHash(iPage); sLoc.aHash[iKey]; iKey = walNextHash(iKey)) {
      if ((nCollide--) == 0)
        return sqlite3CorruptError(68859);
    }
    sLoc.aPgno[(idx - 1) & (4096 - 1)] = iPage;
    __atomic_store_n((&sLoc.aHash[iKey]), ((ht_slot)idx), 0);
  }

  return rc;
}

int walIndexRecover(Wal *pWal) {
  int rc;
  i64 nSize;
  u32 aFrameCksum[2] = {0, 0};
  int iLock;

  iLock = 1 + pWal->ckptLock;
  rc = walLockExclusive(pWal, iLock, (3 + (0)) - iLock);
  if (rc) {
    return rc;
  }

  ;

  memset(&pWal->hdr, 0, sizeof(WalIndexHdr));

  rc = sqlite3OsFileSize(pWal->pWalFd, &nSize);
  if (rc != 0) {
    goto recovery_error;
  }

  if (nSize > 32) {
    u8 aBuf[32];
    u32 *aPrivate = 0;
    u8 *aFrame = 0;
    int szFrame;
    u8 *aData;
    int szPage;
    u32 magic;
    u32 version;
    int isValid;
    u32 iPg;
    u32 iLastFrame;

    rc = sqlite3OsRead(pWal->pWalFd, aBuf, 32, 0);
    if (rc != 0) {
      goto recovery_error;
    }

    magic = sqlite3Get4byte(&aBuf[0]);
    szPage = sqlite3Get4byte(&aBuf[8]);
    if ((magic & 0xFFFFFFFE) != 0x377f0682 || szPage & (szPage - 1) || szPage > 65536 || szPage < 512) {
      goto finished;
    }
    pWal->hdr.bigEndCksum = (u8)(magic & 0x00000001);
    pWal->szPage = szPage;
    pWal->nCkpt = sqlite3Get4byte(&aBuf[12]);
    memcpy(&pWal->hdr.aSalt, &aBuf[16], 8);

    walChecksumBytes(pWal->hdr.bigEndCksum == 0, aBuf, 32 - 2 * 4, 0, pWal->hdr.aFrameCksum);
    if (pWal->hdr.aFrameCksum[0] != sqlite3Get4byte(&aBuf[24]) || pWal->hdr.aFrameCksum[1] != sqlite3Get4byte(&aBuf[28])) {
      goto finished;
    }

    version = sqlite3Get4byte(&aBuf[4]);
    if (version != 3007000) {
      rc = sqlite3CantopenError(68991);
      goto finished;
    }

    szFrame = szPage + 24;
    aFrame = (u8 *)sqlite3_malloc64(szFrame + (sizeof(ht_slot) * (4096 * 2) + 4096 * sizeof(u32)));
    ;
    if (!aFrame) {
      rc = 7;
      goto recovery_error;
    }
    aData = &aFrame[24];
    aPrivate = (u32 *)&aData[szPage];

    iLastFrame = (nSize - 32) / szFrame;
    for (iPg = 0; iPg <= (u32)walFramePage(iLastFrame); iPg++) {
      u32 *aShare;
      u32 iFrame;
      u32 iLast = ((iLastFrame) < ((4096 - ((sizeof(WalIndexHdr) * 2 + sizeof(WalCkptInfo)) / sizeof(u32))) + iPg * 4096) ? (iLastFrame) : ((4096 - ((sizeof(WalIndexHdr) * 2 + sizeof(WalCkptInfo)) / sizeof(u32))) + iPg * 4096));
      u32 iFirst = 1 + (iPg == 0 ? 0 : (4096 - ((sizeof(WalIndexHdr) * 2 + sizeof(WalCkptInfo)) / sizeof(u32))) + (iPg - 1) * 4096);
      u32 nHdr, nHdr32;
      rc = walIndexPage(pWal, iPg, (volatile u32 **)&aShare);


      if (aShare == 0)
        break;
      ;
      pWal->apWiData[iPg] = aPrivate;

      for (iFrame = iFirst; iFrame <= iLast; iFrame++) {
        i64 iOffset = (32 + ((iFrame)-1) * (i64)((szPage) + 24));
        u32 pgno;
        u32 nTruncate;

        rc = sqlite3OsRead(pWal->pWalFd, aFrame, szFrame, iOffset);
        if (rc != 0)
          break;
        isValid = walDecodeFrame(pWal, &pgno, &nTruncate, aData, aFrame);
        if (!isValid)
          break;
        rc = walIndexAppend(pWal, iFrame, pgno);
        if ((rc != 0))
          break;

        if (nTruncate) {
          pWal->hdr.mxFrame = iFrame;
          pWal->hdr.nPage = nTruncate;
          pWal->hdr.szPage = (u16)((szPage & 0xff00) | (szPage >> 16));
          ;
          ;
          aFrameCksum[0] = pWal->hdr.aFrameCksum[0];
          aFrameCksum[1] = pWal->hdr.aFrameCksum[1];
        }
      }
      pWal->apWiData[iPg] = aShare;
      ;
      nHdr = (iPg == 0 ? (sizeof(WalIndexHdr) * 2 + sizeof(WalCkptInfo)) : 0);
      nHdr32 = nHdr / sizeof(u32);

      memcpy(&aShare[nHdr32], &aPrivate[nHdr32], (sizeof(ht_slot) * (4096 * 2) + 4096 * sizeof(u32)) - nHdr);


      ;
      if (iFrame <= iLast)
        break;
    }

    ;
    sqlite3_free(aFrame);
  }

finished:
  if (rc == 0) {
    volatile WalCkptInfo *pInfo;
    int i;
    pWal->hdr.aFrameCksum[0] = aFrameCksum[0];
    pWal->hdr.aFrameCksum[1] = aFrameCksum[1];
    walIndexWriteHdr(pWal);

    pInfo = walCkptInfo(pWal);
    pInfo->nBackfill = 0;
    pInfo->nBackfillAttempted = pWal->hdr.mxFrame;
    pInfo->aReadMark[0] = 0;
    for (i = 1; i < (8 - 3); i++) {
      rc = walLockExclusive(pWal, (3 + (i)), 1);
      if (rc == 0) {
        if (i == 1 && pWal->hdr.mxFrame) {
          pInfo->aReadMark[i] = pWal->hdr.mxFrame;
        } else {
          pInfo->aReadMark[i] = 0xffffffff;
        }


        ;
        walUnlockExclusive(pWal, (3 + (i)), 1);
      } else if (rc != 5) {
        goto recovery_error;
      }
    }

    if (pWal->hdr.nPage) {
      sqlite3_log((27 | (1 << 8)), "recovered %d frames from WAL file %s", pWal->hdr.mxFrame, pWal->zWalName);
    }
  }

recovery_error:;
  walUnlockExclusive(pWal, iLock, (3 + (0)) - iLock);
  return rc;
}

void walIndexClose(Wal *pWal, int isDelete) {
  if (pWal->exclusiveMode == 2 || pWal->bShmUnreliable) {
    int i;
    for (i = 0; i < pWal->nWiData; i++) {
      sqlite3_free((void *)pWal->apWiData[i]);
      pWal->apWiData[i] = 0;
    }
  }
  if (pWal->exclusiveMode != 2) {
    sqlite3OsShmUnmap(pWal->pDbFd, isDelete);
  }
}

void sqlite3WalLimit(Wal *pWal, i64 iLimit) {
  if (pWal)
    pWal->mxWalSize = iLimit;
}

static void walMerge(const u32 *aContent, ht_slot *aLeft, int nLeft, ht_slot **paRight, int *pnRight, ht_slot *aTmp) {
  int iLeft = 0;
  int iRight = 0;
  int iOut = 0;
  int nRight = *pnRight;
  ht_slot *aRight = *paRight;

  while (iRight < nRight || iLeft < nLeft) {
    ht_slot logpage;
    Pgno dbpage;

    if ((iLeft < nLeft) && (iRight >= nRight || aContent[aLeft[iLeft]] < aContent[aRight[iRight]])) {
      logpage = aLeft[iLeft++];
    } else {
      logpage = aRight[iRight++];
    }
    dbpage = aContent[logpage];

    aTmp[iOut++] = logpage;
    if (iLeft < nLeft && aContent[aLeft[iLeft]] == dbpage)
      iLeft++;




  }

  *paRight = aLeft;
  *pnRight = iOut;
  memcpy(aLeft, aTmp, sizeof(aTmp[0]) * iOut);
}

static void walMergesort(const u32 *aContent, ht_slot *aBuffer, ht_slot *aList, int *pnList) {
  struct Sublist {
    int nList;
    ht_slot *aList;
  };

  const int nList = *pnList;
  int nMerge = 0;
  ht_slot *aMerge = 0;
  int iList;
  u32 iSub = 0;
  struct Sublist aSub[13];

  memset(aSub, 0, sizeof(aSub));

  for (iList = 0; iList < nList; iList++) {
    nMerge = 1;
    aMerge = &aList[iList];
    for (iSub = 0; iList & (1 << iSub); iSub++) {
      struct Sublist *p;


      p = &aSub[iSub];




      walMerge(aContent, p->aList, p->nList, &aMerge, &nMerge, aBuffer);
    }
    aSub[iSub].aList = aMerge;
    aSub[iSub].nList = nMerge;
  }

  for (iSub++; iSub < ((int)(sizeof(aSub) / sizeof(aSub[0]))); iSub++) {
    if (nList & (1 << iSub)) {
      struct Sublist *p;


      p = &aSub[iSub];




      walMerge(aContent, p->aList, p->nList, &aMerge, &nMerge, aBuffer);
    }
  }

  *pnList = nMerge;
}

int walIteratorInit(Wal *pWal, u32 nBackfill, WalIterator **pp) {
  WalIterator *p;
  int nSegment;
  u32 iLast;
  sqlite3_int64 nByte;
  int i;
  ht_slot *aTmp;
  int rc = 0;

  iLast = pWal->hdr.mxFrame;

  nSegment = walFramePage(iLast) + 1;
  nByte = (

              __builtin_offsetof(

                  WalIterator

                  ,

                  aSegment

                  )

              + (nSegment) * sizeof(struct WalSegment)) +
          iLast * sizeof(ht_slot);
  p = (WalIterator *)sqlite3_malloc64(nByte + sizeof(ht_slot) * (iLast > 4096 ? 4096 : iLast));
  if (!p) {
    return 7;
  }
  memset(p, 0, nByte);
  p->nSegment = nSegment;
  aTmp = (ht_slot *)&(((u8 *)p)[nByte]);
  ;
  for (i = walFramePage(nBackfill + 1); rc == 0 && i < nSegment; i++) {
    WalHashLoc sLoc;

    rc = walHashGet(pWal, i, &sLoc);
    if (rc == 0) {
      int j;
      int nEntry;
      ht_slot *aIndex;

      if ((i + 1) == nSegment) {
        nEntry = (int)(iLast - sLoc.iZero);
      } else {
        nEntry = (int)((u32 *)sLoc.aHash - (u32 *)sLoc.aPgno);
      }
      aIndex = &((ht_slot *)&p->aSegment[p->nSegment])[sLoc.iZero];
      sLoc.iZero++;

      for (j = 0; j < nEntry; j++) {
        aIndex[j] = (ht_slot)j;
      }
      walMergesort((u32 *)sLoc.aPgno, aTmp, aIndex, &nEntry);
      p->aSegment[i].iZero = sLoc.iZero;
      p->aSegment[i].nEntry = nEntry;
      p->aSegment[i].aIndex = aIndex;
      p->aSegment[i].aPgno = (u32 *)sLoc.aPgno;
    }
  }
  if (rc != 0) {
    ;
    walIteratorFree(p);
    p = 0;
  }
  *pp = p;
  return rc;
}

int walBusyLock(Wal *pWal, int (*xBusy)(void *), void *pBusyArg, int lockIdx, int n) {
  int rc;
  do {
    rc = walLockExclusive(pWal, lockIdx, n);
  } while (xBusy && rc == 5 && xBusy(pBusyArg));

  return rc;
}

int walPagesize(Wal *pWal) { return (pWal->hdr.szPage & 0xfe00) + ((pWal->hdr.szPage & 0x0001) << 16); }

void walRestartHdr(Wal *pWal, u32 salt1) {
  volatile WalCkptInfo *pInfo = walCkptInfo(pWal);
  int i;
  u32 *aSalt = pWal->hdr.aSalt;
  pWal->nCkpt++;
  pWal->hdr.mxFrame = 0;
  sqlite3Put4byte((u8 *)&aSalt[0], 1 + sqlite3Get4byte((u8 *)&aSalt[0]));
  memcpy(&pWal->hdr.aSalt[1], &salt1, 4);
  walIndexWriteHdr(pWal);
  __atomic_store_n((&pInfo->nBackfill), (0), 0);
  pInfo->nBackfillAttempted = 0;
  pInfo->aReadMark[1] = 0;
  for (i = 2; i < (8 - 3); i++)
    pInfo->aReadMark[i] = 0xffffffff;
}

int walCheckpoint(Wal *pWal, sqlite3 *db, int eMode, int (*xBusy)(void *), void *pBusyArg, int sync_flags, u8 *zBuf) {
  int rc = 0;
  int szPage;
  WalIterator *pIter = 0;
  u32 iDbpage = 0;
  u32 iFrame = 0;
  u32 mxSafeFrame;
  u32 mxPage;
  int i;
  volatile WalCkptInfo *pInfo;

  szPage = walPagesize(pWal);
  ;
  ;
  pInfo = walCkptInfo(pWal);
  if (pInfo->nBackfill < pWal->hdr.mxFrame) {



    mxSafeFrame = pWal->hdr.mxFrame;
    mxPage = pWal->hdr.nPage;
    for (i = 1; i < (8 - 3); i++) {
      u32 y = __atomic_load_n((pInfo->aReadMark + i), 0);


      ;
      if (mxSafeFrame > y) {


        rc = walBusyLock(pWal, xBusy, pBusyArg, (3 + (i)), 1);
        if (rc == 0) {
          u32 iMark = (i == 1 ? mxSafeFrame : 0xffffffff);
          __atomic_store_n((pInfo->aReadMark + i), (iMark), 0);


          ;
          walUnlockExclusive(pWal, (3 + (i)), 1);
        } else if (rc == 5) {
          mxSafeFrame = y;
          xBusy = 0;
        } else {
          goto walcheckpoint_out;
        }
      }
    }

    if (pInfo->nBackfill < mxSafeFrame) {
      rc = walIteratorInit(pWal, pInfo->nBackfill, &pIter);


    }

    if (pIter && (rc = walBusyLock(pWal, xBusy, pBusyArg, (3 + (0)), 1)) == 0) {
      u32 nBackfill = pInfo->nBackfill;
      WalIndexHdr *pLive = (WalIndexHdr *)walIndexHdr(pWal);

      int bChg = memcmp(pLive->aSalt, pWal->hdr.aSalt, sizeof(pWal->hdr.aSalt));
      if (0 == bChg) {
        pInfo->nBackfillAttempted = mxSafeFrame;


        ;

        rc = sqlite3OsSync(pWal->pWalFd, (((sync_flags) >> 2) & 0x03));

        if (rc == 0) {
          i64 nReq = ((i64)mxPage * szPage);
          i64 nSize;
          sqlite3OsFileControl(pWal->pDbFd, 39, 0);
          rc = sqlite3OsFileSize(pWal->pDbFd, &nSize);
          if (rc == 0 && nSize < nReq) {
            if ((nSize + 65536 + (i64)pWal->hdr.mxFrame * szPage) < nReq) {

              rc = sqlite3CorruptError(69811);
            } else {
              sqlite3OsFileControlHint(pWal->pDbFd, 5, &nReq);
            }
          }
        }

        while (rc == 0 && 0 == walIteratorNext(pIter, &iDbpage, &iFrame)) {
          i64 iOffset;




          ;
          if (__atomic_load_n((&db->u1.isInterrupted), 0)) {
            rc = db->mallocFailed ? 7 : 9;
            break;
          }
          if (iFrame <= nBackfill || iFrame > mxSafeFrame || iDbpage > mxPage) {
            continue;
          }
          iOffset = (32 + ((iFrame)-1) * (i64)((szPage) + 24)) + 24;

          rc = sqlite3OsRead(pWal->pWalFd, zBuf, szPage, iOffset);
          if (rc != 0)
            break;
          iOffset = (iDbpage - 1) * (i64)szPage;
          ;
          rc = sqlite3OsWrite(pWal->pDbFd, zBuf, szPage, iOffset);
          if (rc != 0)
            break;
        }
        sqlite3OsFileControl(pWal->pDbFd, 37, 0);

        if (rc == 0) {
          if (mxSafeFrame == walIndexHdr(pWal)->mxFrame) {
            i64 szDb = pWal->hdr.nPage * (i64)szPage;
            ;
            rc = sqlite3OsTruncate(pWal->pDbFd, szDb);
            if (rc == 0) {
              rc = sqlite3OsSync(pWal->pDbFd, (((sync_flags) >> 2) & 0x03));
            }
          }
          if (rc == 0) {
            __atomic_store_n((&pInfo->nBackfill), (mxSafeFrame), 0);


            ;
          }
        }
      }

      walUnlockExclusive(pWal, (3 + (0)), 1);
    }

    if (rc == 5) {

      rc = 0;
    }
  }

  if (rc == 0 && eMode != 0) {




    ;
    if (pInfo->nBackfill < pWal->hdr.mxFrame) {
      rc = 5;
    } else if (eMode >= 2) {
      u32 salt1;
      sqlite3_randomness(4, &salt1);


      rc = walBusyLock(pWal, xBusy, pBusyArg, (3 + (1)), (8 - 3) - 1);
      if (rc == 0) {
        if (eMode == 3) {

          walRestartHdr(pWal, salt1);
          rc = sqlite3OsTruncate(pWal->pWalFd, 0);
        }
        walUnlockExclusive(pWal, (3 + (1)), (8 - 3) - 1);
      }
    }
  }

walcheckpoint_out:;
  walIteratorFree(pIter);
  return rc;
}

void walLimitSize(Wal *pWal, i64 nMax) {
  i64 sz;
  int rx;
  sqlite3BeginBenignMalloc();
  rx = sqlite3OsFileSize(pWal->pWalFd, &sz);
  if (rx == 0 && (sz > nMax)) {
    rx = sqlite3OsTruncate(pWal->pWalFd, nMax);
  }
  sqlite3EndBenignMalloc();
  if (rx) {
    sqlite3_log(rx, "cannot limit WAL size: %s", pWal->zWalName);
  }
}

int sqlite3WalClose(Wal *pWal, sqlite3 *db, int sync_flags, int nBuf, u8 *zBuf) {
  int rc = 0;
  if (pWal) {
    int isDelete = 0;



    if (zBuf != 0 && 0 == (rc = sqlite3OsLock(pWal->pDbFd, 4))) {
      if (pWal->exclusiveMode == 0) {
        pWal->exclusiveMode = 1;
      }
      rc = sqlite3WalCheckpoint(pWal, db, 0, 0, 0, sync_flags, nBuf, zBuf, 0, 0);
      if (rc == 0) {
        int bPersist = -1;
        sqlite3OsFileControlHint(pWal->pDbFd, 10, &bPersist);
        if (bPersist != 1) {

          isDelete = 1;
        } else if (pWal->mxWalSize >= 0) {

          walLimitSize(pWal, 0);
        }
      }
    }

    walIndexClose(pWal, isDelete);
    sqlite3OsClose(pWal->pWalFd);
    if (isDelete) {
      sqlite3BeginBenignMalloc();
      sqlite3OsDelete(pWal->pVfs, pWal->zWalName, 0);
      sqlite3EndBenignMalloc();
    };
    sqlite3_free((void *)pWal->apWiData);
    sqlite3_free(pWal);
  }
  return rc;
}

int walIndexTryHdr(Wal *pWal, int *pChanged) {
  u32 aCksum[2];
  WalIndexHdr h1, h2;
  WalIndexHdr volatile *aHdr;

  aHdr = walIndexHdr(pWal);
  memcpy(&h1, (void *)&aHdr[0], sizeof(h1));
  walShmBarrier(pWal);
  memcpy(&h2, (void *)&aHdr[1], sizeof(h2));

  if (memcmp(&h1, &h2, sizeof(h1)) != 0) {
    return 1;
  }
  if (h1.isInit == 0) {
    return 1;
  }
  walChecksumBytes(1, (u8 *)&h1, sizeof(h1) - sizeof(h1.aCksum), 0, aCksum);
  if (aCksum[0] != h1.aCksum[0] || aCksum[1] != h1.aCksum[1]) {
    return 1;
  }

  if (memcmp(&pWal->hdr, &h1, sizeof(WalIndexHdr))) {
    *pChanged = 1;
    memcpy(&pWal->hdr, &h1, sizeof(WalIndexHdr));
    pWal->szPage = (pWal->hdr.szPage & 0xfe00) + ((pWal->hdr.szPage & 0x0001) << 16);
    ;
    ;
  }

  return 0;
}

int walIndexReadHdr(Wal *pWal, int *pChanged) {
  int rc;
  int badHdr;
  volatile u32 *page0;

  rc = walIndexPage(pWal, 0, &page0);
  if (rc != 0) {


    if (rc == (8 | (5 << 8))) {






      pWal->bShmUnreliable = 1;
      pWal->exclusiveMode = 2;
      *pChanged = 1;
    } else {
      return rc;
    }
  } else {

    ;
  }

  badHdr = (page0 ? walIndexTryHdr(pWal, pChanged) : 1);

  if (badHdr) {
    if (pWal->bShmUnreliable == 0 && (pWal->readOnly & 2)) {
      if (0 == (rc = walLockShared(pWal, 0))) {
        walUnlockShared(pWal, 0);
        rc = (8 | (1 << 8));
      }
    } else {
      int bWriteLock = pWal->writeLock;
      if (bWriteLock || 0 == (rc = walLockExclusive(pWal, 0, 1))) {

        if (!bWriteLock)
          pWal->writeLock = 2;
        if (0 == (rc = walIndexPage(pWal, 0, &page0))) {
          badHdr = walIndexTryHdr(pWal, pChanged);
          if (badHdr) {

            ;
            rc = walIndexRecover(pWal);
            *pChanged = 1;
          }
        }
        if (bWriteLock == 0) {
          pWal->writeLock = 0;
          walUnlockExclusive(pWal, 0, 1);
        }
      }
    }
  }

  if (badHdr == 0 && pWal->hdr.iVersion != 3007000) {
    rc = sqlite3CantopenError(70265);
  }
  if (pWal->bShmUnreliable) {
    if (rc != 0) {
      walIndexClose(pWal, 0);
      pWal->bShmUnreliable = 0;



      if (rc == (10 | (2 << 8)))
        rc = (-1);
    }
    pWal->exclusiveMode = 0;
  }

  return rc;
}

int walBeginShmUnreliable(Wal *pWal, int *pChanged) {
  i64 szWal;
  i64 iOffset;
  u8 aBuf[32];
  u8 *aFrame = 0;
  int szFrame;
  u8 *aData;
  volatile void *pDummy;
  int rc;
  u32 aSaveCksum[2];

  rc = walLockShared(pWal, (3 + (0)));
  if (rc != 0) {
    if (rc == 5)
      rc = (-1);
    goto begin_unreliable_shm_out;
  }
  pWal->readLock = 0;

  rc = sqlite3OsShmMap(pWal->pDbFd, 0, (sizeof(ht_slot) * (4096 * 2) + 4096 * sizeof(u32)), 0, &pDummy);

  if (rc != (8 | (5 << 8))) {
    rc = (rc == 8 ? (-1) : rc);
    goto begin_unreliable_shm_out;
  }

  memcpy(&pWal->hdr, (void *)walIndexHdr(pWal), sizeof(WalIndexHdr));

  rc = sqlite3OsFileSize(pWal->pWalFd, &szWal);
  if (rc != 0) {
    goto begin_unreliable_shm_out;
  }
  if (szWal < 32) {

    *pChanged = 1;
    rc = (pWal->hdr.mxFrame == 0 ? 0 : (-1));
    goto begin_unreliable_shm_out;
  }

  rc = sqlite3OsRead(pWal->pWalFd, aBuf, 32, 0);
  if (rc != 0) {
    goto begin_unreliable_shm_out;
  }
  if (memcmp(&pWal->hdr.aSalt, &aBuf[16], 8)) {

    rc = (-1);
    goto begin_unreliable_shm_out;
  }

  szFrame = pWal->szPage + 24;
  aFrame = (u8 *)sqlite3_malloc64(szFrame);
  if (aFrame == 0) {
    rc = 7;
    goto begin_unreliable_shm_out;
  }
  aData = &aFrame[24];

  aSaveCksum[0] = pWal->hdr.aFrameCksum[0];
  aSaveCksum[1] = pWal->hdr.aFrameCksum[1];
  for (iOffset = (32 + ((pWal->hdr.mxFrame + 1) - 1) * (i64)((pWal->szPage) + 24)); iOffset + szFrame <= szWal; iOffset += szFrame) {
    u32 pgno;
    u32 nTruncate;

    rc = sqlite3OsRead(pWal->pWalFd, aFrame, szFrame, iOffset);
    if (rc != 0)
      break;
    if (!walDecodeFrame(pWal, &pgno, &nTruncate, aData, aFrame))
      break;

    if (nTruncate) {
      rc = (-1);
      break;
    }
  }
  pWal->hdr.aFrameCksum[0] = aSaveCksum[0];
  pWal->hdr.aFrameCksum[1] = aSaveCksum[1];

begin_unreliable_shm_out:
  sqlite3_free(aFrame);
  if (rc != 0) {
    int i;
    for (i = 0; i < pWal->nWiData; i++) {
      sqlite3_free((void *)pWal->apWiData[i]);
      pWal->apWiData[i] = 0;
    }
    pWal->bShmUnreliable = 0;
    sqlite3WalEndReadTransaction(pWal);
    *pChanged = 1;
  }
  return rc;
}

int walTryBeginRead(Wal *pWal, int *pChanged, int useWal, int *pCnt) {
  volatile WalCkptInfo *pInfo;
  int rc = 0;

  (*pCnt)++;
  if (*pCnt > 5) {
    int nDelay = 1;
    int cnt = (*pCnt & ~0);
    if (cnt > 100) {

      return 15;
    }
    if (*pCnt >= 10)
      nDelay = (cnt - 9) * (cnt - 9) * 39;

    sqlite3OsSleep(pWal->pVfs, nDelay);
    *pCnt &= ~0;
  }

  if (!useWal) {


    if (pWal->bShmUnreliable == 0) {
      rc = walIndexReadHdr(pWal, pChanged);
    }

    if (rc == 5) {

      (void)0;
      if (pWal->apWiData[0] == 0) {

        rc = (-1);
      } else if (0 == (rc = walLockShared(pWal, 2))) {
        walUnlockShared(pWal, 2);
        rc = (-1);
      } else if (rc == 5) {
        rc = (5 | (1 << 8));
      }
    };
    if (rc != 0) {
      return rc;
    } else if (pWal->bShmUnreliable) {
      return walBeginShmUnreliable(pWal, pChanged);
    }
  }

  pInfo = walCkptInfo(pWal);

  ;
  {
    u32 mxReadMark;
    int mxI;
    int i;
    u32 mxFrame;
    if (!useWal && __atomic_load_n((&pInfo->nBackfill), 0) == pWal->hdr.mxFrame

    ) {

      rc = walLockShared(pWal, (3 + (0)));
      walShmBarrier(pWal);
      if (rc == 0) {
        if (memcmp((void *)walIndexHdr(pWal), &pWal->hdr, sizeof(WalIndexHdr))) {

          walUnlockShared(pWal, (3 + (0)));
          return (-1);
        }
        pWal->readLock = 0;
        return 0;
      } else if (rc != 5) {
        return rc;
      }
    }

    mxReadMark = 0;
    mxI = 0;
    mxFrame = pWal->hdr.mxFrame;

    for (i = 1; i < (8 - 3); i++) {
      u32 thisMark = __atomic_load_n((pInfo->aReadMark + i), 0);


      ;
      if (mxReadMark <= thisMark && thisMark <= mxFrame) {


        mxReadMark = thisMark;
        mxI = i;
      }
    }
    if ((pWal->readOnly & 2) == 0 && (mxReadMark < mxFrame || mxI == 0)) {
      for (i = 1; i < (8 - 3); i++) {
        rc = walLockExclusive(pWal, (3 + (i)), 1);
        if (rc == 0) {
          __atomic_store_n((pInfo->aReadMark + i), (mxFrame), 0);
          mxReadMark = mxFrame;
          mxI = i;
          walUnlockExclusive(pWal, (3 + (i)), 1);
          break;
        } else if (rc != 5) {
          return rc;
        }
      }
    }
    if (mxI == 0) {


      return rc == 5 ? (-1) : (8 | (5 << 8));
    }

    (void)0;
    rc = walLockShared(pWal, (3 + (mxI)));
    ;
    if (rc) {




      return (rc & 0xFF) == 5 ? (-1) : rc;
    }

    pWal->minFrame = __atomic_load_n((&pInfo->nBackfill), 0) + 1;


    ;
    walShmBarrier(pWal);
    if (__atomic_load_n((pInfo->aReadMark + mxI), 0) != mxReadMark || memcmp((void *)walIndexHdr(pWal), &pWal->hdr, sizeof(WalIndexHdr))) {
      walUnlockShared(pWal, (3 + (mxI)));
      return (-1);
    } else {


      pWal->readLock = (i16)mxI;
    }
  }
  return rc;
}

int walBeginReadTransaction(Wal *pWal, int *pChanged) {
  int rc;
  int cnt = 0;

  do {
    rc = walTryBeginRead(pWal, pChanged, 0, &cnt);
  } while (rc == (-1));
  ;
  ;
  ;
  ;

  return rc;
}

int sqlite3WalBeginReadTransaction(Wal *pWal, int *pChanged) {
  int rc;
  ;
  {
    rc = walBeginReadTransaction(pWal, pChanged);
  };

  return rc;
}

void sqlite3WalEndReadTransaction(Wal *pWal) {

  if (pWal->readLock >= 0) {
    (void)sqlite3WalEndWriteTransaction(pWal);
    walUnlockShared(pWal, (3 + (pWal->readLock)));
    pWal->readLock = -1;
  }
}

int walFindFrame(Wal *pWal, Pgno pgno, u32 *piRead) {
  u32 iRead = 0;
  u32 iLast = pWal->hdr.mxFrame;
  int iHash;
  int iMinHash;

  if (iLast == 0 || (pWal->readLock == 0 && pWal->bShmUnreliable == 0)) {
    *piRead = 0;
    return 0;
  }

  iMinHash = walFramePage(pWal->minFrame);
  for (iHash = walFramePage(iLast); iHash >= iMinHash; iHash--) {
    WalHashLoc sLoc;
    int iKey;
    int nCollide;
    int rc;
    u32 iH;

    rc = walHashGet(pWal, iHash, &sLoc);
    if (rc != 0) {
      return rc;
    }
    nCollide = (4096 * 2);
    iKey = walHash(pgno);


    ;
    while ((iH = __atomic_load_n((&sLoc.aHash[iKey]), 0)) != 0) {
      u32 iFrame = iH + sLoc.iZero;
      if (iFrame <= iLast && iFrame >= pWal->minFrame && sLoc.aPgno[(iH - 1) & (4096 - 1)] == pgno) {


        iRead = iFrame;
      }
      if ((nCollide--) == 0) {
        *piRead = 0;
        return sqlite3CorruptError(71118);
      }
      iKey = walNextHash(iKey);
    }
    if (iRead)
      break;
  }

  *piRead = iRead;
  return 0;
}

int sqlite3WalFindFrame(Wal *pWal, Pgno pgno, u32 *piRead) {
  int rc;
  ;
  {
    rc = walFindFrame(pWal, pgno, piRead);
  };

  return rc;
}

int sqlite3WalReadFrame(Wal *pWal, u32 iRead, int nOut, u8 *pOut) {
  int sz;
  i64 iOffset;
  sz = pWal->hdr.szPage;
  sz = (sz & 0xfe00) + ((sz & 0x0001) << 16);
  ;
  ;
  iOffset = (32 + ((iRead)-1) * (i64)((sz) + 24)) + 24;

  return sqlite3OsRead(pWal->pWalFd, pOut, (nOut > sz ? sz : nOut), iOffset);
}

Pgno sqlite3WalDbsize(Wal *pWal) {
  if (pWal && (pWal->readLock >= 0)) {
    return pWal->hdr.nPage;
  }
  return 0;
}

int sqlite3WalBeginWriteTransaction(Wal *pWal) {
  int rc;

  if (pWal->readOnly) {
    return 8;
  }

  rc = walLockExclusive(pWal, 0, 1);
  if (rc) {
    return rc;
  }
  pWal->writeLock = 1;

  ;
  {
    if (memcmp(&pWal->hdr, (void *)walIndexHdr(pWal), sizeof(WalIndexHdr)) != 0) {
      rc = (5 | (2 << 8));
    }
  };

  if (rc != 0) {
    walUnlockExclusive(pWal, 0, 1);
    pWal->writeLock = 0;
  }
  return rc;
}

int sqlite3WalEndWriteTransaction(Wal *pWal) {
  if (pWal->writeLock) {
    walUnlockExclusive(pWal, 0, 1);
    pWal->writeLock = 0;
    pWal->iReCksum = 0;
    pWal->truncateOnCommit = 0;
  }
  return 0;
}

int sqlite3WalUndo(Wal *pWal, int (*xUndo)(void *, Pgno), void *pUndoCtx) {
  int rc = 0;
  if ((pWal->writeLock)) {
    Pgno iMax = pWal->hdr.mxFrame;
    Pgno iFrame;

    ;
    {

      memcpy(&pWal->hdr, (void *)walIndexHdr(pWal), sizeof(WalIndexHdr));

      for (iFrame = pWal->hdr.mxFrame + 1; (rc == 0) && iFrame <= iMax; iFrame++) {


        rc = xUndo(pUndoCtx, walFramePgno(pWal, iFrame));
      }
      if (iMax != pWal->hdr.mxFrame)
        walCleanupHash(pWal);
    };


    pWal->iReCksum = 0;
  }
  return rc;
}

void sqlite3WalSavepoint(Wal *pWal, u32 *aWalData) {

  aWalData[0] = pWal->hdr.mxFrame;
  aWalData[1] = pWal->hdr.aFrameCksum[0];
  aWalData[2] = pWal->hdr.aFrameCksum[1];
  aWalData[3] = pWal->nCkpt;
}

int sqlite3WalSavepointUndo(Wal *pWal, u32 *aWalData) {
  int rc = 0;

  if (aWalData[3] != pWal->nCkpt) {

    aWalData[0] = 0;
    aWalData[3] = pWal->nCkpt;
  }

  if (aWalData[0] < pWal->hdr.mxFrame) {
    pWal->hdr.mxFrame = aWalData[0];
    pWal->hdr.aFrameCksum[0] = aWalData[1];
    pWal->hdr.aFrameCksum[1] = aWalData[2];
    ;
    {
      walCleanupHash(pWal);
    };


    if (pWal->iReCksum > pWal->hdr.mxFrame) {
      pWal->iReCksum = 0;
    }
  }

  return rc;
}

int walRestartLog(Wal *pWal) {
  int rc = 0;
  int cnt;

  if (pWal->readLock == 0) {
    volatile WalCkptInfo *pInfo = walCkptInfo(pWal);


    if (pInfo->nBackfill > 0) {
      u32 salt1;
      sqlite3_randomness(4, &salt1);
      rc = walLockExclusive(pWal, (3 + (1)), (8 - 3) - 1);
      if (rc == 0) {

        walRestartHdr(pWal, salt1);
        walUnlockExclusive(pWal, (3 + (1)), (8 - 3) - 1);
      } else if (rc != 5) {
        return rc;
      }
    }
    walUnlockShared(pWal, (3 + (0)));
    pWal->readLock = -1;
    cnt = 0;
    do {
      int notUsed;
      rc = walTryBeginRead(pWal, &notUsed, 1, &cnt);
    } while (rc == (-1));


    ;
    ;
    ;
  }
  return rc;
}

int walRewriteChecksums(Wal *pWal, u32 iLast) {
  const int szPage = pWal->szPage;
  int rc = 0;
  u8 *aBuf;
  u8 aFrame[24];
  u32 iRead;
  i64 iCksumOff;

  aBuf = sqlite3_malloc(szPage + 24);
  if (aBuf == 0)
    return 7;

  if (pWal->iReCksum == 1) {
    iCksumOff = 24;
  } else {
    iCksumOff = (32 + ((pWal->iReCksum - 1) - 1) * (i64)((szPage) + 24)) + 16;
  }
  rc = sqlite3OsRead(pWal->pWalFd, aBuf, sizeof(u32) * 2, iCksumOff);
  pWal->hdr.aFrameCksum[0] = sqlite3Get4byte(aBuf);
  pWal->hdr.aFrameCksum[1] = sqlite3Get4byte(&aBuf[sizeof(u32)]);

  iRead = pWal->iReCksum;
  pWal->iReCksum = 0;
  for (; rc == 0 && iRead <= iLast; iRead++) {
    i64 iOff = (32 + ((iRead)-1) * (i64)((szPage) + 24));
    rc = sqlite3OsRead(pWal->pWalFd, aBuf, szPage + 24, iOff);
    if (rc == 0) {
      u32 iPgno, nDbSize;
      iPgno = sqlite3Get4byte(aBuf);
      nDbSize = sqlite3Get4byte(&aBuf[4]);

      walEncodeFrame(pWal, iPgno, nDbSize, &aBuf[24], aFrame);
      rc = sqlite3OsWrite(pWal->pWalFd, aFrame, sizeof(aFrame), iOff);
    }
  }

  sqlite3_free(aBuf);
  return rc;
}

int walFrames(Wal *pWal, int szPage, PgHdr *pList, Pgno nTruncate, int isCommit, int sync_flags) {
  int rc;
  u32 iFrame;
  PgHdr *p;
  PgHdr *pLast = 0;
  int nExtra = 0;
  int szFrame;
  i64 iOffset;
  WalWriter w;
  u32 iFirst = 0;
  WalIndexHdr *pLive;

  pLive = (WalIndexHdr *)walIndexHdr(pWal);
  if (memcmp(&pWal->hdr, (void *)pLive, sizeof(WalIndexHdr)) != 0) {
    iFirst = pLive->mxFrame + 1;
  }

  if (0 != (rc = walRestartLog(pWal))) {
    return rc;
  }

  iFrame = pWal->hdr.mxFrame;
  if (iFrame == 0) {
    u8 aWalHdr[32];
    u32 aCksum[2];

    sqlite3Put4byte(&aWalHdr[0], (0x377f0682 | 0));
    sqlite3Put4byte(&aWalHdr[4], 3007000);
    sqlite3Put4byte(&aWalHdr[8], szPage);
    sqlite3Put4byte(&aWalHdr[12], pWal->nCkpt);
    if (pWal->nCkpt == 0)
      sqlite3_randomness(8, pWal->hdr.aSalt);
    memcpy(&aWalHdr[16], pWal->hdr.aSalt, 8);
    walChecksumBytes(1, aWalHdr, 32 - 2 * 4, 0, aCksum);
    sqlite3Put4byte(&aWalHdr[24], aCksum[0]);
    sqlite3Put4byte(&aWalHdr[28], aCksum[1]);

    pWal->szPage = szPage;
    pWal->hdr.bigEndCksum = 0;
    pWal->hdr.aFrameCksum[0] = aCksum[0];
    pWal->hdr.aFrameCksum[1] = aCksum[1];
    pWal->truncateOnCommit = 1;

    rc = sqlite3OsWrite(pWal->pWalFd, aWalHdr, sizeof(aWalHdr), 0);
    ;
    if (rc != 0) {
      return rc;
    }

    if (pWal->syncHeader) {
      rc = sqlite3OsSync(pWal->pWalFd, (((sync_flags) >> 2) & 0x03));
      if (rc)
        return rc;
    }
  }
  if ((int)pWal->szPage != szPage) {
    return sqlite3CorruptError(71645);
  }

  w.pWal = pWal;
  w.pFd = pWal->pWalFd;
  w.iSyncPoint = 0;
  w.syncFlags = sync_flags;
  w.szPage = szPage;
  iOffset = (32 + ((iFrame + 1) - 1) * (i64)((szPage) + 24));
  szFrame = szPage + 24;

  for (p = pList; p; p = p->pDirty) {
    int nDbSize;

    if (iFirst && (p->pDirty || isCommit == 0)) {
      u32 iWrite = 0;
      walFindFrame(pWal, p->pgno, &iWrite);


      if (iWrite >= iFirst) {
        i64 iOff = (32 + ((iWrite)-1) * (i64)((szPage) + 24)) + 24;
        void *pData;
        if (pWal->iReCksum == 0 || iWrite < pWal->iReCksum) {
          pWal->iReCksum = iWrite;
        }
        pData = p->pData;
        rc = sqlite3OsWrite(pWal->pWalFd, pData, szPage, iOff);
        if (rc)
          return rc;
        p->flags &= ~0x040;
        continue;
      }
    }

    iFrame++;


    nDbSize = (isCommit && p->pDirty == 0) ? nTruncate : 0;
    rc = walWriteOneFrame(&w, p, nDbSize, iOffset);
    if (rc)
      return rc;
    pLast = p;
    iOffset += szFrame;
    p->flags |= 0x040;
  }

  if (isCommit && pWal->iReCksum) {
    rc = walRewriteChecksums(pWal, iFrame);
    if (rc)
      return rc;
  }

  if (isCommit && ((sync_flags) & 0x03) != 0) {
    int bSync = 1;
    if (pWal->padToSectorBoundary) {
      int sectorSize = sqlite3SectorSize(pWal->pWalFd);
      w.iSyncPoint = ((iOffset + sectorSize - 1) / sectorSize) * sectorSize;
      bSync = (w.iSyncPoint == iOffset);
      ;
      while (iOffset < w.iSyncPoint) {
        rc = walWriteOneFrame(&w, pLast, nTruncate, iOffset);
        if (rc)
          return rc;
        iOffset += szFrame;
        nExtra++;


      }
    }
    if (bSync) {


      rc = sqlite3OsSync(w.pFd, ((sync_flags) & 0x03));
    }
  }

  if (isCommit && pWal->truncateOnCommit && pWal->mxWalSize >= 0) {
    i64 sz = pWal->mxWalSize;
    if ((32 + ((iFrame + nExtra + 1) - 1) * (i64)((szPage) + 24)) > pWal->mxWalSize) {
      sz = (32 + ((iFrame + nExtra + 1) - 1) * (i64)((szPage) + 24));
    }
    walLimitSize(pWal, sz);
    pWal->truncateOnCommit = 0;
  }

  iFrame = pWal->hdr.mxFrame;
  for (p = pList; p && rc == 0; p = p->pDirty) {
    if ((p->flags & 0x040) == 0)
      continue;
    iFrame++;
    rc = walIndexAppend(pWal, iFrame, p->pgno);
  }

  while (rc == 0 && nExtra > 0) {
    iFrame++;
    nExtra--;
    rc = walIndexAppend(pWal, iFrame, pLast->pgno);
  }

  if (rc == 0) {

    pWal->hdr.szPage = (u16)((szPage & 0xff00) | (szPage >> 16));
    ;
    ;
    pWal->hdr.mxFrame = iFrame;
    if (isCommit) {
      pWal->hdr.iChange++;
      pWal->hdr.nPage = nTruncate;
    }

    if (isCommit) {
      walIndexWriteHdr(pWal);
      pWal->iCallback = iFrame;
    }
  }

  ;
  return rc;
}

int sqlite3WalFrames(Wal *pWal, int szPage, PgHdr *pList, Pgno nTruncate, int isCommit, int sync_flags) {
  int rc;
  ;
  {
    rc = walFrames(pWal, szPage, pList, nTruncate, isCommit, sync_flags);
  };

  return rc;
}

int sqlite3WalCheckpoint(Wal *pWal, sqlite3 *db, int eMode, int (*xBusy)(void *), void *pBusyArg, int sync_flags, int nBuf, u8 *zBuf, int *pnLog, int *pnCkpt) {
  int rc;
  int isChanged = 0;
  int eMode2 = eMode;
  int (*xBusy2)(void *) = xBusy;

  if (pWal->readOnly)
    return 8;
  ;

  ;
  if (xBusy2)
    (void)0;

  if (eMode != -1) {
    rc = walLockExclusive(pWal, 1, 1);
    ;
    ;
    if (rc == 0) {
      pWal->ckptLock = 1;

      if (eMode != 0) {
        rc = walBusyLock(pWal, xBusy2, pBusyArg, 0, 1);
        if (rc == 0) {
          pWal->writeLock = 1;
        } else if (rc == 5) {
          eMode2 = 0;
          xBusy2 = 0;
          rc = 0;
        }
      }
    }
  } else {
    rc = 0;
  }

  ;
  {
    if (rc == 0) {

      ;
      rc = walIndexReadHdr(pWal, &isChanged);
      if (eMode2 > 0)
        (void)0;
      if (isChanged && pWal->pDbFd->pMethods->iVersion >= 3) {
        sqlite3OsUnfetch(pWal->pDbFd, 0, 0);
      }
    }

    if (rc == 0) {
      sqlite3FaultSim(660);
      if (pWal->hdr.mxFrame && walPagesize(pWal) != nBuf) {
        rc = sqlite3CorruptError(71911);
      } else if (eMode2 != -1) {
        rc = walCheckpoint(pWal, db, eMode2, xBusy2, pBusyArg, sync_flags, zBuf);
      }

      if (rc == 0 || rc == 5) {
        if (pnLog)
          *pnLog = (int)pWal->hdr.mxFrame;


        ;
        if (pnCkpt)
          *pnCkpt = (int)(walCkptInfo(pWal)->nBackfill);
      }
    }
  };

  if (isChanged) {

    memset(&pWal->hdr, 0, sizeof(WalIndexHdr));
  }

  ;
  ;

  (void)sqlite3WalEndWriteTransaction(pWal);
  if (pWal->ckptLock) {
    walUnlockExclusive(pWal, 1, 1);
    pWal->ckptLock = 0;
  };

  return (rc == 0 && eMode != eMode2 ? 5 : rc);
}

int sqlite3WalCallback(Wal *pWal) {
  u32 ret = 0;
  if (pWal) {
    ret = pWal->iCallback;
    pWal->iCallback = 0;
  }
  return (int)ret;
}

int sqlite3WalExclusiveMode(Wal *pWal, int op) {
  int rc;

  if (op == 0) {
    if (pWal->exclusiveMode != 0) {
      pWal->exclusiveMode = 0;
      if (walLockShared(pWal, (3 + (pWal->readLock))) != 0) {
        pWal->exclusiveMode = 1;
      }
      rc = pWal->exclusiveMode == 0;
    } else {

      rc = 0;
    }
  } else if (op > 0) {




    walUnlockShared(pWal, (3 + (pWal->readLock)));
    pWal->exclusiveMode = 1;
    rc = 1;
  } else {
    rc = pWal->exclusiveMode == 0;
  }
  return rc;
}

int sqlite3WalHeapMemory(Wal *pWal) { return (pWal && pWal->exclusiveMode == 2); }

sqlite3_file *sqlite3WalFile(Wal *pWal) { return pWal->pWalFd; }

int sqlite3WalDefaultHook(void *pClientData, sqlite3 *db, const char *zDb, int nFrame) {
  if (nFrame >= ((int)(intptr_t)(pClientData))) {
    sqlite3BeginBenignMalloc();
    sqlite3_wal_checkpoint(db, zDb);
    sqlite3EndBenignMalloc();
  }
  return 0;
}
