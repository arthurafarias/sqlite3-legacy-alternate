#define _GNU_SOURCE 1
#include <string.h>
#include "sqlite/BtShared.h"
#include "sqlite/Bitvec.h"
#include "sqlite/BtCursor.h"
#include "sqlite/CellInfo.h"
#include "sqlite/DbPage.h"
#include "sqlite/MemPage.h"
#include "sqlite/Pager.h"
#include "sqlite/Pgno.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/Bitvec.h"
#include "sqlite/SqliteMutexType.h"
#include "sqlite/SqliteResultCode.h"
static const char zMagicHeader[] = "SQLite format 3";

BtShared *sqlite3SharedCacheList = 0;

void invalidateAllOverflowCache(BtShared *pBt) {
  BtCursor *p;

  for (p = pBt->pCursor; p; p = p->pNext) {
    (p->curFlags &= ~0x04);
  }
}

int btreeSetHasContent(BtShared *pBt, Pgno pgno) {
  int rc = SQLITE_OK;
  if (!pBt->pHasContent) {
    pBt->pHasContent = sqlite3BitvecCreate(pBt->nPage);
    if (!pBt->pHasContent) {
      rc = 7;
    }
  }
  if (rc == SQLITE_OK && pgno <= sqlite3BitvecSize(pBt->pHasContent)) {
    rc = sqlite3BitvecSet(pBt->pHasContent, pgno);
  }
  return rc;
}

int btreeGetHasContent(BtShared *pBt, Pgno pgno) {
  Bitvec *p = pBt->pHasContent;
  return p && (pgno > sqlite3BitvecSize(p) || sqlite3BitvecTestNotNull(p, pgno));
}

void btreeClearHasContent(BtShared *pBt) {
  sqlite3BitvecDestroy(pBt->pHasContent);
  pBt->pHasContent = 0;
}

int saveAllCursors(BtShared *pBt, Pgno iRoot, BtCursor *pExcept) {
  BtCursor *p;

  for (p = pBt->pCursor; p; p = p->pNext) {
    if (p != pExcept && (0 == iRoot || p->pgnoRoot == iRoot))
      break;
  }
  if (p)
    return saveCursorsOnList(p, iRoot, pExcept);
  if (pExcept)
    pExcept->curFlags &= ~0x20;
  return SQLITE_OK;
}

Pgno ptrmapPageno(BtShared *pBt, Pgno pgno) {
  int nPagesPerMapPage;
  Pgno iPtrMap, ret;

  if (pgno < 2)
    return 0;
  nPagesPerMapPage = (pBt->usableSize / 5) + 1;
  iPtrMap = (pgno - 2) / nPagesPerMapPage;
  ret = (iPtrMap * nPagesPerMapPage) + 2;
  if (ret == ((Pgno)((sqlite3PendingByte / ((pBt)->pageSize)) + 1))) {
    ret++;
  }
  return ret;
}

void ptrmapPut(BtShared *pBt, Pgno key, u8 eType, Pgno parent, int *pRC) {
  DbPage *pDbPage;
  u8 *pPtrmap;
  Pgno iPtrmap;
  int offset;
  int rc;

  if (*pRC)
    return;

  if (key == 0) {
    *pRC = sqlite3CorruptError(74300);
    return;
  }
  iPtrmap = ptrmapPageno(pBt, key);
  rc = sqlite3PagerGet(pBt->pPager, iPtrmap, &pDbPage, 0);
  if (rc != SQLITE_OK) {
    *pRC = rc;
    return;
  }
  if (((char *)sqlite3PagerGetExtra(pDbPage))[0] != 0) {
    *pRC = sqlite3CorruptError(74313);
    goto ptrmap_exit;
  }
  offset = (5 * (key - iPtrmap - 1));
  if (offset < 0) {
    *pRC = sqlite3CorruptError(74318);
    goto ptrmap_exit;
  }

  pPtrmap = (u8 *)sqlite3PagerGetData(pDbPage);

  if (eType != pPtrmap[offset] || sqlite3Get4byte(&pPtrmap[offset + 1]) != parent) {
    *pRC = rc = sqlite3PagerWrite(pDbPage);
    if (rc == SQLITE_OK) {
      pPtrmap[offset] = eType;
      sqlite3Put4byte(&pPtrmap[offset + 1], parent);
    }
  }

ptrmap_exit:
  sqlite3PagerUnref(pDbPage);
}

int ptrmapGet(BtShared *pBt, Pgno key, u8 *pEType, Pgno *pPgno) {
  DbPage *pDbPage;
  int iPtrmap;
  u8 *pPtrmap;
  int offset;
  int rc;

  iPtrmap = ptrmapPageno(pBt, key);
  rc = sqlite3PagerGet(pBt->pPager, iPtrmap, &pDbPage, 0);
  if (rc != 0) {
    return rc;
  }
  pPtrmap = (u8 *)sqlite3PagerGetData(pDbPage);

  offset = (5 * (key - iPtrmap - 1));
  if (offset < 0) {
    sqlite3PagerUnref(pDbPage);
    return sqlite3CorruptError(74363);
  }

  *pEType = pPtrmap[offset];
  if (pPgno)
    *pPgno = sqlite3Get4byte(&pPtrmap[offset + 1]);

  sqlite3PagerUnref(pDbPage);
  if (*pEType < 1 || *pEType > 5)
    return sqlite3CorruptError(74371);
  return SQLITE_OK;
}

int btreeGetPage(BtShared *pBt, Pgno pgno, MemPage **ppPage, int flags) {
  int rc;
  DbPage *pDbPage;

  rc = sqlite3PagerGet(pBt->pPager, pgno, (DbPage **)&pDbPage, flags);
  if (rc)
    return rc;
  *ppPage = btreePageFromDbPage(pDbPage, pgno, pBt);
  return SQLITE_OK;
}

MemPage *btreePageLookup(BtShared *pBt, Pgno pgno) {
  DbPage *pDbPage;

  pDbPage = sqlite3PagerLookup(pBt->pPager, pgno);
  if (pDbPage) {
    return btreePageFromDbPage(pDbPage, pgno, pBt);
  }
  return 0;
}

Pgno btreePagecount(BtShared *pBt) {
  return pBt->nPage;
}

int getAndInitPage(BtShared *pBt, Pgno pgno, MemPage **ppPage, int bReadOnly) {
  int rc;
  DbPage *pDbPage;
  MemPage *pPage;

  if (pgno > btreePagecount(pBt)) {
    *ppPage = 0;
    return sqlite3CorruptError(75617);
  }
  rc = sqlite3PagerGet(pBt->pPager, pgno, (DbPage **)&pDbPage, bReadOnly);
  if (rc) {
    *ppPage = 0;
    return rc;
  }
  pPage = (MemPage *)sqlite3PagerGetExtra(pDbPage);
  if (pPage->isInit == 0) {
    btreePageFromDbPage(pDbPage, pgno, pBt);
    rc = btreeInitPage(pPage);
    if (rc != SQLITE_OK) {
      releasePage(pPage);
      *ppPage = 0;
      return rc;
    }
  }

  *ppPage = pPage;
  return SQLITE_OK;
}

int btreeGetUnusedPage(BtShared *pBt, Pgno pgno, MemPage **ppPage, int flags) {
  int rc = btreeGetPage(pBt, pgno, ppPage, flags);
  if (rc == SQLITE_OK) {
    if (sqlite3PagerPageRefcount((*ppPage)->pDbPage) > 1) {
      releasePage(*ppPage);
      *ppPage = 0;
      return sqlite3CorruptError(75689);
    }
    (*ppPage)->isInit = 0;
  } else {
    *ppPage = 0;
  }
  return rc;
}

int removeFromSharingList(BtShared *pBt) {
  sqlite3_mutex *pMainMtx;
  BtShared *pList;
  int removed = 0;

  pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
  sqlite3_mutex_enter(pMainMtx);
  pBt->nRef--;
  if (pBt->nRef <= 0) {
    if (sqlite3SharedCacheList == pBt) {
      sqlite3SharedCacheList = pBt->pNext;
    } else {
      pList = sqlite3SharedCacheList;
      while ((pList) && pList->pNext != pBt) {
        pList = pList->pNext;
      }
      if ((pList)) {
        pList->pNext = pBt->pNext;
      }
    }
    if (1) {
      sqlite3_mutex_free(pBt->mutex);
    }
    removed = 1;
  }
  sqlite3_mutex_leave(pMainMtx);
  return removed;
}

__attribute__((noinline)) int allocateTempSpace(BtShared *pBt) {
  pBt->pTmpSpace = (u8*)(sqlite3PageMalloc(pBt->pageSize));
  if (pBt->pTmpSpace == 0) {
    BtCursor *pCur = pBt->pCursor;
    pBt->pCursor = pCur->pNext;
    memset(pCur, 0, sizeof(*pCur));
    return 7;
  }

  memset(pBt->pTmpSpace, 0, 8);
  pBt->pTmpSpace += 4;
  return SQLITE_OK;
}

void freeTempSpace(BtShared *pBt) {
  if (pBt->pTmpSpace) {
    pBt->pTmpSpace -= 4;
    sqlite3PageFree(pBt->pTmpSpace);
    pBt->pTmpSpace = 0;
  }
}

int lockBtree(BtShared *pBt) {
  int rc;
  MemPage *pPage1;
  u32 nPage;
  u32 nPageFile = 0;

  rc = sqlite3PagerSharedLock(pBt->pPager);
  if (rc != SQLITE_OK)
    return rc;
  rc = btreeGetPage(pBt, 1, &pPage1, 0);
  if (rc != SQLITE_OK)
    return rc;

  nPage = sqlite3Get4byte(28 + (u8 *)pPage1->aData);
  sqlite3PagerPagecount(pBt->pPager, (int *)&nPageFile);
  if (nPage == 0 || memcmp(24 + (u8 *)pPage1->aData, 92 + (u8 *)pPage1->aData, 4) != 0) {
    nPage = nPageFile;
  }
  if ((pBt->db->flags & 0x02000000) != 0) {
    nPage = 0;
  }
  if (nPage > 0) {
    u32 pageSize;
    u32 usableSize;
    u8 *page1 = pPage1->aData;
    rc = SQLITE_NOTADB;

    if (memcmp(page1, zMagicHeader, 16) != 0) {
      goto page1_init_failed;
    }

    if (page1[18] > 2) {
      pBt->btsFlags |= 0x0001;
    }
    if (page1[19] > 2) {
      goto page1_init_failed;
    }

    if (page1[19] == 2 && (pBt->btsFlags & 0x0020) == 0) {
      int isOpen = 0;
      rc = sqlite3PagerOpenWal(pBt->pPager, &isOpen);
      if (rc != SQLITE_OK) {
        goto page1_init_failed;
      } else {
        if (isOpen == 0) {
          releasePageOne(pPage1);
          return SQLITE_OK;
        }
      }
      rc = SQLITE_NOTADB;
    } else {
    }

    if (memcmp(&page1[21], "\100\040\040", 3) != 0) {
      goto page1_init_failed;
    }

    pageSize = (page1[16] << 8) | (page1[17] << 16);

    if (((pageSize - 1) & pageSize) != 0 || pageSize > 65536 || pageSize <= 256) {
      goto page1_init_failed;
    }

    usableSize = pageSize - page1[20];
    if ((u32)pageSize != pBt->pageSize) {
      releasePageOne(pPage1);
      pBt->usableSize = usableSize;
      pBt->pageSize = pageSize;
      pBt->btsFlags |= 0x0002;
      freeTempSpace(pBt);
      rc = sqlite3PagerSetPagesize(pBt->pPager, &pBt->pageSize, pageSize - usableSize);
      return rc;
    }
    if (nPage > nPageFile) {
      if (sqlite3WritableSchema(pBt->db) == 0) {
        rc = sqlite3CorruptError(76632);
        goto page1_init_failed;
      } else {
        nPage = nPageFile;
      }
    }

    if (usableSize < 480) {
      goto page1_init_failed;
    }
    pBt->btsFlags |= 0x0002;
    pBt->pageSize = pageSize;
    pBt->usableSize = usableSize;

    pBt->autoVacuum = (sqlite3Get4byte(&page1[36 + 4 * 4]) ? 1 : 0);
    pBt->incrVacuum = (sqlite3Get4byte(&page1[36 + 7 * 4]) ? 1 : 0);
  }

  pBt->maxLocal = (u16)((pBt->usableSize - 12) * 64 / 255 - 23);
  pBt->minLocal = (u16)((pBt->usableSize - 12) * 32 / 255 - 23);
  pBt->maxLeaf = (u16)(pBt->usableSize - 35);
  pBt->minLeaf = (u16)((pBt->usableSize - 12) * 32 / 255 - 23);
  if (pBt->maxLocal > 127) {
    pBt->max1bytePayload = 127;
  } else {
    pBt->max1bytePayload = (u8)pBt->maxLocal;
  }

  pBt->pPage1 = pPage1;
  pBt->nPage = nPage;
  return SQLITE_OK;

page1_init_failed:
  releasePageOne(pPage1);
  pBt->pPage1 = 0;
  return rc;
}

void unlockBtreeIfUnused(BtShared *pBt) {
  if (pBt->inTransaction == 0 && pBt->pPage1 != 0) {
    MemPage *pPage1 = pBt->pPage1;

    pBt->pPage1 = 0;
    releasePageOne(pPage1);
  }
}

int newDatabase(BtShared *pBt) {
  MemPage *pP1;
  unsigned char *data;
  int rc;

  if (pBt->nPage > 0) {
    return SQLITE_OK;
  }
  pP1 = pBt->pPage1;

  data = pP1->aData;
  rc = sqlite3PagerWrite(pP1->pDbPage);
  if (rc)
    return rc;
  memcpy(data, zMagicHeader, sizeof(zMagicHeader));

  data[16] = (u8)((pBt->pageSize >> 8) & 0xff);
  data[17] = (u8)((pBt->pageSize >> 16) & 0xff);
  data[18] = 1;
  data[19] = 1;

  data[20] = (u8)(pBt->pageSize - pBt->usableSize);
  data[21] = 64;
  data[22] = 32;
  data[23] = 32;
  memset(&data[24], 0, 100 - 24);
  zeroPage(pP1, 0x01 | 0x08 | 0x04);
  pBt->btsFlags |= 0x0002;

  sqlite3Put4byte(&data[36 + 4 * 4], pBt->autoVacuum);
  sqlite3Put4byte(&data[36 + 7 * 4], pBt->incrVacuum);

  pBt->nPage = 1;
  data[31] = 1;
  return SQLITE_OK;
}

int relocatePage(BtShared *pBt, MemPage *pDbPage, u8 eType, Pgno iPtrPage, Pgno iFreePage, int isCommit) {
  MemPage *pPtrPage;
  Pgno iDbPage = pDbPage->pgno;
  Pager *pPager = pBt->pPager;
  int rc;

  if (iDbPage < 3)
    return sqlite3CorruptError(77186);

  rc = sqlite3PagerMovepage(pPager, pDbPage->pDbPage, iFreePage, isCommit);
  if (rc != SQLITE_OK) {
    return rc;
  }
  pDbPage->pgno = iFreePage;

  if (eType == 5 || eType == 1) {
    rc = setChildPtrmaps(pDbPage);
    if (rc != SQLITE_OK) {
      return rc;
    }
  } else {
    Pgno nextOvfl = sqlite3Get4byte(pDbPage->aData);
    if (nextOvfl != 0) {
      ptrmapPut(pBt, nextOvfl, 4, iFreePage, &rc);
      if (rc != SQLITE_OK) {
        return rc;
      }
    }
  }

  if (eType != 1) {
    rc = btreeGetPage(pBt, iPtrPage, &pPtrPage, 0);
    if (rc != SQLITE_OK) {
      return rc;
    }
    rc = sqlite3PagerWrite(pPtrPage->pDbPage);
    if (rc != SQLITE_OK) {
      releasePage(pPtrPage);
      return rc;
    }
    rc = modifyPagePointer(pPtrPage, iDbPage, iFreePage, eType);
    releasePage(pPtrPage);
    if (rc == SQLITE_OK) {
      ptrmapPut(pBt, iFreePage, eType, iPtrPage, &rc);
    }
  }
  return rc;
}

int incrVacuumStep(BtShared *pBt, Pgno nFin, Pgno iLastPg, int bCommit) {
  Pgno nFreeList;
  int rc;

  if (!(ptrmapPageno((pBt), (iLastPg)) == (iLastPg)) &&
      iLastPg != ((Pgno)((sqlite3PendingByte / ((pBt)->pageSize)) + 1))) {
    u8 eType;
    Pgno iPtrPage;

    nFreeList = sqlite3Get4byte(&pBt->pPage1->aData[36]);
    if (nFreeList == 0) {
      return SQLITE_DONE;
    }

    rc = ptrmapGet(pBt, iLastPg, &eType, &iPtrPage);
    if (rc != SQLITE_OK) {
      return rc;
    }
    if (eType == 1) {
      return sqlite3CorruptError(77284);
    }

    if (eType == 2) {
      if (bCommit == 0) {
        Pgno iFreePg;
        MemPage *pFreePg;
        rc = allocateBtreePage(pBt, &pFreePg, &iFreePg, iLastPg, 1);
        if (rc != SQLITE_OK) {
          return rc;
        }

        releasePage(pFreePg);
      }
    } else {
      Pgno iFreePg;
      MemPage *pLastPg;
      u8 eMode = 0;
      Pgno iNear = 0;

      rc = btreeGetPage(pBt, iLastPg, &pLastPg, 0);
      if (rc != SQLITE_OK) {
        return rc;
      }

      if (bCommit == 0) {
        eMode = 2;
        iNear = nFin;
      }
      do {
        MemPage *pFreePg;
        Pgno dbSize = btreePagecount(pBt);
        rc = allocateBtreePage(pBt, &pFreePg, &iFreePg, iNear, eMode);
        if (rc != SQLITE_OK) {
          releasePage(pLastPg);
          return rc;
        }
        releasePage(pFreePg);
        if (iFreePg > dbSize) {
          releasePage(pLastPg);
          return sqlite3CorruptError(77336);
        }
      } while (bCommit && iFreePg > nFin);

      rc = relocatePage(pBt, pLastPg, eType, iPtrPage, iFreePg, bCommit);
      releasePage(pLastPg);
      if (rc != SQLITE_OK) {
        return rc;
      }
    }
  }

  if (bCommit == 0) {
    do {
      iLastPg--;
    } while (iLastPg == ((Pgno)((sqlite3PendingByte / ((pBt)->pageSize)) + 1)) ||
             (ptrmapPageno((pBt), (iLastPg)) == (iLastPg)));
    pBt->bDoTruncate = 1;
    pBt->nPage = iLastPg;
  }
  return SQLITE_OK;
}

Pgno finalDbSize(BtShared *pBt, Pgno nOrig, Pgno nFree) {
  int nEntry;
  Pgno nPtrmap;
  Pgno nFin;

  nEntry = pBt->usableSize / 5;
  nPtrmap = (nFree - nOrig + ptrmapPageno(pBt, nOrig) + nEntry) / nEntry;
  nFin = nOrig - nFree - nPtrmap;
  if (nOrig > ((Pgno)((sqlite3PendingByte / ((pBt)->pageSize)) + 1)) &&
      nFin < ((Pgno)((sqlite3PendingByte / ((pBt)->pageSize)) + 1))) {
    nFin--;
  }
  while ((ptrmapPageno((pBt), (nFin)) == (nFin)) || nFin == ((Pgno)((sqlite3PendingByte / ((pBt)->pageSize)) + 1))) {
    nFin--;
  }

  return nFin;
}

void btreeSetNPage(BtShared *pBt, MemPage *pPage1) {
  int nPage = sqlite3Get4byte(&pPage1->aData[28]);
  if (nPage == 0)
    sqlite3PagerPagecount(pBt->pPager, &nPage);
  pBt->nPage = nPage;
}

int getOverflowPage(BtShared *pBt, Pgno ovfl, MemPage **ppPage, Pgno *pPgnoNext) {
  Pgno next = 0;
  MemPage *pPage = 0;
  int rc = SQLITE_OK;

  if (pBt->autoVacuum) {
    Pgno pgno;
    Pgno iGuess = ovfl + 1;
    u8 eType;

    while ((ptrmapPageno((pBt), (iGuess)) == (iGuess)) ||
           iGuess == ((Pgno)((sqlite3PendingByte / ((pBt)->pageSize)) + 1))) {
      iGuess++;
    }

    if (iGuess <= btreePagecount(pBt)) {
      rc = ptrmapGet(pBt, iGuess, &eType, &pgno);
      if (rc == SQLITE_OK && eType == 4 && pgno == ovfl) {
        next = iGuess;
        rc = SQLITE_DONE;
      }
    }
  }

  if (rc == SQLITE_OK) {
    rc = btreeGetPage(pBt, ovfl, &pPage, (ppPage == 0) ? 0x02 : 0);

    if (rc == SQLITE_OK) {
      next = sqlite3Get4byte(pPage->aData);
    }
  }

  *pPgnoNext = next;
  if (ppPage) {
    *ppPage = pPage;
  } else {
    releasePage(pPage);
  }
  return (rc == SQLITE_DONE ? SQLITE_OK : rc);
}

int allocateBtreePage(BtShared *pBt, MemPage **ppPage, Pgno *pPgno, Pgno nearby, u8 eMode) {
  MemPage *pPage1;
  int rc;
  u32 n;
  u32 k;
  MemPage *pTrunk = 0;
  MemPage *pPrevTrunk = 0;
  Pgno mxPage;

  pPage1 = pBt->pPage1;
  mxPage = btreePagecount(pBt);

  n = sqlite3Get4byte(&pPage1->aData[36]);
  if (n >= mxPage) {
    return sqlite3CorruptError(79763);
  }
  if (n > 0) {
    Pgno iTrunk;
    u8 searchList = 0;
    u32 nSearch = 0;

    if (eMode == 1) {
      if (nearby <= mxPage) {
        u8 eType;

        rc = ptrmapGet(pBt, nearby, &eType, 0);
        if (rc)
          return rc;
        if (eType == 2) {
          searchList = 1;
        }
      }
    } else if (eMode == 2) {
      searchList = 1;
    }

    rc = sqlite3PagerWrite(pPage1->pDbPage);
    if (rc)
      return rc;
    sqlite3Put4byte(&pPage1->aData[36], n - 1);

    do {
      pPrevTrunk = pTrunk;
      if (pPrevTrunk) {
        iTrunk = sqlite3Get4byte(&pPrevTrunk->aData[0]);
      } else {
        iTrunk = sqlite3Get4byte(&pPage1->aData[32]);
      };
      if (iTrunk > mxPage || nSearch++ > n) {
        rc = sqlite3CorruptError(79819);
      } else {
        rc = btreeGetUnusedPage(pBt, iTrunk, &pTrunk, 0);
      }
      if (rc) {
        pTrunk = 0;
        goto end_allocate_page;
      }

      k = sqlite3Get4byte(&pTrunk->aData[4]);
      if (k == 0 && !searchList) {
        rc = sqlite3PagerWrite(pTrunk->pDbPage);
        if (rc) {
          goto end_allocate_page;
        }
        *pPgno = iTrunk;
        memcpy(&pPage1->aData[32], &pTrunk->aData[0], 4);
        *ppPage = pTrunk;
        pTrunk = 0;
      } else if (k > (u32)(pBt->usableSize / 4 - 2)) {
        rc = sqlite3CorruptError(79848);
        goto end_allocate_page;

      } else if (searchList && (nearby == iTrunk || (iTrunk < nearby && eMode == 2))) {
        *pPgno = iTrunk;
        *ppPage = pTrunk;
        searchList = 0;
        rc = sqlite3PagerWrite(pTrunk->pDbPage);
        if (rc) {
          goto end_allocate_page;
        }
        if (k == 0) {
          if (!pPrevTrunk) {
            memcpy(&pPage1->aData[32], &pTrunk->aData[0], 4);
          } else {
            rc = sqlite3PagerWrite(pPrevTrunk->pDbPage);
            if (rc != SQLITE_OK) {
              goto end_allocate_page;
            }
            memcpy(&pPrevTrunk->aData[0], &pTrunk->aData[0], 4);
          }
        } else {
          MemPage *pNewTrunk;
          Pgno iNewTrunk = sqlite3Get4byte(&pTrunk->aData[8]);
          if (iNewTrunk > mxPage) {
            rc = sqlite3CorruptError(79882);
            goto end_allocate_page;
          };
          rc = btreeGetUnusedPage(pBt, iNewTrunk, &pNewTrunk, 0);
          if (rc != SQLITE_OK) {
            goto end_allocate_page;
          }
          rc = sqlite3PagerWrite(pNewTrunk->pDbPage);
          if (rc != SQLITE_OK) {
            releasePage(pNewTrunk);
            goto end_allocate_page;
          }
          memcpy(&pNewTrunk->aData[0], &pTrunk->aData[0], 4);
          sqlite3Put4byte(&pNewTrunk->aData[4], k - 1);
          memcpy(&pNewTrunk->aData[8], &pTrunk->aData[12], (k - 1) * 4);
          releasePage(pNewTrunk);
          if (!pPrevTrunk) {
            sqlite3Put4byte(&pPage1->aData[32], iNewTrunk);
          } else {
            rc = sqlite3PagerWrite(pPrevTrunk->pDbPage);
            if (rc) {
              goto end_allocate_page;
            }
            sqlite3Put4byte(&pPrevTrunk->aData[0], iNewTrunk);
          }
        }
        pTrunk = 0;

      } else if (k > 0) {
        u32 closest;
        Pgno iPage;
        unsigned char *aData = pTrunk->aData;
        if (nearby > 0) {
          u32 i;
          closest = 0;
          if (eMode == 2) {
            for (i = 0; i < k; i++) {
              iPage = sqlite3Get4byte(&aData[8 + i * 4]);
              if (iPage <= nearby) {
                closest = i;
                break;
              }
            }
          } else {
            int dist;
            dist = sqlite3AbsInt32(sqlite3Get4byte(&aData[8]) - nearby);
            for (i = 1; i < k; i++) {
              int d2 = sqlite3AbsInt32(sqlite3Get4byte(&aData[8 + i * 4]) - nearby);
              if (d2 < dist) {
                closest = i;
                dist = d2;
              }
            }
          }
        } else {
          closest = 0;
        }

        iPage = sqlite3Get4byte(&aData[8 + closest * 4]);
        if (iPage > mxPage || iPage < 2) {
          rc = sqlite3CorruptError(79947);
          goto end_allocate_page;
        };
        if (!searchList || (iPage == nearby || (iPage < nearby && eMode == 2))) {
          int noContent;
          *pPgno = iPage;

          rc = sqlite3PagerWrite(pTrunk->pDbPage);
          if (rc)
            goto end_allocate_page;
          if (closest < k - 1) {
            memcpy(&aData[8 + closest * 4], &aData[4 + k * 4], 4);
          }
          sqlite3Put4byte(&aData[4], k - 1);
          noContent = !btreeGetHasContent(pBt, *pPgno) ? 0x01 : 0;
          rc = btreeGetUnusedPage(pBt, *pPgno, ppPage, noContent);
          if (rc == SQLITE_OK) {
            rc = sqlite3PagerWrite((*ppPage)->pDbPage);
            if (rc != SQLITE_OK) {
              releasePage(*ppPage);
              *ppPage = 0;
            }
          }
          searchList = 0;
        }
      }
      releasePage(pPrevTrunk);
      pPrevTrunk = 0;
    } while (searchList);
  } else {
    int bNoContent = (0 == (pBt->bDoTruncate)) ? 0x01 : 0;

    rc = sqlite3PagerWrite(pBt->pPage1->pDbPage);
    if (rc)
      return rc;
    pBt->nPage++;
    if (pBt->nPage == ((Pgno)((sqlite3PendingByte / ((pBt)->pageSize)) + 1)))
      pBt->nPage++;

    if (pBt->autoVacuum && (ptrmapPageno((pBt), (pBt->nPage)) == (pBt->nPage))) {
      MemPage *pPg = 0;

      rc = btreeGetUnusedPage(pBt, pBt->nPage, &pPg, bNoContent);
      if (rc == SQLITE_OK) {
        rc = sqlite3PagerWrite(pPg->pDbPage);
        releasePage(pPg);
      }
      if (rc)
        return rc;
      pBt->nPage++;
      if (pBt->nPage == ((Pgno)((sqlite3PendingByte / ((pBt)->pageSize)) + 1))) {
        pBt->nPage++;
      }
    }

    sqlite3Put4byte(28 + (u8 *)pBt->pPage1->aData, pBt->nPage);
    *pPgno = pBt->nPage;

    rc = btreeGetUnusedPage(pBt, *pPgno, ppPage, bNoContent);
    if (rc)
      return rc;
    rc = sqlite3PagerWrite((*ppPage)->pDbPage);
    if (rc != SQLITE_OK) {
      releasePage(*ppPage);
      *ppPage = 0;
    };
  }

end_allocate_page:
  releasePage(pTrunk);
  releasePage(pPrevTrunk);

  return rc;
}

int freePage2(BtShared *pBt, MemPage *pMemPage, Pgno iPage) {
  MemPage *pTrunk = 0;
  Pgno iTrunk = 0;
  MemPage *pPage1 = pBt->pPage1;
  MemPage *pPage;
  int rc;
  u32 nFree;

  if (iPage < 2 || iPage > pBt->nPage) {
    return sqlite3CorruptError(80074);
  }
  if (pMemPage) {
    pPage = pMemPage;
    sqlite3PagerRef(pPage->pDbPage);
  } else {
    pPage = btreePageLookup(pBt, iPage);
  }

  rc = sqlite3PagerWrite(pPage1->pDbPage);
  if (rc)
    goto freepage_out;
  nFree = sqlite3Get4byte(&pPage1->aData[36]);
  sqlite3Put4byte(&pPage1->aData[36], nFree + 1);

  if (pBt->btsFlags & 0x0004) {
    if ((!pPage && ((rc = btreeGetPage(pBt, iPage, &pPage, 0)) != 0)) ||
        ((rc = sqlite3PagerWrite(pPage->pDbPage)) != 0)) {
      goto freepage_out;
    }
    memset(pPage->aData, 0, pPage->pBt->pageSize);
  }

  if ((pBt->autoVacuum)) {
    ptrmapPut(pBt, iPage, 2, 0, &rc);
    if (rc)
      goto freepage_out;
  }

  if (nFree != 0) {
    u32 nLeaf;

    iTrunk = sqlite3Get4byte(&pPage1->aData[32]);
    if (iTrunk > btreePagecount(pBt)) {
      rc = sqlite3CorruptError(80121);
      goto freepage_out;
    }
    rc = btreeGetPage(pBt, iTrunk, &pTrunk, 0);
    if (rc != SQLITE_OK) {
      goto freepage_out;
    }

    nLeaf = sqlite3Get4byte(&pTrunk->aData[4]);

    if (nLeaf > (u32)pBt->usableSize / 4 - 2) {
      rc = sqlite3CorruptError(80132);
      goto freepage_out;
    }
    if (nLeaf < (u32)pBt->usableSize / 4 - 8) {
      rc = sqlite3PagerWrite(pTrunk->pDbPage);
      if (rc == SQLITE_OK) {
        sqlite3Put4byte(&pTrunk->aData[4], nLeaf + 1);
        sqlite3Put4byte(&pTrunk->aData[8 + nLeaf * 4], iPage);
        if (pPage && (pBt->btsFlags & 0x0004) == 0) {
          sqlite3PagerDontWrite(pPage->pDbPage);
        }
        rc = btreeSetHasContent(pBt, iPage);
      };
      goto freepage_out;
    }
  }

  if (pPage == 0 && SQLITE_OK != (rc = btreeGetPage(pBt, iPage, &pPage, 0))) {
    goto freepage_out;
  }
  rc = sqlite3PagerWrite(pPage->pDbPage);
  if (rc != SQLITE_OK) {
    goto freepage_out;
  }
  sqlite3Put4byte(pPage->aData, iTrunk);
  sqlite3Put4byte(&pPage->aData[4], 0);
  sqlite3Put4byte(&pPage1->aData[32], iPage);

freepage_out:
  if (pPage) {
    pPage->isInit = 0;
  }
  releasePage(pPage);
  releasePage(pTrunk);
  return rc;
}

int clearDatabasePage(BtShared *pBt, Pgno pgno, int freePageFlag, i64 *pnChange) {
  MemPage *pPage;
  int rc;
  unsigned char *pCell;
  int i;
  int hdr;
  CellInfo info;

  if (pgno > btreePagecount(pBt)) {
    return sqlite3CorruptError(83451);
  }
  rc = getAndInitPage(pBt, pgno, &pPage, 0);
  if (rc)
    return rc;
  if ((pBt->openFlags & 4) == 0 && sqlite3PagerPageRefcount(pPage->pDbPage) != (1 + (pgno == 1))) {
    rc = sqlite3CorruptError(83458);
    goto cleardatabasepage_out;
  }
  hdr = pPage->hdrOffset;
  for (i = 0; i < pPage->nCell; i++) {
    pCell = ((pPage)->aData + ((pPage)->maskPage & __builtin_bswap16(*(u16 *)(&(pPage)->aCellIdx[2 * (i)]))));
    if (!pPage->leaf) {
      rc = clearDatabasePage(pBt, sqlite3Get4byte(pCell), 1, pnChange);
      if (rc)
        goto cleardatabasepage_out;
    }
    pPage->xParseCell(pPage, pCell, &info);
    if (info.nLocal != info.nPayload) {
      rc = clearCellOverflow(pPage, pCell, &info);
    } else {
      rc = 0;
    };
    if (rc)
      goto cleardatabasepage_out;
  }
  if (!pPage->leaf) {
    rc = clearDatabasePage(pBt, sqlite3Get4byte(&pPage->aData[hdr + 8]), 1, pnChange);
    if (rc)
      goto cleardatabasepage_out;
    if (pPage->intKey)
      pnChange = 0;
  }
  if (pnChange) {
    *pnChange += pPage->nCell;
  }
  if (freePageFlag) {
    freePage(pPage, &rc);
  } else if ((rc = sqlite3PagerWrite(pPage->pDbPage)) == 0) {
    zeroPage(pPage, pPage->aData[hdr] | 0x08);
  }

cleardatabasepage_out:
  releasePage(pPage);
  return rc;
}