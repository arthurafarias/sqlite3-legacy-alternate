#define _GNU_SOURCE 1
#include <stdio.h>
#include "sqlite/IntegrityCk.h"
#include "sqlite/BtShared.h"
#include "sqlite/Btree.h"
#include "sqlite/CellInfo.h"
#include "sqlite/DbPage.h"
#include "sqlite/MemPage.h"
#include "sqlite/Pager.h"
#include "sqlite/Pgno.h"
#include "sqlite/StrAccum.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_str.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
#include "sqlite/SqliteResultCode.h"
void checkOom(IntegrityCk *pCheck) {
  pCheck->rc = SQLITE_NOMEM;
  pCheck->mxErr = 0;
  if (pCheck->nErr == 0)
    pCheck->nErr++;
}

void checkProgress(IntegrityCk *pCheck) {
  sqlite3 *db = pCheck->db;
  if (__atomic_load_n((&db->u1.isInterrupted), 0)) {
    pCheck->rc = SQLITE_INTERRUPT;
    pCheck->nErr++;
    pCheck->mxErr = 0;
  }

  if (db->xProgress) {
    pCheck->nStep++;
    if ((pCheck->nStep % db->nProgressOps) == 0 && db->xProgress(db->pProgressArg)) {
      pCheck->rc = SQLITE_INTERRUPT;
      pCheck->nErr++;
      pCheck->mxErr = 0;
    }
  }
}

void checkAppendMsg(IntegrityCk *pCheck, const char *zFormat, ...) {
  va_list ap;
  checkProgress(pCheck);
  if (!pCheck->mxErr)
    return;
  pCheck->mxErr--;
  pCheck->nErr++;

  va_start(ap, zFormat);
  if (pCheck->errMsg.nChar) {
    sqlite3_str_append(&pCheck->errMsg, "\n", 1);
  }
  if (pCheck->zPfx) {
    sqlite3_str_appendf(&pCheck->errMsg, pCheck->zPfx, pCheck->v0, pCheck->v1, pCheck->v2);
  }
  sqlite3_str_vappendf(&pCheck->errMsg, zFormat, ap);

  va_end(ap);
  if (pCheck->errMsg.accError == SQLITE_NOMEM) {
    checkOom(pCheck);
  }
}

int getPageReferenced(IntegrityCk *pCheck, Pgno iPg) {
  return (pCheck->aPgRef[iPg / 8] & (1 << (iPg & 0x07)));
}

void setPageReferenced(IntegrityCk *pCheck, Pgno iPg) {
  pCheck->aPgRef[iPg / 8] |= (1 << (iPg & 0x07));
}

int checkRef(IntegrityCk *pCheck, Pgno iPage) {
  if (iPage > pCheck->nCkPage || iPage == 0) {
    checkAppendMsg(pCheck, "invalid page number %u", iPage);
    return 1;
  }
  if (getPageReferenced(pCheck, iPage)) {
    checkAppendMsg(pCheck, "2nd reference to page %u", iPage);
    return 1;
  }
  setPageReferenced(pCheck, iPage);
  return 0;
}

void checkPtrmap(IntegrityCk *pCheck, Pgno iChild, u8 eType, Pgno iParent) {
  int rc;
  u8 ePtrmapType;
  Pgno iPtrmapParent;

  rc = ptrmapGet(pCheck->pBt, iChild, &ePtrmapType, &iPtrmapParent);
  if (rc != SQLITE_OK) {
    if (rc == SQLITE_NOMEM || rc == (10 | (12 << 8)))
      checkOom(pCheck);
    checkAppendMsg(pCheck, "Failed to read ptrmap key=%u", iChild);
    return;
  }

  if (ePtrmapType != eType || iPtrmapParent != iParent) {
    checkAppendMsg(pCheck, "Bad ptr map entry key=%u expected=(%u,%u) got=(%u,%u)", iChild, eType, iParent, ePtrmapType,
                   iPtrmapParent);
  }
}

void checkList(IntegrityCk *pCheck, int isFreeList, Pgno iPage, u32 N) {
  int i;
  u32 expected = N;
  int nErrAtStart = pCheck->nErr;
  while (iPage != 0 && pCheck->mxErr) {
    DbPage *pOvflPage;
    unsigned char *pOvflData;
    if (checkRef(pCheck, iPage))
      break;
    N--;
    if (sqlite3PagerGet(pCheck->pPager, (Pgno)iPage, &pOvflPage, 0)) {
      checkAppendMsg(pCheck, "failed to get page %u", iPage);
      break;
    }
    pOvflData = (unsigned char *)sqlite3PagerGetData(pOvflPage);
    if (isFreeList) {
      u32 n = (u32)sqlite3Get4byte(&pOvflData[4]);

      if (pCheck->pBt->autoVacuum) {
        checkPtrmap(pCheck, iPage, 2, 0);
      }

      if (n > pCheck->pBt->usableSize / 4 - 2) {
        checkAppendMsg(pCheck, "freelist leaf count too big on page %u", iPage);
        N--;
      } else {
        for (i = 0; i < (int)n; i++) {
          Pgno iFreePage = sqlite3Get4byte(&pOvflData[8 + i * 4]);

          if (pCheck->pBt->autoVacuum) {
            checkPtrmap(pCheck, iFreePage, 2, 0);
          }

          checkRef(pCheck, iFreePage);
        }
        N -= n;
      }
    }

    else {
      if (pCheck->pBt->autoVacuum && N > 0) {
        i = sqlite3Get4byte(pOvflData);
        checkPtrmap(pCheck, i, 4, iPage);
      }
    }

    iPage = sqlite3Get4byte(pOvflData);
    sqlite3PagerUnref(pOvflPage);
  }
  if (N && nErrAtStart == pCheck->nErr) {
    checkAppendMsg(pCheck, "%s is %u but should be %u", isFreeList ? "size" : "overflow list length", expected - N,
                   expected);
  }
}

int checkTreePage(IntegrityCk *pCheck, Pgno iPage, i64 *piMinKey, i64 maxKey) {
  MemPage *pPage = 0;
  int i;
  int rc;
  int depth = -1, d2;
  int pgno;
  int nFrag;
  int hdr;
  int cellStart;
  int nCell;
  int doCoverageCheck = 1;
  int keyCanBeEqual = 1;

  u8 *data;
  u8 *pCell;
  u8 *pCellIdx;
  BtShared *pBt;
  u32 pc;
  u32 usableSize;
  u32 contentOffset;
  u32 *heap = 0;
  u32 x, prev = 0;
  const char *saved_zPfx = pCheck->zPfx;
  int saved_v1 = pCheck->v1;
  int saved_v2 = pCheck->v2;
  u8 savedIsInit = 0;

  checkProgress(pCheck);
  if (pCheck->mxErr == 0)
    goto end_of_check;
  pBt = pCheck->pBt;
  usableSize = pBt->usableSize;
  if (iPage == 0)
    return 0;
  if (checkRef(pCheck, iPage))
    return 0;
  pCheck->zPfx = "Tree %u page %u: ";
  pCheck->v1 = iPage;
  if ((rc = btreeGetPage(pBt, iPage, &pPage, 0)) != 0) {
    checkAppendMsg(pCheck, "unable to get the page. error code=%d", rc);
    if (rc == (10 | (12 << 8)))
      pCheck->rc = SQLITE_NOMEM;
    goto end_of_check;
  }

  savedIsInit = pPage->isInit;
  pPage->isInit = 0;
  if ((rc = btreeInitPage(pPage)) != 0) {
    checkAppendMsg(pCheck, "btreeInitPage() returns error code %d", rc);
    goto end_of_check;
  }
  if ((rc = btreeComputeFreeSpace(pPage)) != 0) {
    checkAppendMsg(pCheck, "free space corruption", rc);
    goto end_of_check;
  }
  data = pPage->aData;
  hdr = pPage->hdrOffset;

  pCheck->zPfx = "Tree %u page %u cell %u: ";
  contentOffset = (((((int)((&data[hdr + 5])[0] << 8 | (&data[hdr + 5])[1])) - 1) & 0xffff) + 1);

  nCell = ((&data[hdr + 3])[0] << 8 | (&data[hdr + 3])[1]);

  if (pPage->leaf || pPage->intKey == 0) {
    pCheck->nRow += nCell;
  }

  cellStart = hdr + 12 - 4 * pPage->leaf;

  pCellIdx = &data[cellStart + 2 * (nCell - 1)];

  if (!pPage->leaf) {
    pgno = sqlite3Get4byte(&data[hdr + 8]);

    if (pBt->autoVacuum) {
      pCheck->zPfx = "Tree %u page %u right child: ";
      checkPtrmap(pCheck, pgno, 5, iPage);
    }

    depth = checkTreePage(pCheck, pgno, &maxKey, maxKey);
    keyCanBeEqual = 0;
  } else {
    heap = pCheck->heap;
    heap[0] = 0;
  }

  for (i = nCell - 1; i >= 0 && pCheck->mxErr; i--) {
    CellInfo info;

    pCheck->v2 = i;

    pc = __builtin_bswap16(*(u16 *)(pCellIdx));
    pCellIdx -= 2;
    if (pc < contentOffset || pc > usableSize - 4) {
      checkAppendMsg(pCheck, "Offset %u out of range %u..%u", pc, contentOffset, usableSize - 4);
      doCoverageCheck = 0;
      continue;
    }
    pCell = &data[pc];
    pPage->xParseCell(pPage, pCell, &info);
    if (pc + info.nSize > usableSize) {
      checkAppendMsg(pCheck, "Extends off end of page");
      doCoverageCheck = 0;
      continue;
    }

    if (pPage->intKey) {
      if (keyCanBeEqual ? (info.nKey > maxKey) : (info.nKey >= maxKey)) {
        checkAppendMsg(pCheck, "Rowid %lld out of order", info.nKey);
      }
      maxKey = info.nKey;
      keyCanBeEqual = 0;
    }

    if (info.nPayload > info.nLocal) {
      u32 nPage;
      Pgno pgnoOvfl;

      nPage = (info.nPayload - info.nLocal + usableSize - 5) / (usableSize - 4);
      pgnoOvfl = sqlite3Get4byte(&pCell[info.nSize - 4]);

      if (pBt->autoVacuum) {
        checkPtrmap(pCheck, pgnoOvfl, 3, iPage);
      }

      checkList(pCheck, 0, pgnoOvfl, nPage);
    }

    if (!pPage->leaf) {
      pgno = sqlite3Get4byte(pCell);

      if (pBt->autoVacuum) {
        checkPtrmap(pCheck, pgno, 5, iPage);
      }

      d2 = checkTreePage(pCheck, pgno, &maxKey, maxKey);
      keyCanBeEqual = 0;
      if (d2 != depth) {
        checkAppendMsg(pCheck, "Child page depth differs");
        depth = d2;
      }
    } else {
      btreeHeapInsert(heap, (pc << 16) | (pc + info.nSize - 1));
    }
  }
  *piMinKey = maxKey;

  pCheck->zPfx = 0;
  if (doCoverageCheck && pCheck->mxErr > 0) {
    if (!pPage->leaf) {
      heap = pCheck->heap;
      heap[0] = 0;
      for (i = nCell - 1; i >= 0; i--) {
        u32 size;
        pc = __builtin_bswap16(*(u16 *)(&data[cellStart + i * 2]));
        size = pPage->xCellSize(pPage, &data[pc]);

        btreeHeapInsert(heap, (pc << 16) | (pc + size - 1));
      }
    }

    i = ((&data[hdr + 1])[0] << 8 | (&data[hdr + 1])[1]);
    while (i > 0) {
      int size, j;

      size = ((&data[i + 2])[0] << 8 | (&data[i + 2])[1]);

      btreeHeapInsert(heap, (((u32)i) << 16) | (i + size - 1));

      j = ((&data[i])[0] << 8 | (&data[i])[1]);

      i = j;
    }

    nFrag = 0;
    prev = contentOffset - 1;
    while (btreeHeapPull(heap, &x)) {
      if ((prev & 0xffff) >= (x >> 16)) {
        checkAppendMsg(pCheck, "Multiple uses for byte %u of page %u", x >> 16, iPage);
        break;
      } else {
        nFrag += (x >> 16) - (prev & 0xffff) - 1;
        prev = x;
      }
    }
    nFrag += usableSize - (prev & 0xffff) - 1;

    if (heap[0] == 0 && nFrag != data[hdr + 7]) {
      checkAppendMsg(pCheck, "Fragmentation of %u bytes reported as %u on page %u", nFrag, data[hdr + 7], iPage);
    }
  }

end_of_check:
  if (!doCoverageCheck)
    pPage->isInit = savedIsInit;
  releasePage(pPage);
  pCheck->zPfx = saved_zPfx;
  pCheck->v1 = saved_v1;
  pCheck->v2 = saved_v2;
  return depth + 1;
}
