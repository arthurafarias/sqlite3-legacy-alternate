#define _GNU_SOURCE 1
#include <string.h>
#include <stddef.h>
#include "sqlite/BtCursor.h"
#include "sqlite/BtShared.h"
#include "sqlite/Btree.h"
#include "sqlite/BtreePayload.h"
#include "sqlite/CellInfo.h"
#include "sqlite/CollSeq.h"
#include "sqlite/Column.h"
#include "sqlite/DbPage.h"
#include "sqlite/Index.h"
#include "sqlite/KeyInfo.h"
#include "sqlite/Mem.h"
#include "sqlite/MemPage.h"
#include "sqlite/Pager.h"
#include "sqlite/Pgno.h"
#include "sqlite/RecordCompare.h"
#include "sqlite/Table.h"
#include "sqlite/UnpackedRecord.h"
#include "sqlite/Vdbe.h"
#include "sqlite/i16.h"
#include "sqlite/i64.h"
#include "sqlite/i8.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_file.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_value.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/uptr.h"
#include "sqlite/SqliteResultCode.h"
/* Private helpers, formerly declared in _Uncategorized.h. */
static int copyPayload(void *pPayload, void *pBuf, int nByte, int eOp, DbPage *pDbPage);

static int copyPayload(void *pPayload, void *pBuf, int nByte, int eOp, DbPage *pDbPage) {
  if (eOp) {
    int rc = sqlite3PagerWrite(pDbPage);
    if (rc != SQLITE_OK) {
      return rc;
    }
    memcpy(pPayload, pBuf, nByte);
  } else {
    memcpy(pBuf, pPayload, nByte);
  }
  return SQLITE_OK;
}

void sqlite3BtreeEnterCursor(BtCursor *pCur) {
  sqlite3BtreeEnter(pCur->pBtree);
}

void sqlite3BtreeLeaveCursor(BtCursor *pCur) {
  sqlite3BtreeLeave(pCur->pBtree);
}

void btreeReleaseAllCursorPages(BtCursor *pCur) {
  int i;
  if (pCur->iPage >= 0) {
    for (i = 0; i < pCur->iPage; i++) {
      releasePageNotNull(pCur->apPage[i]);
    }
    releasePageNotNull(pCur->pPage);
    pCur->iPage = -1;
  }
}

int saveCursorKey(BtCursor *pCur) {
  int rc = 0;

  if (pCur->curIntKey) {
    pCur->nKey = sqlite3BtreeIntegerKey(pCur);
  } else {
    void *pKey;
    pCur->nKey = sqlite3BtreePayloadSize(pCur);
    pKey = sqlite3Malloc(((i64)pCur->nKey) + 9 + 8);
    if (pKey) {
      rc = sqlite3BtreePayload(pCur, 0, (int)pCur->nKey, pKey);
      if (rc == SQLITE_OK) {
        memset(((u8 *)pKey) + pCur->nKey, 0, 9 + 8);
        pCur->pKey = pKey;
      } else {
        sqlite3_free(pKey);
      }
    } else {
      rc = 7;
    }
  }

  return rc;
}

int saveCursorPosition(BtCursor *pCur) {
  int rc;

  if (pCur->curFlags & 0x40) {
    return (19 | (11 << 8));
  }
  if (pCur->eState == 2) {
    pCur->eState = 0;
  } else {
    pCur->skipNext = 0;
  }

  rc = saveCursorKey(pCur);
  if (rc == SQLITE_OK) {
    btreeReleaseAllCursorPages(pCur);
    pCur->eState = 3;
  }

  pCur->curFlags &= ~(0x02 | 0x04 | 0x08);
  return rc;
}

int __attribute__((noinline)) saveCursorsOnList(BtCursor *p, Pgno iRoot, BtCursor *pExcept) {
  do {
    if (p != pExcept && (0 == iRoot || p->pgnoRoot == iRoot)) {
      if (p->eState == 0 || p->eState == 2) {
        int rc = saveCursorPosition(p);
        if (SQLITE_OK != rc) {
          return rc;
        }
      } else {
        btreeReleaseAllCursorPages(p);
      }
    }
    p = p->pNext;
  } while (p);
  return SQLITE_OK;
}

void sqlite3BtreeClearCursor(BtCursor *pCur) {
  sqlite3_free(pCur->pKey);
  pCur->pKey = 0;
  pCur->eState = 1;
}

int btreeMoveto(BtCursor *pCur, const void *pKey, i64 nKey, int bias, int *pRes) {
  int rc;
  UnpackedRecord *pIdxKey;

  if (pKey) {
    KeyInfo *pKeyInfo = pCur->pKeyInfo;

    pIdxKey = sqlite3VdbeAllocUnpackedRecord(pKeyInfo);
    if (pIdxKey == 0)
      return 7;
    sqlite3VdbeRecordUnpack((int)nKey, pKey, pIdxKey);
    if (pIdxKey->nField == 0 || pIdxKey->nField > pKeyInfo->nAllField) {
      rc = sqlite3CorruptError(74102);
    } else {
      rc = sqlite3BtreeIndexMoveto(pCur, pIdxKey, pRes);
    }
    sqlite3DbFree(pCur->pKeyInfo->db, pIdxKey);
  } else {
    pIdxKey = 0;
    rc = sqlite3BtreeTableMoveto(pCur, nKey, bias, pRes);
  }
  return rc;
}

int btreeRestoreCursorPosition(BtCursor *pCur) {
  int rc;
  int skipNext = 0;

  if (pCur->eState == 4) {
    return pCur->skipNext;
  }
  pCur->eState = 1;
  if (sqlite3FaultSim(410)) {
    rc = SQLITE_IOERR;
  } else {
    rc = btreeMoveto(pCur, pCur->pKey, pCur->nKey, 0, &skipNext);
  }
  if (rc == SQLITE_OK) {
    sqlite3_free(pCur->pKey);
    pCur->pKey = 0;

    if (skipNext)
      pCur->skipNext = skipNext;
    if (pCur->skipNext && pCur->eState == 0) {
      pCur->eState = 2;
    }
  }
  return rc;
}

int sqlite3BtreeCursorHasMoved(BtCursor *pCur) {
  return 0 != *(u8 *)pCur;
}

int sqlite3BtreeCursorRestore(BtCursor *pCur, int *pDifferentRow) {
  int rc;

  rc = (pCur->eState >= 3 ? btreeRestoreCursorPosition(pCur) : 0);
  if (rc) {
    *pDifferentRow = 1;
    return rc;
  }
  if (pCur->eState != 0) {
    *pDifferentRow = 1;
  } else {
    *pDifferentRow = 0;
  }
  return SQLITE_OK;
}

void sqlite3BtreeCursorHintFlags(BtCursor *pCur, unsigned x) {
  pCur->hints = (u8)x;
}

void sqlite3BtreeCursorZero(BtCursor *p) {
  memset(p, 0, offsetof(BtCursor, pBt));
}

int sqlite3BtreeCloseCursor(BtCursor *pCur) {
  Btree *pBtree = pCur->pBtree;
  if (pBtree) {
    BtShared *pBt = pCur->pBt;
    sqlite3BtreeEnter(pBtree);

    if (pBt->pCursor == pCur) {
      pBt->pCursor = pCur->pNext;
    } else {
      BtCursor *pPrev = pBt->pCursor;
      do {
        if (pPrev->pNext == pCur) {
          pPrev->pNext = pCur->pNext;
          break;
        }
        pPrev = pPrev->pNext;
      } while ((pPrev));
    }
    btreeReleaseAllCursorPages(pCur);
    unlockBtreeIfUnused(pBt);
    sqlite3_free(pCur->aOverflow);
    sqlite3_free(pCur->pKey);
    if ((pBt->openFlags & 4) && pBt->pCursor == 0) {
      sqlite3BtreeClose(pBtree);
    } else {
      sqlite3BtreeLeave(pBtree);
    }
    pCur->pBtree = 0;
  }
  return SQLITE_OK;
}

__attribute__((noinline)) void getCellInfo(BtCursor *pCur) {
  if (pCur->info.nSize == 0) {
    pCur->curFlags |= 0x02;
    btreeParseCell(pCur->pPage, pCur->ix, &pCur->info);
  } else {
  }
}

int sqlite3BtreeCursorIsValidNN(BtCursor *pCur) {
  return pCur->eState == 0;
}

i64 sqlite3BtreeIntegerKey(BtCursor *pCur) {
  getCellInfo(pCur);
  return pCur->info.nKey;
}

void sqlite3BtreeCursorPin(BtCursor *pCur) {
  pCur->curFlags |= 0x40;
}

void sqlite3BtreeCursorUnpin(BtCursor *pCur) {
  pCur->curFlags &= ~0x40;
}

i64 sqlite3BtreeOffset(BtCursor *pCur) {
  getCellInfo(pCur);
  return (i64)pCur->pBt->pageSize * ((i64)pCur->pPage->pgno - 1) + (i64)(pCur->info.pPayload - pCur->pPage->aData);
}

u32 sqlite3BtreePayloadSize(BtCursor *pCur) {
  getCellInfo(pCur);
  return pCur->info.nPayload;
}

sqlite3_int64 sqlite3BtreeMaxRecordSize(BtCursor *pCur) {
  return pCur->pBt->pageSize * (sqlite3_int64)pCur->pBt->nPage;
}

int accessPayload(BtCursor *pCur, u32 offset, u32 amt, unsigned char *pBuf, int eOp) {
  unsigned char *aPayload;
  int rc = SQLITE_OK;
  int iIdx = 0;
  MemPage *pPage = pCur->pPage;
  BtShared *pBt = pCur->pBt;

  unsigned char *const pBufStart = pBuf;

  if (pCur->ix >= pPage->nCell) {
    return sqlite3CorruptError(78370);
  }

  getCellInfo(pCur);
  aPayload = pCur->info.pPayload;

  if ((uptr)(aPayload - pPage->aData) > (pBt->usableSize - pCur->info.nLocal)) {
    return sqlite3CorruptError(78385);
  }

  if (offset < pCur->info.nLocal) {
    int a = amt;
    if (a + offset > pCur->info.nLocal) {
      a = pCur->info.nLocal - offset;
    }
    rc = copyPayload(&aPayload[offset], pBuf, a, eOp, pPage->pDbPage);
    offset = 0;
    pBuf += a;
    amt -= a;
  } else {
    offset -= pCur->info.nLocal;
  }

  if (rc == SQLITE_OK && amt > 0) {
    const u32 ovflSize = pBt->usableSize - 4;
    Pgno nextPage;

    nextPage = sqlite3Get4byte(&aPayload[pCur->info.nLocal]);

    if ((pCur->curFlags & 0x04) == 0) {
      i64 nOvfl = pCur->info.nPayload;
      nOvfl = (nOvfl - pCur->info.nLocal + ovflSize - 1) / ovflSize;
      if (pCur->aOverflow == 0 || nOvfl * (int)sizeof(Pgno) > sqlite3MallocSize(pCur->aOverflow)) {
        Pgno *aNew;
        if (sqlite3FaultSim(413)) {
          aNew = 0;
        } else {
          aNew = (Pgno *)sqlite3Realloc(pCur->aOverflow, nOvfl * 2 * sizeof(Pgno));
        }
        if (aNew == 0) {
          return 7;
        } else {
          pCur->aOverflow = aNew;
        }
      }
      memset(pCur->aOverflow, 0, nOvfl * sizeof(Pgno));
      pCur->curFlags |= 0x04;
    } else {
      if (pCur->aOverflow[offset / ovflSize]) {
        iIdx = (offset / ovflSize);
        nextPage = pCur->aOverflow[iIdx];
        offset = (offset % ovflSize);
      }
    }

    while (nextPage) {
      if (nextPage > pBt->nPage)
        return sqlite3CorruptError(78458);

      pCur->aOverflow[iIdx] = nextPage;

      if (offset >= ovflSize) {
        if (pCur->aOverflow[iIdx + 1]) {
          nextPage = pCur->aOverflow[iIdx + 1];
        } else {
          rc = getOverflowPage(pBt, nextPage, 0, &nextPage);
        }
        offset -= ovflSize;
      } else {
        int a = amt;
        if (a + offset > ovflSize) {
          a = ovflSize - offset;
        }

        if (eOp == 0 && offset == 0 && sqlite3PagerDirectReadOk(pBt->pPager, nextPage) && &pBuf[-4] >= pBufStart) {
          sqlite3_file *fd = sqlite3PagerFile(pBt->pPager);
          u8 aSave[4];
          u8 *aWrite = &pBuf[-4];

          memcpy(aSave, aWrite, 4);
          rc = sqlite3OsRead(fd, aWrite, a + 4, (i64)pBt->pageSize * (nextPage - 1));
          nextPage = sqlite3Get4byte(aWrite);
          memcpy(aWrite, aSave, 4);
        } else {
          DbPage *pDbPage;
          rc = sqlite3PagerGet(pBt->pPager, nextPage, &pDbPage, (eOp == 0 ? 0x02 : 0));
          if (rc == SQLITE_OK) {
            if (eOp != 0 &&
                (sqlite3PagerPageRefcount(pDbPage) != 1 || (((MemPage *)sqlite3PagerGetExtra(pDbPage))->isInit))) {
              sqlite3PagerUnref(pDbPage);
              return sqlite3CorruptError(78528);
            }
            aPayload = (unsigned char*)(sqlite3PagerGetData(pDbPage));
            nextPage = sqlite3Get4byte(aPayload);
            rc = copyPayload(&aPayload[offset + 4], pBuf, a, eOp, pDbPage);
            sqlite3PagerUnref(pDbPage);
            offset = 0;
          }
        }
        amt -= a;
        if (amt == 0)
          return rc;
        pBuf += a;
      }
      if (rc)
        break;
      iIdx++;
    }
  }

  if (rc == SQLITE_OK && amt > 0) {
    return sqlite3CorruptError(78548);
  }
  return rc;
}

int sqlite3BtreePayload(BtCursor *pCur, u32 offset, u32 amt, void *pBuf) {
  return accessPayload(pCur, offset, amt, (unsigned char *)pBuf, 0);
}

__attribute__((noinline)) int accessPayloadChecked(BtCursor *pCur, u32 offset, u32 amt, void *pBuf) {
  int rc;
  if (pCur->eState == 1) {
    return SQLITE_ABORT;
  }

  rc = btreeRestoreCursorPosition(pCur);
  return rc ? rc : accessPayload(pCur, offset, amt, (unsigned char*)(pBuf), 0);
}

int sqlite3BtreePayloadChecked(BtCursor *pCur, u32 offset, u32 amt, void *pBuf) {
  if (pCur->eState == 0) {
    return accessPayload(pCur, offset, amt, (unsigned char*)(pBuf), 0);
  } else {
    return accessPayloadChecked(pCur, offset, amt, pBuf);
  }
}

const void *fetchPayload(BtCursor *pCur, u32 *pAmt) {
  int amt;

  amt = pCur->info.nLocal;
  if (amt > (int)(pCur->pPage->aDataEnd - pCur->info.pPayload)) {
    amt = ((0) > ((int)(pCur->pPage->aDataEnd - pCur->info.pPayload))
               ? (0)
               : ((int)(pCur->pPage->aDataEnd - pCur->info.pPayload)));
  }
  *pAmt = (u32)amt;
  return (void *)pCur->info.pPayload;
}

const void *sqlite3BtreePayloadFetch(BtCursor *pCur, u32 *pAmt) {
  return fetchPayload(pCur, pAmt);
}

int moveToChild(BtCursor *pCur, u32 newPgno) {
  int rc;

  if (pCur->iPage >= (20 - 1)) {
    return sqlite3CorruptError(78686);
  }
  pCur->info.nSize = 0;
  pCur->curFlags &= ~(0x02 | 0x04);
  pCur->aiIdx[(int)(pCur->iPage)] = pCur->ix;
  pCur->apPage[(int)(pCur->iPage)] = pCur->pPage;
  pCur->ix = 0;
  pCur->iPage++;
  rc = getAndInitPage(pCur->pBt, newPgno, &pCur->pPage, pCur->curPagerFlags);

  if (rc == SQLITE_OK && (pCur->pPage->nCell < 1 || pCur->pPage->intKey != pCur->curIntKey)) {
    releasePage(pCur->pPage);
    rc = sqlite3CorruptError(78700);
  }
  if (rc) {
    pCur->pPage = pCur->apPage[(int)(--pCur->iPage)];
  }
  return rc;
}

void moveToParent(BtCursor *pCur) {
  MemPage *pLeaf;

  pCur->info.nSize = 0;
  pCur->curFlags &= ~(0x02 | 0x04);
  pCur->ix = pCur->aiIdx[pCur->iPage - 1];
  pLeaf = pCur->pPage;
  pCur->pPage = pCur->apPage[(int)(--pCur->iPage)];
  releasePageNotNull(pLeaf);
}

int moveToRoot(BtCursor *pCur) {
  MemPage *pRoot;
  int rc = 0;

  if (pCur->iPage >= 0) {
    if (pCur->iPage) {
      releasePageNotNull(pCur->pPage);
      while (--pCur->iPage) {
        releasePageNotNull(pCur->apPage[(int)(pCur->iPage)]);
      }
      pRoot = pCur->pPage = pCur->apPage[0];
      goto skip_init;
    }
  } else if (pCur->pgnoRoot == 0) {
    pCur->eState = 1;
    return SQLITE_EMPTY;
  } else {
    if (pCur->eState >= 3) {
      if (pCur->eState == 4) {
        return pCur->skipNext;
      }
      sqlite3BtreeClearCursor(pCur);
    }
    rc = getAndInitPage(pCur->pBt, pCur->pgnoRoot, &pCur->pPage, pCur->curPagerFlags);
    if (rc != SQLITE_OK) {
      pCur->eState = 1;
      return rc;
    }
    pCur->iPage = 0;
    pCur->curIntKey = pCur->pPage->intKey;
  }
  pRoot = pCur->pPage;

  if (pRoot->isInit == 0 || (pCur->pKeyInfo == 0) != pRoot->intKey) {
    return sqlite3CorruptError(78835);
  }

skip_init:
  pCur->ix = 0;
  pCur->info.nSize = 0;
  pCur->curFlags &= ~(0x08 | 0x02 | 0x04);

  if (pRoot->nCell > 0) {
    pCur->eState = 0;
  } else if (!pRoot->leaf) {
    Pgno subpage;
    if (pRoot->pgno != 1)
      return sqlite3CorruptError(78847);
    subpage = sqlite3Get4byte(&pRoot->aData[pRoot->hdrOffset + 8]);
    pCur->eState = 0;
    rc = moveToChild(pCur, subpage);
  } else {
    pCur->eState = 1;
    rc = SQLITE_EMPTY;
  }
  return rc;
}

int moveToLeftmost(BtCursor *pCur) {
  Pgno pgno;
  int rc = SQLITE_OK;
  MemPage *pPage;

  while (rc == SQLITE_OK && !(pPage = pCur->pPage)->leaf) {
    pgno = sqlite3Get4byte(
        ((pPage)->aData + ((pPage)->maskPage & __builtin_bswap16(*(u16 *)(&(pPage)->aCellIdx[2 * (pCur->ix)])))));
    rc = moveToChild(pCur, pgno);
  }
  return rc;
}

int moveToRightmost(BtCursor *pCur) {
  Pgno pgno;
  int rc = SQLITE_OK;
  MemPage *pPage = 0;

  while (!(pPage = pCur->pPage)->leaf) {
    pgno = sqlite3Get4byte(&pPage->aData[pPage->hdrOffset + 8]);
    pCur->ix = pPage->nCell;
    rc = moveToChild(pCur, pgno);
    if (rc)
      return rc;
  }
  pCur->ix = pPage->nCell - 1;

  return 0;
}

int sqlite3BtreeFirst(BtCursor *pCur, int *pRes) {
  int rc;

  rc = moveToRoot(pCur);
  if (rc == SQLITE_OK) {
    *pRes = 0;
    rc = moveToLeftmost(pCur);
  } else if (rc == SQLITE_EMPTY) {
    *pRes = 1;
    rc = SQLITE_OK;
  }
  return rc;
}

int sqlite3BtreeIsEmpty(BtCursor *pCur, int *pRes) {
  int rc;

  if (pCur->eState == 0) {
    *pRes = 0;
    return SQLITE_OK;
  }
  rc = moveToRoot(pCur);
  if (rc == SQLITE_EMPTY) {
    *pRes = 1;
    rc = SQLITE_OK;
  } else {
    *pRes = 0;
  }
  return rc;
}

__attribute__((noinline)) int btreeLast(BtCursor *pCur, int *pRes) {
  int rc = moveToRoot(pCur);
  if (rc == SQLITE_OK) {
    *pRes = 0;
    rc = moveToRightmost(pCur);
    if (rc == SQLITE_OK) {
      pCur->curFlags |= 0x08;
    } else {
      pCur->curFlags &= ~0x08;
    }
  } else if (rc == SQLITE_EMPTY) {
    *pRes = 1;
    rc = SQLITE_OK;
  }
  return rc;
}

int sqlite3BtreeLast(BtCursor *pCur, int *pRes) {
  if (0 == pCur->eState && (pCur->curFlags & 0x08) != 0) {
    *pRes = 0;
    return SQLITE_OK;
  }
  return btreeLast(pCur, pRes);
}

int sqlite3BtreeTableMoveto(BtCursor *pCur, i64 intKey, int biasRight, int *pRes) {
  int rc;

  if (pCur->eState == 0 && (pCur->curFlags & 0x02) != 0) {
    if (pCur->info.nKey == intKey) {
      *pRes = 0;
      return SQLITE_OK;
    }
    if (pCur->info.nKey < intKey) {
      if ((pCur->curFlags & 0x08) != 0) {
        *pRes = -1;
        return SQLITE_OK;
      }

      if (pCur->info.nKey + 1 == intKey) {
        *pRes = 0;
        rc = sqlite3BtreeNext(pCur, 0);
        if (rc == SQLITE_OK) {
          getCellInfo(pCur);
          if (pCur->info.nKey == intKey) {
            return SQLITE_OK;
          }
        } else if (rc != SQLITE_DONE) {
          return rc;
        }
      }
    }
  }

  rc = moveToRoot(pCur);
  if (rc) {
    if (rc == SQLITE_EMPTY) {
      *pRes = -1;
      return SQLITE_OK;
    }
    return rc;
  }

  for (;;) {
    int lwr, upr, idx, c;
    Pgno chldPg;
    MemPage *pPage = pCur->pPage;
    u8 *pCell;

    lwr = 0;
    upr = pPage->nCell - 1;

    idx = upr >> (1 - biasRight);
    for (;;) {
      i64 nCellKey;
      pCell = ((pPage)->aDataOfst + ((pPage)->maskPage & __builtin_bswap16(*(u16 *)(&(pPage)->aCellIdx[2 * (idx)]))));
      if (pPage->intKeyLeaf) {
        while (0x80 <= *(pCell++)) {
          if (pCell >= pPage->aDataEnd) {
            return sqlite3CorruptError(79120);
          }
        }
      }
      sqlite3GetVarint(pCell, (u64 *)&nCellKey);
      if (nCellKey < intKey) {
        lwr = idx + 1;
        if (lwr > upr) {
          c = -1;
          break;
        }
      } else if (nCellKey > intKey) {
        upr = idx - 1;
        if (lwr > upr) {
          c = +1;
          break;
        }
      } else {
        pCur->ix = (u16)idx;
        if (!pPage->leaf) {
          lwr = idx;
          goto moveto_table_next_layer;
        } else {
          pCur->curFlags |= 0x02;
          pCur->info.nKey = nCellKey;
          pCur->info.nSize = 0;
          *pRes = 0;
          return SQLITE_OK;
        }
      }

      idx = (lwr + upr) >> 1;
    }

    if (pPage->leaf) {
      pCur->ix = (u16)idx;
      *pRes = c;
      rc = SQLITE_OK;
      goto moveto_table_finish;
    }
  moveto_table_next_layer:
    if (lwr >= pPage->nCell) {
      chldPg = sqlite3Get4byte(&pPage->aData[pPage->hdrOffset + 8]);
    } else {
      chldPg = sqlite3Get4byte(
          ((pPage)->aData + ((pPage)->maskPage & __builtin_bswap16(*(u16 *)(&(pPage)->aCellIdx[2 * (lwr)])))));
    }
    pCur->ix = (u16)lwr;
    rc = moveToChild(pCur, chldPg);
    if (rc)
      break;
  }
moveto_table_finish:
  pCur->info.nSize = 0;

  return rc;
}

int cursorOnLastPage(BtCursor *pCur) {
  int i;

  for (i = 0; i < pCur->iPage; i++) {
    MemPage *pPage = pCur->apPage[i];
    if (pCur->aiIdx[i] < pPage->nCell)
      return 0;
  }
  return 1;
}

int sqlite3BtreeIndexMoveto(BtCursor *pCur, UnpackedRecord *pIdxKey, int *pRes) {
  int rc;
  RecordCompare xRecordCompare;

  xRecordCompare = sqlite3VdbeFindCompare(pIdxKey);
  pIdxKey->errCode = 0;

  if (pCur->eState == 0 && pCur->pPage->leaf && cursorOnLastPage(pCur)) {
    int c;
    if (pCur->ix == pCur->pPage->nCell - 1 &&
        (c = indexCellCompare(pCur->pPage, pCur->ix, pIdxKey, xRecordCompare)) <= 0 && pIdxKey->errCode == SQLITE_OK) {
      *pRes = c;
      return SQLITE_OK;
    }
    if (pCur->iPage > 0 && indexCellCompare(pCur->pPage, 0, pIdxKey, xRecordCompare) <= 0 &&
        pIdxKey->errCode == SQLITE_OK) {
      pCur->curFlags &= ~(0x04 | 0x08);
      if (!pCur->pPage->isInit) {
        return sqlite3CorruptError(79315);
      }
      goto bypass_moveto_root;
    }
    pIdxKey->errCode = SQLITE_OK;
  }

  rc = moveToRoot(pCur);
  if (rc) {
    if (rc == SQLITE_EMPTY) {
      *pRes = -1;
      return SQLITE_OK;
    }
    return rc;
  }

bypass_moveto_root:
  for (;;) {
    int lwr, upr, idx, c;
    Pgno chldPg;
    MemPage *pPage = pCur->pPage;
    u8 *pCell;

    lwr = 0;
    upr = pPage->nCell - 1;
    idx = upr >> 1;
    for (;;) {
      int nCell;
      pCell = ((pPage)->aDataOfst + ((pPage)->maskPage & __builtin_bswap16(*(u16 *)(&(pPage)->aCellIdx[2 * (idx)]))));

      nCell = pCell[0];
      if (nCell <= pPage->max1bytePayload) {
        if (pCell + nCell >= pPage->aDataEnd) {
          rc = sqlite3CorruptError(79374);
          goto moveto_index_finish;
        }
        c = xRecordCompare(nCell, (void *)&pCell[1], pIdxKey);
      } else if (!(pCell[1] & 0x80) && (nCell = ((nCell & 0x7f) << 7) + pCell[1]) <= pPage->maxLocal &&
                 pCell + nCell < pPage->aDataEnd) {
        c = xRecordCompare(nCell, (void *)&pCell[2], pIdxKey);
      } else {
        void *pCellKey;
        u8 *const pCellBody = pCell - pPage->childPtrSize;
        const int nOverrun = 18;
        pPage->xParseCell(pPage, pCellBody, &pCur->info);
        nCell = (int)pCur->info.nKey;
        if (nCell < 2 || nCell / pCur->pBt->usableSize > pCur->pBt->nPage) {
          rc = sqlite3CorruptError(79405);
          goto moveto_index_finish;
        }
        pCellKey = sqlite3Malloc((u64)nCell + (u64)nOverrun);
        if (pCellKey == 0) {
          rc = 7;
          goto moveto_index_finish;
        }
        pCur->ix = (u16)idx;
        rc = accessPayload(pCur, 0, nCell, (unsigned char *)pCellKey, 0);
        memset(((u8 *)pCellKey) + nCell, 0, nOverrun);
        pCur->curFlags &= ~0x04;
        if (rc) {
          sqlite3_free(pCellKey);
          goto moveto_index_finish;
        }
        c = sqlite3VdbeRecordCompare(nCell, pCellKey, pIdxKey);
        sqlite3_free(pCellKey);
      }

      if (c < 0) {
        lwr = idx + 1;
      } else if (c > 0) {
        upr = idx - 1;
      } else {
        *pRes = 0;
        rc = SQLITE_OK;
        pCur->ix = (u16)idx;
        if (pIdxKey->errCode)
          rc = sqlite3CorruptError(79437);
        goto moveto_index_finish;
      }
      if (lwr > upr)
        break;

      idx = (lwr + upr) >> 1;
    }

    if (pPage->leaf) {
      pCur->ix = (u16)idx;
      *pRes = c;
      rc = SQLITE_OK;
      goto moveto_index_finish;
    }
    if (lwr >= pPage->nCell) {
      chldPg = sqlite3Get4byte(&pPage->aData[pPage->hdrOffset + 8]);
    } else {
      chldPg = sqlite3Get4byte(
          ((pPage)->aData + ((pPage)->maskPage & __builtin_bswap16(*(u16 *)(&(pPage)->aCellIdx[2 * (lwr)])))));
    }

    pCur->info.nSize = 0;
    pCur->curFlags &= ~(0x02 | 0x04);
    if (pCur->iPage >= (20 - 1)) {
      return sqlite3CorruptError(79468);
    }
    pCur->aiIdx[(int)(pCur->iPage)] = (u16)lwr;
    pCur->apPage[(int)(pCur->iPage)] = pCur->pPage;
    pCur->ix = 0;
    pCur->iPage++;
    rc = getAndInitPage(pCur->pBt, chldPg, &pCur->pPage, pCur->curPagerFlags);
    if (rc == SQLITE_OK && (pCur->pPage->nCell < 1 || pCur->pPage->intKey != pCur->curIntKey)) {
      releasePage(pCur->pPage);
      rc = sqlite3CorruptError(79479);
    }
    if (rc) {
      pCur->pPage = pCur->apPage[(int)(--pCur->iPage)];
      break;
    }
  }
moveto_index_finish:
  pCur->info.nSize = 0;

  return rc;
}

int sqlite3BtreeEof(BtCursor *pCur) {
  return (0 != pCur->eState);
}

i64 sqlite3BtreeRowCountEst(BtCursor *pCur) {
  i64 n;
  u8 i;

  if (pCur->eState != 0)
    return 0;
  if (pCur->pPage->leaf == 0)
    return -1;

  n = pCur->pPage->nCell;
  for (i = 0; i < pCur->iPage; i++) {
    n *= pCur->apPage[i]->nCell + 1;
  }
  return n;
}

__attribute__((noinline)) int btreeNext(BtCursor *pCur) {
  int rc;
  int idx;
  MemPage *pPage;

  if (pCur->eState != 0) {
    rc = (pCur->eState >= 3 ? btreeRestoreCursorPosition(pCur) : 0);
    if (rc != SQLITE_OK) {
      return rc;
    }
    if (1 == pCur->eState) {
      return SQLITE_DONE;
    }
    if (pCur->eState == 2) {
      pCur->eState = 0;
      if (pCur->skipNext > 0)
        return SQLITE_OK;
    }
  }

  pPage = pCur->pPage;
  idx = ++pCur->ix;
  if (sqlite3FaultSim(412))
    pPage->isInit = 0;
  if (!pPage->isInit) {
    return sqlite3CorruptError(79580);
  }

  if (idx >= pPage->nCell) {
    if (!pPage->leaf) {
      rc = moveToChild(pCur, sqlite3Get4byte(&pPage->aData[pPage->hdrOffset + 8]));
      if (rc)
        return rc;
      return moveToLeftmost(pCur);
    }
    do {
      if (pCur->iPage == 0) {
        pCur->eState = 1;
        return SQLITE_DONE;
      }
      moveToParent(pCur);
      pPage = pCur->pPage;
    } while (pCur->ix >= pPage->nCell);
    if (pPage->intKey) {
      return sqlite3BtreeNext(pCur, 0);
    } else {
      return SQLITE_OK;
    }
  }
  if (pPage->leaf) {
    return SQLITE_OK;
  } else {
    return moveToLeftmost(pCur);
  }
}

int sqlite3BtreeNext(BtCursor *pCur, int flags) {
  MemPage *pPage;
  (void)(flags);

  pCur->info.nSize = 0;
  pCur->curFlags &= ~(0x02 | 0x04);
  if (pCur->eState != 0)
    return btreeNext(pCur);
  pPage = pCur->pPage;
  if ((++pCur->ix) >= pPage->nCell) {
    pCur->ix--;
    return btreeNext(pCur);
  }
  if (pPage->leaf) {
    return SQLITE_OK;
  } else {
    return moveToLeftmost(pCur);
  }
}

__attribute__((noinline)) int btreePrevious(BtCursor *pCur) {
  int rc;
  MemPage *pPage;

  if (pCur->eState != 0) {
    rc = (pCur->eState >= 3 ? btreeRestoreCursorPosition(pCur) : 0);
    if (rc != SQLITE_OK) {
      return rc;
    }
    if (1 == pCur->eState) {
      return SQLITE_DONE;
    }
    if (2 == pCur->eState) {
      pCur->eState = 0;
      if (pCur->skipNext < 0)
        return SQLITE_OK;
    }
  }

  pPage = pCur->pPage;
  if (sqlite3FaultSim(412))
    pPage->isInit = 0;
  if (!pPage->isInit) {
    return sqlite3CorruptError(79673);
  }
  if (!pPage->leaf) {
    int idx = pCur->ix;
    rc = moveToChild(
        pCur, sqlite3Get4byte(
                  ((pPage)->aData + ((pPage)->maskPage & __builtin_bswap16(*(u16 *)(&(pPage)->aCellIdx[2 * (idx)]))))));
    if (rc)
      return rc;
    rc = moveToRightmost(pCur);
  } else {
    while (pCur->ix == 0) {
      if (pCur->iPage == 0) {
        pCur->eState = 1;
        return SQLITE_DONE;
      }
      moveToParent(pCur);
    }

    pCur->ix--;
    pPage = pCur->pPage;
    if (pPage->intKey && !pPage->leaf) {
      rc = sqlite3BtreePrevious(pCur, 0);
    } else {
      rc = SQLITE_OK;
    }
  }
  return rc;
}

int sqlite3BtreePrevious(BtCursor *pCur, int flags) {
  (void)(flags);
  pCur->curFlags &= ~(0x08 | 0x04 | 0x02);
  pCur->info.nSize = 0;
  if (pCur->eState != 0 || pCur->ix == 0 || pCur->pPage->leaf == 0) {
    return btreePrevious(pCur);
  }
  pCur->ix--;
  return SQLITE_OK;
}

int anotherValidCursor(BtCursor *pCur) {
  BtCursor *pOther;
  for (pOther = pCur->pBt->pCursor; pOther; pOther = pOther->pNext) {
    if (pOther != pCur && pOther->eState == 0 && pOther->pPage == pCur->pPage) {
      return sqlite3CorruptError(82339);
    }
  }
  return SQLITE_OK;
}

int balance(BtCursor *pCur) {
  int rc = SQLITE_OK;
  u8 aBalanceQuickSpace[13];
  u8 *pFree = 0;

  do {
    int iPage;
    MemPage *pPage = pCur->pPage;

    if ((pPage->nFree < 0) && btreeComputeFreeSpace(pPage))
      break;
    if (pPage->nOverflow == 0 && pPage->nFree * 3 <= (int)pCur->pBt->usableSize * 2) {
      break;
    } else if ((iPage = pCur->iPage) == 0) {
      if (pPage->nOverflow && (rc = anotherValidCursor(pCur)) == SQLITE_OK) {
        rc = balance_deeper(pPage, &pCur->apPage[1]);
        if (rc == SQLITE_OK) {
          pCur->iPage = 1;
          pCur->ix = 0;
          pCur->aiIdx[0] = 0;
          pCur->apPage[0] = pPage;
          pCur->pPage = pCur->apPage[1];
        }
      } else {
        break;
      }
    } else if (sqlite3PagerPageRefcount(pPage->pDbPage) > 1) {
      rc = sqlite3CorruptError(82399);
    } else {
      MemPage *const pParent = pCur->apPage[iPage - 1];
      int const iIdx = pCur->aiIdx[iPage - 1];

      rc = sqlite3PagerWrite(pParent->pDbPage);
      if (rc == SQLITE_OK && pParent->nFree < 0) {
        rc = btreeComputeFreeSpace(pParent);
      }
      if (rc == SQLITE_OK) {
        if (pPage->intKeyLeaf && pPage->nOverflow == 1 && pPage->aiOvfl[0] == pPage->nCell && pParent->pgno != 1 &&
            pParent->nCell == iIdx) {
          rc = balance_quick(pParent, pPage, aBalanceQuickSpace);
        } else {
          u8 *pSpace = (u8*)(sqlite3PageMalloc(pCur->pBt->pageSize));
          rc = balance_nonroot(pParent, iIdx, pSpace, iPage == 1, pCur->hints & 0x00000001);
          if (pFree) {
            sqlite3PageFree(pFree);
          }

          pFree = pSpace;
        }
      }

      pPage->nOverflow = 0;

      releasePage(pPage);
      pCur->iPage--;

      pCur->pPage = pCur->apPage[(int)(pCur->iPage)];
    }
  } while (rc == SQLITE_OK);

  if (pFree) {
    sqlite3PageFree(pFree);
  }
  return rc;
}

__attribute__((noinline)) int btreeOverwriteOverflowCell(BtCursor *pCur, const BtreePayload *pX) {
  int iOffset;
  int nTotal = pX->nData + pX->nZero;
  int rc;
  MemPage *pPage = pCur->pPage;
  BtShared *pBt;
  Pgno ovflPgno;
  u32 ovflPageSize;

  rc = btreeOverwriteContent(pPage, pCur->info.pPayload, pX, 0, pCur->info.nLocal);
  if (rc)
    return rc;

  iOffset = pCur->info.nLocal;

  ovflPgno = sqlite3Get4byte(pCur->info.pPayload + iOffset);
  pBt = pPage->pBt;
  ovflPageSize = pBt->usableSize - 4;
  do {
    rc = btreeGetPage(pBt, ovflPgno, &pPage, 0);
    if (rc)
      return rc;
    if (sqlite3PagerPageRefcount(pPage->pDbPage) != 1 || pPage->isInit) {
      rc = sqlite3CorruptError(82563);
    } else {
      if (iOffset + ovflPageSize < (u32)nTotal) {
        ovflPgno = sqlite3Get4byte(pPage->aData);
      } else {
        ovflPageSize = nTotal - iOffset;
      }
      rc = btreeOverwriteContent(pPage, pPage->aData + 4, pX, iOffset, ovflPageSize);
    }
    sqlite3PagerUnref(pPage->pDbPage);
    if (rc)
      return rc;
    iOffset += ovflPageSize;
  } while (iOffset < nTotal);
  return SQLITE_OK;
}

int btreeOverwriteCell(BtCursor *pCur, const BtreePayload *pX) {
  int nTotal = pX->nData + pX->nZero;
  MemPage *pPage = pCur->pPage;

  if (pCur->info.pPayload + pCur->info.nLocal > pPage->aDataEnd ||
      pCur->info.pPayload < pPage->aData + pPage->cellOffset) {
    return sqlite3CorruptError(82591);
  }
  if (pCur->info.nLocal == nTotal) {
    return btreeOverwriteContent(pPage, pCur->info.pPayload, pX, 0, pCur->info.nLocal);
  } else {
    return btreeOverwriteOverflowCell(pCur, pX);
  }
}

int sqlite3BtreeInsert(BtCursor *pCur, const BtreePayload *pX, int flags, int seekResult) {
  int rc;
  int loc = seekResult;
  int szNew = 0;
  int idx;
  MemPage *pPage;
  Btree *p = pCur->pBtree;
  unsigned char *oldCell;
  unsigned char *newCell = 0;

  if (pCur->curFlags & 0x20) {
    rc = saveAllCursors(p->pBt, pCur->pgnoRoot, pCur);
    if (rc)
      return rc;
    if (loc && pCur->iPage < 0) {
      return sqlite3CorruptError(82672);
    }
  }

  if (pCur->eState >= 3) {
    rc = moveToRoot(pCur);
    if (rc && rc != SQLITE_EMPTY)
      return rc;
  }

  if (pCur->pKeyInfo == 0) {
    if (p->hasIncrblobCur) {
      invalidateIncrblobCursors(p, pCur->pgnoRoot, pX->nKey, 0);
    }

    if ((pCur->curFlags & 0x02) != 0 && pX->nKey == pCur->info.nKey) {
      if (pCur->info.nSize != 0 && pCur->info.nPayload == (u32)pX->nData + pX->nZero) {
        return btreeOverwriteCell(pCur, pX);
      }

    } else if (loc == 0) {
      rc = sqlite3BtreeTableMoveto(pCur, pX->nKey, (flags & 0x08) != 0, &loc);
      if (rc)
        return rc;
    }
  } else {
    if (loc == 0 && (flags & 0x02) == 0) {
      if (pX->nMem) {
        UnpackedRecord r;
        r.pKeyInfo = pCur->pKeyInfo;
        r.aMem = pX->aMem;
        r.nField = pX->nMem;
        r.default_rc = 0;
        r.eqSeen = 0;
        rc = sqlite3BtreeIndexMoveto(pCur, &r, &loc);
      } else {
        rc = btreeMoveto(pCur, pX->pKey, pX->nKey, (flags & 0x08) != 0, &loc);
      }
      if (rc)
        return rc;
    }

    if (loc == 0) {
      getCellInfo(pCur);
      if (pCur->info.nKey == pX->nKey) {
        BtreePayload x2;
        x2.pData = pX->pKey;
        x2.nData = (int)pX->nKey;

        x2.nZero = 0;
        return btreeOverwriteCell(pCur, &x2);
      }
    }
  }

  pPage = pCur->pPage;

  if (pPage->nFree < 0) {
    if ((pCur->eState > 1)) {
      rc = sqlite3CorruptError(82795);
    } else {
      rc = btreeComputeFreeSpace(pPage);
    }
    if (rc)
      return rc;
  }

  newCell = p->pBt->pTmpSpace;

  if (flags & 0x80) {
    rc = SQLITE_OK;
    szNew = p->pBt->nPreformatSize;
    if (szNew < 4) {
      szNew = 4;
      newCell[3] = 0;
    }
    if ((p->pBt->autoVacuum) && szNew > pPage->maxLocal) {
      CellInfo info;
      pPage->xParseCell(pPage, newCell, &info);
      if (info.nPayload != info.nLocal) {
        Pgno ovfl = sqlite3Get4byte(&newCell[szNew - 4]);
        ptrmapPut(p->pBt, ovfl, 3, pPage->pgno, &rc);
        if ((rc))
          goto end_insert;
      }
    }
  } else {
    rc = fillInCell(pPage, newCell, pX, &szNew);
    if (rc)
      goto end_insert;
  }

  idx = pCur->ix;
  pCur->info.nSize = 0;
  if (loc == 0) {
    CellInfo info;

    if (idx >= pPage->nCell) {
      return sqlite3CorruptError(82837);
    }
    rc = sqlite3PagerWrite(pPage->pDbPage);
    if (rc) {
      goto end_insert;
    }
    oldCell = ((pPage)->aData + ((pPage)->maskPage & __builtin_bswap16(*(u16 *)(&(pPage)->aCellIdx[2 * (idx)]))));
    if (!pPage->leaf) {
      memcpy(newCell, oldCell, 4);
    }
    pPage->xParseCell(pPage, oldCell, &info);
    if (info.nLocal != info.nPayload) {
      rc = clearCellOverflow(pPage, oldCell, &info);
    } else {
      rc = 0;
    };
    (pCur->curFlags &= ~0x04);
    if (info.nSize == szNew && info.nLocal == info.nPayload && (!(p->pBt->autoVacuum) || szNew < pPage->minLocal)) {
      if (oldCell < pPage->aData + pPage->hdrOffset + 10) {
        return sqlite3CorruptError(82864);
      }
      if (oldCell + szNew > pPage->aDataEnd) {
        return sqlite3CorruptError(82867);
      }
      memcpy(oldCell, newCell, szNew);
      return SQLITE_OK;
    }
    dropCell(pPage, idx, info.nSize, &rc);
    if (rc)
      goto end_insert;
  } else if (loc < 0 && pPage->nCell > 0) {
    idx = ++pCur->ix;
    pCur->curFlags &= ~(0x02 | 0x04);
  } else {
  }
  rc = insertCellFast(pPage, idx, newCell, szNew);

  if (pPage->nOverflow) {
    pCur->curFlags &= ~(0x02 | 0x04);
    rc = balance(pCur);

    pCur->pPage->nOverflow = 0;
    pCur->eState = 1;
    if ((flags & 0x02) && rc == SQLITE_OK) {
      btreeReleaseAllCursorPages(pCur);
      if (pCur->pKeyInfo) {
        pCur->pKey = sqlite3Malloc(pX->nKey);
        if (pCur->pKey == 0) {
          rc = SQLITE_NOMEM;
        } else {
          memcpy(pCur->pKey, pX->pKey, pX->nKey);
        }
      }
      pCur->eState = 3;
      pCur->nKey = pX->nKey;
    }
  }

end_insert:
  return rc;
}

int sqlite3BtreeTransferRow(BtCursor *pDest, BtCursor *pSrc, i64 iKey) {
  BtShared *pBt = pDest->pBt;
  u8 *aOut = pBt->pTmpSpace;
  const u8 *aIn;
  u32 nIn;
  u32 nRem;

  getCellInfo(pSrc);
  if (pSrc->info.nPayload < 0x80) {
    *(aOut++) = (u8)pSrc->info.nPayload;
  } else {
    aOut += sqlite3PutVarint(aOut, pSrc->info.nPayload);
  }
  if (pDest->pKeyInfo == 0)
    aOut += sqlite3PutVarint(aOut, iKey);
  nIn = pSrc->info.nLocal;
  aIn = pSrc->info.pPayload;
  if (aIn + nIn > pSrc->pPage->aDataEnd) {
    return sqlite3CorruptError(82969);
  }
  nRem = pSrc->info.nPayload;
  if (nIn == nRem && nIn < pDest->pPage->maxLocal) {
    memcpy(aOut, aIn, nIn);
    pBt->nPreformatSize = nIn + (int)(aOut - pBt->pTmpSpace);
    return SQLITE_OK;
  } else {
    int rc = SQLITE_OK;
    Pager *pSrcPager = pSrc->pBt->pPager;
    u8 *pPgnoOut = 0;
    Pgno ovflIn = 0;
    DbPage *pPageIn = 0;
    MemPage *pPageOut = 0;
    u32 nOut;

    nOut = btreePayloadToLocal(pDest->pPage, pSrc->info.nPayload);
    pBt->nPreformatSize = (int)nOut + (int)(aOut - pBt->pTmpSpace);
    if (nOut < pSrc->info.nPayload) {
      pPgnoOut = &aOut[nOut];
      pBt->nPreformatSize += 4;
    }

    if (nRem > nIn) {
      if (aIn + nIn + 4 > pSrc->pPage->aDataEnd) {
        return sqlite3CorruptError(82994);
      }
      ovflIn = sqlite3Get4byte(&pSrc->info.pPayload[nIn]);
    }

    do {
      nRem -= nOut;
      do {
        if (nIn > 0) {
          int nCopy = ((nOut) < (nIn) ? (nOut) : (nIn));
          memcpy(aOut, aIn, nCopy);
          nOut -= nCopy;
          nIn -= nCopy;
          aOut += nCopy;
          aIn += nCopy;
        }
        if (nOut > 0) {
          sqlite3PagerUnref(pPageIn);
          pPageIn = 0;
          rc = sqlite3PagerGet(pSrcPager, ovflIn, &pPageIn, 0x02);
          if (rc == SQLITE_OK) {
            aIn = (const u8 *)sqlite3PagerGetData(pPageIn);
            ovflIn = sqlite3Get4byte(aIn);
            aIn += 4;
            nIn = pSrc->pBt->usableSize - 4;
          }
        }
      } while (rc == SQLITE_OK && nOut > 0);

      if (rc == SQLITE_OK && nRem > 0 && (pPgnoOut)) {
        Pgno pgnoNew = 0;
        MemPage *pNew = 0;
        rc = allocateBtreePage(pBt, &pNew, &pgnoNew, 0, 0);
        sqlite3Put4byte(pPgnoOut, pgnoNew);
        if ((pBt->autoVacuum) && pPageOut) {
          ptrmapPut(pBt, pgnoNew, 4, pPageOut->pgno, &rc);
        }
        releasePage(pPageOut);
        pPageOut = pNew;
        if (pPageOut) {
          pPgnoOut = pPageOut->aData;
          sqlite3Put4byte(pPgnoOut, 0);
          aOut = &pPgnoOut[4];
          nOut = ((pBt->usableSize - 4) < (nRem) ? (pBt->usableSize - 4) : (nRem));
        }
      }
    } while (nRem > 0 && rc == SQLITE_OK);

    releasePage(pPageOut);
    sqlite3PagerUnref(pPageIn);
    return rc;
  }
}

int sqlite3BtreeDelete(BtCursor *pCur, u8 flags) {
  Btree *p = pCur->pBtree;
  BtShared *pBt = p->pBt;
  int rc;
  MemPage *pPage;
  unsigned char *pCell;
  int iCellIdx;
  int iCellDepth;
  CellInfo info;
  u8 bPreserve;

  if (pCur->eState != 0) {
    if (pCur->eState >= 3) {
      rc = btreeRestoreCursorPosition(pCur);

      if (rc || pCur->eState != 0)
        return rc;
    } else {
      return sqlite3CorruptError(83090);
    }
  }

  iCellDepth = pCur->iPage;
  iCellIdx = pCur->ix;
  pPage = pCur->pPage;
  if (pPage->nCell <= iCellIdx) {
    return sqlite3CorruptError(83099);
  }
  pCell = ((pPage)->aData + ((pPage)->maskPage & __builtin_bswap16(*(u16 *)(&(pPage)->aCellIdx[2 * (iCellIdx)]))));
  if (pPage->nFree < 0 && btreeComputeFreeSpace(pPage)) {
    return sqlite3CorruptError(83103);
  }
  if (pCell < &pPage->aCellIdx[pPage->nCell]) {
    return sqlite3CorruptError(83106);
  }

  bPreserve = (flags & 0x02) != 0;
  if (bPreserve) {
    if (!pPage->leaf || (pPage->nFree + pPage->xCellSize(pPage, pCell) + 2) > (int)(pBt->usableSize * 2 / 3) ||
        pPage->nCell == 1) {
      rc = saveCursorKey(pCur);
      if (rc)
        return rc;
    } else {
      bPreserve = 2;
    }
  }

  if (!pPage->leaf) {
    rc = sqlite3BtreePrevious(pCur, 0);

    if (rc)
      return rc;
  }

  if (pCur->curFlags & 0x20) {
    rc = saveAllCursors(pBt, pCur->pgnoRoot, pCur);
    if (rc)
      return rc;
  }

  if (pCur->pKeyInfo == 0 && p->hasIncrblobCur) {
    invalidateIncrblobCursors(p, pCur->pgnoRoot, pCur->info.nKey, 0);
  }

  rc = sqlite3PagerWrite(pPage->pDbPage);
  if (rc)
    return rc;
  pPage->xParseCell(pPage, pCell, &info);
  if (info.nLocal != info.nPayload) {
    rc = clearCellOverflow(pPage, pCell, &info);
  } else {
    rc = 0;
  };
  dropCell(pPage, iCellIdx, info.nSize, &rc);
  if (rc)
    return rc;

  if (!pPage->leaf) {
    MemPage *pLeaf = pCur->pPage;
    int nCell;
    Pgno n;
    unsigned char *pTmp;

    if (pLeaf->nFree < 0) {
      rc = btreeComputeFreeSpace(pLeaf);
      if (rc)
        return rc;
    }
    if (iCellDepth < pCur->iPage - 1) {
      n = pCur->apPage[iCellDepth + 1]->pgno;
    } else {
      n = pCur->pPage->pgno;
    }
    pCell = ((pLeaf)->aData +
             ((pLeaf)->maskPage & __builtin_bswap16(*(u16 *)(&(pLeaf)->aCellIdx[2 * (pLeaf->nCell - 1)]))));
    if (pCell < &pLeaf->aData[4])
      return sqlite3CorruptError(83197);
    nCell = pLeaf->xCellSize(pLeaf, pCell);

    pTmp = pBt->pTmpSpace;

    rc = sqlite3PagerWrite(pLeaf->pDbPage);
    if (rc == SQLITE_OK) {
      rc = insertCell(pPage, iCellIdx, pCell - 4, nCell + 4, pTmp, n);
    }
    dropCell(pLeaf, pLeaf->nCell - 1, nCell, &rc);
    if (rc)
      return rc;
  }

  if (pCur->pPage->nFree * 3 <= (int)pCur->pBt->usableSize * 2) {
    rc = SQLITE_OK;
  } else {
    rc = balance(pCur);
  }
  if (rc == SQLITE_OK && pCur->iPage > iCellDepth) {
    releasePageNotNull(pCur->pPage);
    pCur->iPage--;
    while (pCur->iPage > iCellDepth) {
      releasePage(pCur->apPage[(int)(pCur->iPage--)]);
    }
    pCur->pPage = pCur->apPage[(int)(pCur->iPage)];
    rc = balance(pCur);
  }

  if (rc == SQLITE_OK) {
    if (bPreserve > 1) {
      pCur->eState = 2;
      if (iCellIdx >= pPage->nCell) {
        pCur->skipNext = -1;
        pCur->ix = pPage->nCell - 1;
      } else {
        pCur->skipNext = 1;
      }
    } else {
      rc = moveToRoot(pCur);
      if (bPreserve) {
        btreeReleaseAllCursorPages(pCur);
        pCur->eState = 3;
      }
      if (rc == SQLITE_EMPTY)
        rc = SQLITE_OK;
    }
  }
  return rc;
}

int sqlite3BtreeClearTableOfCursor(BtCursor *pCur) {
  return sqlite3BtreeClearTable(pCur->pBtree, pCur->pgnoRoot, 0);
}

int sqlite3BtreePutData(BtCursor *pCsr, u32 offset, u32 amt, void *z) {
  int rc;

  rc = (pCsr->eState >= 3 ? btreeRestoreCursorPosition(pCsr) : 0);
  if (rc != SQLITE_OK) {
    return rc;
  }

  if (pCsr->eState != 0) {
    return SQLITE_ABORT;
  }

  saveAllCursors(pCsr->pBt, pCsr->pgnoRoot, pCsr);

  if ((pCsr->curFlags & 0x01) == 0) {
    return SQLITE_READONLY;
  }

  return accessPayload(pCsr, offset, amt, (unsigned char *)z, 1);
}

void sqlite3BtreeIncrblobCursor(BtCursor *pCur) {
  pCur->curFlags |= 0x10;
  pCur->pBtree->hasIncrblobCur = 1;
}

int sqlite3BtreeCursorHasHint(BtCursor *pCsr, unsigned int mask) {
  return (pCsr->hints & mask) != 0;
}

int sqlite3VdbeMemFromBtree(BtCursor *pCur, u32 offset, u32 amt, Mem *pMem) {
  int rc;
  pMem->flags = 0x0001;
  if (amt >= 2147483391) {
    return 7;
  }
  if ((u64)amt + (u64)offset > (u64)sqlite3BtreeMaxRecordSize(pCur)) {
    return sqlite3CorruptError(87090);
  }
  if (SQLITE_OK == (rc = sqlite3VdbeMemClearAndResize(pMem, amt + 1))) {
    rc = sqlite3BtreePayload(pCur, offset, amt, pMem->z);
    if (rc == SQLITE_OK) {
      pMem->z[amt] = 0;
      pMem->flags = 0x0010;
      pMem->n = (int)amt;
    } else {
      sqlite3VdbeMemRelease(pMem);
    }
  }
  return rc;
}

int sqlite3VdbeMemFromBtreeZeroOffset(BtCursor *pCur, u32 amt, Mem *pMem) {
  u32 available = 0;
  int rc = 0;

  pMem->z = (char *)sqlite3BtreePayloadFetch(pCur, &available);

  if (amt <= available) {
    pMem->flags = 0x0010 | 0x4000;
    pMem->n = (int)amt;
  } else {
    rc = sqlite3VdbeMemFromBtree(pCur, 0, amt, pMem);
  }

  return rc;
}

int vdbeIsMatchingIndexKey(BtCursor *pCur, int bInt, Bitmask mask, UnpackedRecord *p, int *piRes) {
  u8 *aRec = 0;
  u32 nRec = 0;
  Mem mem;
  int rc = SQLITE_OK;

  memset(&mem, 0, sizeof(mem));
  mem.enc = p->pKeyInfo->enc;
  mem.db = p->pKeyInfo->db;
  nRec = sqlite3BtreePayloadSize(pCur);
  if (nRec > 0x7fffffff) {
    return sqlite3CorruptError(93335);
  }

  aRec = (u8*)(sqlite3MallocZero(nRec + 5));
  if (aRec == 0) {
    rc = 7;
  } else {
    rc = sqlite3BtreePayload(pCur, 0, nRec, aRec);
  }

  if (rc == SQLITE_OK) {
    u32 szHdr = 0;
    u32 idxHdr = 0;

    idxHdr = (u8)((*(aRec) < (u8)0x80) ? ((szHdr) = (u32) * (aRec)), 1 : sqlite3GetVarint32((aRec), (u32 *)&(szHdr)));
    if (szHdr > 98307) {
      rc = SQLITE_CORRUPT;
    } else {
      int res = 0;
      u32 idxRec = szHdr;
      int ii = 0;

      int nCol = p->pKeyInfo->nAllField;
      for (ii = 0; ii < nCol && rc == SQLITE_OK; ii++) {
        u32 iSerial = 0;
        int nSerial = 0;

        if (idxHdr >= szHdr) {
          rc = sqlite3CorruptError(93366);
          break;
        }
        idxHdr += (u8)((*(&aRec[idxHdr]) < (u8)0x80) ? ((iSerial) = (u32) * (&aRec[idxHdr])),
                       1                             : sqlite3GetVarint32((&aRec[idxHdr]), (u32 *)&(iSerial)));
        nSerial = sqlite3VdbeSerialTypeLen(iSerial);
        if ((idxRec + nSerial) > nRec) {
          rc = sqlite3CorruptError(93372);
        } else {
          sqlite3VdbeSerialGet(&aRec[idxRec], iSerial, &mem);
          if (vdbeSkipField(mask, ii, &p->aMem[ii], &mem, bInt) == 0) {
            res = sqlite3MemCompare(&mem, &p->aMem[ii], p->pKeyInfo->aColl[ii]);
            if (res != 0)
              break;
          }
        }
        idxRec += sqlite3VdbeSerialTypeLen(iSerial);
      }

      *piRes = res;
    }
  }

  sqlite3_free(aRec);
  return rc;
}

int sqlite3VdbeFindIndexKey(BtCursor *pCur, Index *pIdx, UnpackedRecord *p, int *pRes, int bIntegrity) {
  int nStep = 0;
  int res = 1;
  int rc = SQLITE_OK;
  int ii = 0;

  Bitmask mask = 0;
  for (ii = 0;
       ii < ((pIdx->nColumn) < (((int)(sizeof(Bitmask) * 8))) ? (pIdx->nColumn) : (((int)(sizeof(Bitmask) * 8))));
       ii++) {
    int iCol = pIdx->aiColumn[ii];
    if ((iCol == (-2)) || (iCol >= 0 && (pIdx->pTable->aCol[iCol].colFlags & 0x0020))) {
      mask |= (((Bitmask)1) << (ii));
    }
  }

  if (mask != 0) {
    for (ii = 0; sqlite3BtreeEof(pCur) == 0 && ii < 10; ii++) {
      rc = sqlite3BtreePrevious(pCur, 0);
    }
    if (rc == SQLITE_DONE) {
      rc = sqlite3BtreeFirst(pCur, &res);
      nStep = -1;
    } else {
      nStep = 10 * 2;
    }

    while (sqlite3BtreeCursorIsValidNN(pCur)) {
      for (ii = 0; rc == SQLITE_OK && (ii < nStep || nStep < 0); ii++) {
        rc = vdbeIsMatchingIndexKey(pCur, bIntegrity, mask, p, &res);
        if (res == 0 || rc != SQLITE_OK)
          break;
        rc = sqlite3BtreeNext(pCur, 0);
      }
      if (rc == SQLITE_DONE) {
        rc = 0;
      }
      if (nStep < 0 || rc != SQLITE_OK || res == 0 || bIntegrity)
        break;

      nStep = -1;
      rc = sqlite3BtreeFirst(pCur, &res);
    }
  }

  *pRes = res;
  return rc;
}