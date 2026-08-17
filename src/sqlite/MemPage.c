#define _GNU_SOURCE 1

#include <stdint.h>
#include <string.h>

#include "sqlite/MemPage.h"

#include "sqlite/BtShared.h"
#include "sqlite/BtreePayload.h"
#include "sqlite/CellArray.h"
#include "sqlite/CellInfo.h"
#include "sqlite/DbPage.h"
#include "sqlite/Pager.h"
#include "sqlite/PgHdr.h"
#include "sqlite/Pgno.h"
#include "sqlite/RecordCompare.h"
#include "sqlite/UnpackedRecord.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/uptr.h"
__attribute__((noinline)) void btreeParseCellAdjustSizeForOverflow(MemPage *pPage, u8 *pCell, CellInfo *pInfo) {

  int minLocal;
  int maxLocal;
  int surplus;

  minLocal = pPage->minLocal;
  maxLocal = pPage->maxLocal;
  surplus = minLocal + (pInfo->nPayload - minLocal) % (pPage->pBt->usableSize - 4);
  ;
  ;
  if (surplus <= maxLocal) {
    pInfo->nLocal = (u16)surplus;
  } else {
    pInfo->nLocal = (u16)minLocal;
  }
  pInfo->nSize = (u16)(&pInfo->pPayload[pInfo->nLocal] - pCell) + 4;
}

int btreePayloadToLocal(MemPage *pPage, i64 nPayload) {
  int maxLocal;
  maxLocal = pPage->maxLocal;

  if (nPayload <= maxLocal) {
    return (int)nPayload;
  } else {
    int minLocal;
    int surplus;
    minLocal = pPage->minLocal;
    surplus = (int)(minLocal + (nPayload - minLocal) % (pPage->pBt->usableSize - 4));
    return (surplus <= maxLocal) ? surplus : minLocal;
  }
}

void btreeParseCellPtrNoPayload(MemPage *pPage, u8 *pCell, CellInfo *pInfo) {

  (void)(pPage);

  pInfo->nSize = 4 + sqlite3GetVarint(&pCell[4], (u64 *)&pInfo->nKey);
  pInfo->nPayload = 0;
  pInfo->nLocal = 0;
  pInfo->pPayload = 0;
  return;
}

void btreeParseCellPtr(MemPage *pPage, u8 *pCell, CellInfo *pInfo) {
  u8 *pIter;
  u64 nPayload;
  u64 iKey;

  pIter = pCell;

  nPayload = *pIter;
  if (nPayload >= 0x80) {
    u8 *pEnd = &pIter[8];
    nPayload &= 0x7f;
    do {
      nPayload = (nPayload << 7) | (*++pIter & 0x7f);
    } while ((*pIter) >= 0x80 && pIter < pEnd);
    nPayload &= 0xffffffff;
  }
  pIter++;

  iKey = *pIter;
  if (iKey >= 0x80) {
    u8 x;
    iKey = (iKey << 7) ^ (x = *++pIter);
    if (x >= 0x80) {
      iKey = (iKey << 7) ^ (x = *++pIter);
      if (x >= 0x80) {
        iKey = (iKey << 7) ^ 0x10204000 ^ (x = *++pIter);
        if (x >= 0x80) {
          iKey = (iKey << 7) ^ 0x4000 ^ (x = *++pIter);
          if (x >= 0x80) {
            iKey = (iKey << 7) ^ 0x4000 ^ (x = *++pIter);
            if (x >= 0x80) {
              iKey = (iKey << 7) ^ 0x4000 ^ (x = *++pIter);
              if (x >= 0x80) {
                iKey = (iKey << 7) ^ 0x4000 ^ (x = *++pIter);
                if (x >= 0x80) {
                  iKey = (iKey << 8) ^ 0x8000 ^ (*++pIter);
                }
              }
            }
          }
        }
      } else {
        iKey ^= 0x204000;
      }
    } else {
      iKey ^= 0x4000;
    }
  }
  pIter++;

  pInfo->nKey = *(i64 *)&iKey;
  pInfo->nPayload = (u32)nPayload;
  pInfo->pPayload = pIter;
  ;
  ;

  if (nPayload <= pPage->maxLocal) {

    pInfo->nSize = (u16)nPayload + (u16)(pIter - pCell);
    if (pInfo->nSize < 4)
      pInfo->nSize = 4;
    pInfo->nLocal = (u16)nPayload;
  } else {
    btreeParseCellAdjustSizeForOverflow(pPage, pCell, pInfo);
  }
}

void btreeParseCellPtrIndex(MemPage *pPage, u8 *pCell, CellInfo *pInfo) {
  u8 *pIter;
  u32 nPayload;

  pIter = pCell + pPage->childPtrSize;
  nPayload = *pIter;
  if (nPayload >= 0x80) {
    u8 *pEnd = &pIter[8];
    nPayload &= 0x7f;
    do {
      nPayload = (nPayload << 7) | (*++pIter & 0x7f);
    } while (*(pIter) >= 0x80 && pIter < pEnd);
  }
  pIter++;
  pInfo->nKey = nPayload;
  pInfo->nPayload = nPayload;
  pInfo->pPayload = pIter;
  ;
  ;

  if (nPayload <= pPage->maxLocal) {

    pInfo->nSize = (u16)nPayload + (u16)(pIter - pCell);
    if (pInfo->nSize < 4)
      pInfo->nSize = 4;
    pInfo->nLocal = (u16)nPayload;
  } else {
    btreeParseCellAdjustSizeForOverflow(pPage, pCell, pInfo);
  }
}

void btreeParseCell(MemPage *pPage, int iCell, CellInfo *pInfo) { pPage->xParseCell(pPage, ((pPage)->aData + ((pPage)->maskPage & __builtin_bswap16(*(u16 *)(&(pPage)->aCellIdx[2 * (iCell)])))), pInfo); }

u16 cellSizePtr(MemPage *pPage, u8 *pCell) {
  u8 *pIter = pCell + 4;
  u8 *pEnd;
  u32 nSize;

  nSize = *pIter;
  if (nSize >= 0x80) {
    pEnd = &pIter[8];
    nSize &= 0x7f;
    do {
      nSize = (nSize << 7) | (*++pIter & 0x7f);
    } while (*(pIter) >= 0x80 && pIter < pEnd);
  }
  pIter++;
  ;
  ;
  if (nSize <= pPage->maxLocal) {
    nSize += (u32)(pIter - pCell);

    ((void)(0))

        ;
  } else {
    int minLocal = pPage->minLocal;
    nSize = minLocal + (nSize - minLocal) % (pPage->pBt->usableSize - 4);
    ;
    ;
    if (nSize > pPage->maxLocal) {
      nSize = minLocal;
    }
    nSize += 4 + (u16)(pIter - pCell);
  }

  return (u16)nSize;
}

u16 cellSizePtrIdxLeaf(MemPage *pPage, u8 *pCell) {
  u8 *pIter = pCell;
  u8 *pEnd;
  u32 nSize;

  nSize = *pIter;
  if (nSize >= 0x80) {
    pEnd = &pIter[8];
    nSize &= 0x7f;
    do {
      nSize = (nSize << 7) | (*++pIter & 0x7f);
    } while (*(pIter) >= 0x80 && pIter < pEnd);
  }
  pIter++;
  ;
  ;
  if (nSize <= pPage->maxLocal) {
    nSize += (u32)(pIter - pCell);
    if (nSize < 4)
      nSize = 4;
  } else {
    int minLocal = pPage->minLocal;
    nSize = minLocal + (nSize - minLocal) % (pPage->pBt->usableSize - 4);
    ;
    ;
    if (nSize > pPage->maxLocal) {
      nSize = minLocal;
    }
    nSize += 4 + (u16)(pIter - pCell);
  }

  return (u16)nSize;
}

u16 cellSizePtrNoPayload(MemPage *pPage, u8 *pCell) {
  u8 *pIter = pCell + 4;
  u8 *pEnd;

  (void)(pPage);

  pEnd = pIter + 9;
  while ((*pIter++) & 0x80 && pIter < pEnd)
    ;

  return (u16)(pIter - pCell);
}

u16 cellSizePtrTableLeaf(MemPage *pPage, u8 *pCell) {
  u8 *pIter = pCell;
  u8 *pEnd;
  u32 nSize;

  nSize = *pIter;
  if (nSize >= 0x80) {
    pEnd = &pIter[8];
    nSize &= 0x7f;
    do {
      nSize = (nSize << 7) | (*++pIter & 0x7f);
    } while (*(pIter) >= 0x80 && pIter < pEnd);
  }
  pIter++;

  if ((*pIter++) & 0x80 && (*pIter++) & 0x80 && (*pIter++) & 0x80 && (*pIter++) & 0x80 && (*pIter++) & 0x80 && (*pIter++) & 0x80 && (*pIter++) & 0x80 && (*pIter++) & 0x80) {
    pIter++;
  };
  ;
  if (nSize <= pPage->maxLocal) {
    nSize += (u32)(pIter - pCell);
    if (nSize < 4)
      nSize = 4;
  } else {
    int minLocal = pPage->minLocal;
    nSize = minLocal + (nSize - minLocal) % (pPage->pBt->usableSize - 4);
    ;
    ;
    if (nSize > pPage->maxLocal) {
      nSize = minLocal;
    }
    nSize += 4 + (u16)(pIter - pCell);
  }

  return (u16)nSize;
}

void ptrmapPutOvflPtr(MemPage *pPage, MemPage *pSrc, u8 *pCell, int *pRC) {
  CellInfo info;
  if (*pRC)
    return;

  pPage->xParseCell(pPage, pCell, &info);
  if (info.nLocal < info.nPayload) {
    Pgno ovfl;
    if ((((uptr)(pCell) < (uptr)(pSrc->aDataEnd)) && ((uptr)(pCell + info.nLocal) > (uptr)(pSrc->aDataEnd)))) {
      ;
      *pRC = sqlite3CorruptError(74816);
      return;
    }
    ovfl = sqlite3Get4byte(&pCell[info.nSize - 4]);
    ptrmapPut(pPage->pBt, ovfl, 3, pPage->pgno, pRC);
  }
}

int defragmentPage(MemPage *pPage, int nMaxFrag) {
  int i;
  int pc;
  int hdr;
  int size;
  int usableSize;
  int cellOffset;
  int cbrk;
  int nCell;
  unsigned char *data;
  unsigned char *temp;
  unsigned char *src;
  int iCellFirst;
  int iCellLast;
  int iCellStart;

  data = pPage->aData;
  hdr = pPage->hdrOffset;
  cellOffset = pPage->cellOffset;
  nCell = pPage->nCell;

  iCellFirst = cellOffset + 2 * nCell;
  usableSize = pPage->pBt->usableSize;

  if ((int)data[hdr + 7] <= nMaxFrag) {
    int iFree = ((&data[hdr + 1])[0] << 8 | (&data[hdr + 1])[1]);
    if (iFree > usableSize - 4)
      return sqlite3CorruptError(74874);
    if (iFree) {
      int iFree2 = ((&data[iFree])[0] << 8 | (&data[iFree])[1]);
      if (iFree2 > usableSize - 4)
        return sqlite3CorruptError(74877);
      if (0 == iFree2 || (data[iFree2] == 0 && data[iFree2 + 1] == 0)) {
        u8 *pEnd = &data[cellOffset + nCell * 2];
        u8 *pAddr;
        int sz2 = 0;
        int sz = ((&data[iFree + 2])[0] << 8 | (&data[iFree + 2])[1]);
        int top = ((&data[hdr + 5])[0] << 8 | (&data[hdr + 5])[1]);
        if (top >= iFree) {
          return sqlite3CorruptError(74885);
        }
        if (iFree2) {
          if (iFree + sz > iFree2)
            return sqlite3CorruptError(74888);
          sz2 = ((&data[iFree2 + 2])[0] << 8 | (&data[iFree2 + 2])[1]);
          if (iFree2 + sz2 > usableSize)
            return sqlite3CorruptError(74890);
          memmove(&data[iFree + sz + sz2], &data[iFree + sz], iFree2 - (iFree + sz));
          sz += sz2;
        } else if (iFree + sz > usableSize) {
          return sqlite3CorruptError(74894);
        }

        cbrk = top + sz;

        ((void)(0))

            ;
        memmove(&data[cbrk], &data[top], iFree - top);
        for (pAddr = &data[cellOffset]; pAddr < pEnd; pAddr += 2) {
          pc = ((pAddr)[0] << 8 | (pAddr)[1]);
          if (pc < iFree) {
            ((pAddr)[0] = (u8)((pc + sz) >> 8), (pAddr)[1] = (u8)(pc + sz));
          } else if (pc < iFree2) {
            ((pAddr)[0] = (u8)((pc + sz2) >> 8), (pAddr)[1] = (u8)(pc + sz2));
          }
        }
        goto defragment_out;
      }
    }
  }

  cbrk = usableSize;
  iCellLast = usableSize - 4;
  iCellStart = ((&data[hdr + 5])[0] << 8 | (&data[hdr + 5])[1]);
  if (nCell > 0) {
    temp = sqlite3PagerTempSpace(pPage->pBt->pPager);
    memcpy(temp, data, usableSize);
    src = temp;
    for (i = 0; i < nCell; i++) {
      u8 *pAddr;
      pAddr = &data[cellOffset + i * 2];
      pc = ((pAddr)[0] << 8 | (pAddr)[1]);
      ;
      ;

      if (pc > iCellLast) {
        return sqlite3CorruptError(74927);
      }

      ((void)(0))

          ;
      size = pPage->xCellSize(pPage, &src[pc]);
      cbrk -= size;
      if (cbrk < iCellStart || pc + size > usableSize) {
        return sqlite3CorruptError(74933);
      }

      ((void)(0))

          ;
      ;
      ;
      ((pAddr)[0] = (u8)((cbrk) >> 8), (pAddr)[1] = (u8)(cbrk));
      memcpy(&data[cbrk], &src[pc], size);
    }
  }
  data[hdr + 7] = 0;

defragment_out:

  if (data[hdr + 7] + cbrk - iCellFirst != pPage->nFree) {
    return sqlite3CorruptError(74947);
  }

  ((&data[hdr + 5])[0] = (u8)((cbrk) >> 8), (&data[hdr + 5])[1] = (u8)(cbrk));
  data[hdr + 1] = 0;
  data[hdr + 2] = 0;
  memset(&data[iCellFirst], 0, cbrk - iCellFirst);

  return 0;
}

u8 *pageFindSlot(MemPage *pPg, int nByte, int *pRc) {
  const int hdr = pPg->hdrOffset;
  u8 *const aData = pPg->aData;
  int iAddr = hdr + 1;
  u8 *pTmp = &aData[iAddr];
  int pc = ((pTmp)[0] << 8 | (pTmp)[1]);
  int x;
  int maxPC = pPg->pBt->usableSize - nByte;
  int size;

  while (pc <= maxPC) {

    pTmp = &aData[pc + 2];
    size = ((pTmp)[0] << 8 | (pTmp)[1]);
    if ((x = size - nByte) >= 0) {
      ;
      ;
      if (x < 4) {

        if (aData[hdr + 7] > 57)
          return 0;

        memcpy(&aData[iAddr], &aData[pc], 2);
        aData[hdr + 7] += (u8)x;
        return &aData[pc];
      } else if (x + pc > maxPC) {

        *pRc = sqlite3CorruptError(75004);
        return 0;
      } else {

        ((&aData[pc + 2])[0] = (u8)((x) >> 8), (&aData[pc + 2])[1] = (u8)(x));
      }
      return &aData[pc + x];
    }
    iAddr = pc;
    pTmp = &aData[pc];
    pc = ((pTmp)[0] << 8 | (pTmp)[1]);
    if (pc <= iAddr) {
      if (pc) {

        *pRc = sqlite3CorruptError(75019);
      }
      return 0;
    }
  }
  if (pc > maxPC + nByte - 4) {

    *pRc = sqlite3CorruptError(75026);
  }
  return 0;
}

__attribute__((always_inline)) inline int allocateSpace(MemPage *pPage, int nByte, int *pIdx) {
  const int hdr = pPage->hdrOffset;
  u8 *const data = pPage->aData;
  int top;
  int rc = 0;
  u8 *pTmp;
  int gap;

  gap = pPage->cellOffset + 2 * pPage->nCell;

  pTmp = &data[hdr + 5];
  top = ((pTmp)[0] << 8 | (pTmp)[1]);
  if (gap > top) {
    if (top == 0 && pPage->pBt->usableSize == 65536) {
      top = 65536;
    } else {
      return sqlite3CorruptError(75074);
    }
  } else if (top > (int)pPage->pBt->usableSize) {
    return sqlite3CorruptError(75077);
  }

  ;
  ;
  ;
  if ((data[hdr + 2] || data[hdr + 1]) && gap + 2 <= top) {
    u8 *pSpace = pageFindSlot(pPage, nByte, &rc);
    if (pSpace) {
      int g2;

      ((void)(0))

          ;
      *pIdx = g2 = (int)(pSpace - data);
      if (g2 <= gap) {
        return sqlite3CorruptError(75094);
      } else {
        return 0;
      }
    } else if (rc) {
      return rc;
    }
  }

  ;
  if (gap + 2 + nByte > top) {

    ((void)(0))

        ;

    ((void)(0))

        ;
    rc = defragmentPage(pPage, ((4) < (pPage->nFree - (2 + nByte)) ? (4) : (pPage->nFree - (2 + nByte))));
    if (rc)
      return rc;
    top = (((((int)((&data[hdr + 5])[0] << 8 | (&data[hdr + 5])[1])) - 1) & 0xffff) + 1);

    ((void)(0))

        ;
  }

  top -= nByte;
  ((&data[hdr + 5])[0] = (u8)((top) >> 8), (&data[hdr + 5])[1] = (u8)(top));

  *pIdx = top;
  return 0;
}

int freeSpace(MemPage *pPage, int iStart, int iSize) {
  int iPtr;
  int iFreeBlk;
  u8 hdr;
  int nFrag = 0;
  int iOrigSize = iSize;
  int x;
  int iEnd = iStart + iSize;
  unsigned char *data = pPage->aData;
  u8 *pTmp;

  hdr = pPage->hdrOffset;
  iPtr = hdr + 1;
  if (data[iPtr + 1] == 0 && data[iPtr] == 0) {
    iFreeBlk = 0;
  } else {
    while ((iFreeBlk = ((&data[iPtr])[0] << 8 | (&data[iPtr])[1])) < iStart) {
      if (iFreeBlk <= iPtr) {
        if (iFreeBlk == 0)
          break;
        return sqlite3CorruptError(75173);
      }
      iPtr = iFreeBlk;
    }
    if (iFreeBlk > (int)pPage->pBt->usableSize - 4) {
      return sqlite3CorruptError(75178);
    }

    ((void)(0))

        ;

    if (iFreeBlk && iEnd + 3 >= iFreeBlk) {
      nFrag = iFreeBlk - iEnd;
      if (iEnd > iFreeBlk)
        return sqlite3CorruptError(75190);
      iEnd = iFreeBlk + ((&data[iFreeBlk + 2])[0] << 8 | (&data[iFreeBlk + 2])[1]);
      if (iEnd > (int)pPage->pBt->usableSize) {
        return sqlite3CorruptError(75193);
      }
      iSize = iEnd - iStart;
      iFreeBlk = ((&data[iFreeBlk])[0] << 8 | (&data[iFreeBlk])[1]);
    }

    if (iPtr > hdr + 1) {
      int iPtrEnd = iPtr + ((&data[iPtr + 2])[0] << 8 | (&data[iPtr + 2])[1]);
      if (iPtrEnd + 3 >= iStart) {
        if (iPtrEnd > iStart)
          return sqlite3CorruptError(75206);
        nFrag += iStart - iPtrEnd;
        iSize = iEnd - iPtr;
        iStart = iPtr;
      }
    }
    if (nFrag > data[hdr + 7])
      return sqlite3CorruptError(75212);
    data[hdr + 7] -= (u8)nFrag;
  }
  pTmp = &data[hdr + 5];
  x = ((pTmp)[0] << 8 | (pTmp)[1]);
  if (pPage->pBt->btsFlags & 0x000c) {

    memset(&data[iStart], 0, iSize);
  }
  if (iStart <= x) {

    if (iStart < x)
      return sqlite3CorruptError(75226);
    if (iPtr != hdr + 1)
      return sqlite3CorruptError(75227);
    ((&data[hdr + 1])[0] = (u8)((iFreeBlk) >> 8), (&data[hdr + 1])[1] = (u8)(iFreeBlk));
    ((&data[hdr + 5])[0] = (u8)((iEnd) >> 8), (&data[hdr + 5])[1] = (u8)(iEnd));
  } else {

    ((&data[iPtr])[0] = (u8)((iStart) >> 8), (&data[iPtr])[1] = (u8)(iStart));
    ((&data[iStart])[0] = (u8)((iFreeBlk) >> 8), (&data[iStart])[1] = (u8)(iFreeBlk));

    ((void)(0))

        ;
    ((&data[iStart + 2])[0] = (u8)(((u16)iSize) >> 8), (&data[iStart + 2])[1] = (u8)((u16)iSize));
  }
  pPage->nFree += iOrigSize;
  return 0;
}

int decodeFlags(MemPage *pPage, int flagByte) {
  BtShared *pBt;

  pBt = pPage->pBt;
  pPage->max1bytePayload = pBt->max1bytePayload;
  if (flagByte >= (0x02 | 0x08)) {
    pPage->childPtrSize = 0;
    pPage->leaf = 1;
    if (flagByte == (0x04 | 0x01 | 0x08)) {
      pPage->intKeyLeaf = 1;
      pPage->xCellSize = cellSizePtrTableLeaf;
      pPage->xParseCell = btreeParseCellPtr;
      pPage->intKey = 1;
      pPage->maxLocal = pBt->maxLeaf;
      pPage->minLocal = pBt->minLeaf;
    } else if (flagByte == (0x02 | 0x08)) {
      pPage->intKey = 0;
      pPage->intKeyLeaf = 0;
      pPage->xCellSize = cellSizePtrIdxLeaf;
      pPage->xParseCell = btreeParseCellPtrIndex;
      pPage->maxLocal = pBt->maxLocal;
      pPage->minLocal = pBt->minLocal;
    } else {
      pPage->intKey = 0;
      pPage->intKeyLeaf = 0;
      pPage->xCellSize = cellSizePtrIdxLeaf;
      pPage->xParseCell = btreeParseCellPtrIndex;
      return sqlite3CorruptError(75282);
    }
  } else {
    pPage->childPtrSize = 4;
    pPage->leaf = 0;
    if (flagByte == (0x02)) {
      pPage->intKey = 0;
      pPage->intKeyLeaf = 0;
      pPage->xCellSize = cellSizePtr;
      pPage->xParseCell = btreeParseCellPtrIndex;
      pPage->maxLocal = pBt->maxLocal;
      pPage->minLocal = pBt->minLocal;
    } else if (flagByte == (0x04 | 0x01)) {
      pPage->intKeyLeaf = 0;
      pPage->xCellSize = cellSizePtrNoPayload;
      pPage->xParseCell = btreeParseCellPtrNoPayload;
      pPage->intKey = 1;
      pPage->maxLocal = pBt->maxLeaf;
      pPage->minLocal = pBt->minLeaf;
    } else {
      pPage->intKey = 0;
      pPage->intKeyLeaf = 0;
      pPage->xCellSize = cellSizePtr;
      pPage->xParseCell = btreeParseCellPtrIndex;
      return sqlite3CorruptError(75306);
    }
  }
  return 0;
}

int btreeComputeFreeSpace(MemPage *pPage) {
  int pc;
  u8 hdr;
  u8 *data;
  int usableSize;
  int nFree;
  int top;
  int iCellFirst;
  int iCellLast;

  usableSize = pPage->pBt->usableSize;
  hdr = pPage->hdrOffset;
  data = pPage->aData;

  top = (((((int)((&data[hdr + 5])[0] << 8 | (&data[hdr + 5])[1])) - 1) & 0xffff) + 1);
  iCellFirst = hdr + 8 + pPage->childPtrSize + 2 * pPage->nCell;
  iCellLast = usableSize - 4;

  pc = ((&data[hdr + 1])[0] << 8 | (&data[hdr + 1])[1]);
  nFree = data[hdr + 7] + top;
  if (pc > 0) {
    u32 next, size;
    if (pc < top) {

      return sqlite3CorruptError(75357);
    }
    while (1) {
      if (pc > iCellLast) {

        return sqlite3CorruptError(75362);
      }
      next = ((&data[pc])[0] << 8 | (&data[pc])[1]);
      size = ((&data[pc + 2])[0] << 8 | (&data[pc + 2])[1]);
      if (size < 4) {

        return sqlite3CorruptError(75368);
      }
      nFree = nFree + size;
      if (next < pc + size + 4)
        break;
      pc = next;
    }
    if (next > 0) {

      return sqlite3CorruptError(75376);
    }
    if (pc + size > (unsigned int)usableSize) {

      return sqlite3CorruptError(75380);
    }
  }

  if (nFree > usableSize || nFree < iCellFirst) {
    return sqlite3CorruptError(75392);
  }
  pPage->nFree = (u16)(nFree - iCellFirst);
  return 0;
}

__attribute__((noinline)) int btreeCellSizeCheck(MemPage *pPage) {
  int iCellFirst;
  int iCellLast;
  int i;
  int sz;
  int pc;
  u8 *data;
  int usableSize;
  int cellOffset;

  iCellFirst = pPage->cellOffset + 2 * pPage->nCell;
  usableSize = pPage->pBt->usableSize;
  iCellLast = usableSize - 4;
  data = pPage->aData;
  cellOffset = pPage->cellOffset;
  if (!pPage->leaf)
    iCellLast--;
  for (i = 0; i < pPage->nCell; i++) {
    pc = __builtin_bswap16(*(u16 *)(&data[cellOffset + i * 2]));
    ;
    ;
    if (pc < iCellFirst || pc > iCellLast) {
      return sqlite3CorruptError(75423);
    }
    sz = pPage->xCellSize(pPage, &data[pc]);
    ;
    if (pc + sz > usableSize) {
      return sqlite3CorruptError(75428);
    }
  }
  return 0;
}

int btreeInitPage(MemPage *pPage) {
  u8 *data;
  BtShared *pBt;

  pBt = pPage->pBt;
  data = pPage->aData + pPage->hdrOffset;

  if (decodeFlags(pPage, data[0])) {
    return sqlite3CorruptError(75460);
  }

  pPage->maskPage = (u16)(pBt->pageSize - 1);
  pPage->nOverflow = 0;
  pPage->cellOffset = (u16)(pPage->hdrOffset + 8 + pPage->childPtrSize);
  pPage->aCellIdx = data + pPage->childPtrSize + 8;
  pPage->aDataEnd = pPage->aData + pBt->pageSize;
  pPage->aDataOfst = pPage->aData + pPage->childPtrSize;

  pPage->nCell = ((&data[3])[0] << 8 | (&data[3])[1]);
  if (pPage->nCell > ((pBt->pageSize - 8) / 6)) {

    return sqlite3CorruptError(75474);
  };

  pPage->nFree = -1;
  pPage->isInit = 1;
  if (pBt->db->flags & 0x00200000) {
    return btreeCellSizeCheck(pPage);
  }
  return 0;
}

void zeroPage(MemPage *pPage, int flags) {
  unsigned char *data = pPage->aData;
  BtShared *pBt = pPage->pBt;
  int hdr = pPage->hdrOffset;
  int first;

  if (pBt->btsFlags & 0x000c) {
    memset(&data[hdr], 0, pBt->usableSize - hdr);
  }
  data[hdr] = (char)flags;
  first = hdr + ((flags & 0x08) == 0 ? 12 : 8);
  memset(&data[hdr + 1], 0, 4);
  data[hdr + 7] = 0;
  ((&data[hdr + 5])[0] = (u8)((pBt->usableSize) >> 8), (&data[hdr + 5])[1] = (u8)(pBt->usableSize));
  pPage->nFree = (u16)(pBt->usableSize - first);
  decodeFlags(pPage, flags);
  pPage->cellOffset = (u16)first;
  pPage->aDataEnd = &data[pBt->pageSize];
  pPage->aCellIdx = &data[first];
  pPage->aDataOfst = &data[pPage->childPtrSize];
  pPage->nOverflow = 0;

  pPage->maskPage = (u16)(pBt->pageSize - 1);
  pPage->nCell = 0;
  pPage->isInit = 1;
}

void releasePageNotNull(MemPage *pPage) { sqlite3PagerUnrefNotNull(pPage->pDbPage); }

void releasePage(MemPage *pPage) {
  if (pPage)
    releasePageNotNull(pPage);
}

void releasePageOne(MemPage *pPage) { sqlite3PagerUnrefPageOne(pPage->pDbPage); }

int setChildPtrmaps(MemPage *pPage) {
  int i;
  int nCell;
  int rc;
  BtShared *pBt = pPage->pBt;
  Pgno pgno = pPage->pgno;

  rc = pPage->isInit ? 0 : btreeInitPage(pPage);
  if (rc != 0)
    return rc;
  nCell = pPage->nCell;

  for (i = 0; i < nCell; i++) {
    u8 *pCell = ((pPage)->aData + ((pPage)->maskPage & __builtin_bswap16(*(u16 *)(&(pPage)->aCellIdx[2 * (i)]))));

    ptrmapPutOvflPtr(pPage, pPage, pCell, &rc);

    if (!pPage->leaf) {
      Pgno childPgno = sqlite3Get4byte(pCell);
      ptrmapPut(pBt, childPgno, 5, pgno, &rc);
    }
  }

  if (!pPage->leaf) {
    Pgno childPgno = sqlite3Get4byte(&pPage->aData[pPage->hdrOffset + 8]);
    ptrmapPut(pBt, childPgno, 5, pgno, &rc);
  }

  return rc;
}

int modifyPagePointer(MemPage *pPage, Pgno iFrom, Pgno iTo, u8 eType) {

  if (eType == 4) {

    if (sqlite3Get4byte(pPage->aData) != iFrom) {
      return sqlite3CorruptError(77111);
    }
    sqlite3Put4byte(pPage->aData, iTo);
  } else {
    int i;
    int nCell;
    int rc;

    rc = pPage->isInit ? 0 : btreeInitPage(pPage);
    if (rc)
      return rc;
    nCell = pPage->nCell;

    for (i = 0; i < nCell; i++) {
      u8 *pCell = ((pPage)->aData + ((pPage)->maskPage & __builtin_bswap16(*(u16 *)(&(pPage)->aCellIdx[2 * (i)]))));
      if (eType == 3) {
        CellInfo info;
        pPage->xParseCell(pPage, pCell, &info);
        if (info.nLocal < info.nPayload) {
          if (pCell + info.nSize > pPage->aData + pPage->pBt->usableSize) {
            return sqlite3CorruptError(77130);
          }
          if (iFrom == sqlite3Get4byte(pCell + info.nSize - 4)) {
            sqlite3Put4byte(pCell + info.nSize - 4, iTo);
            break;
          }
        }
      } else {
        if (pCell + 4 > pPage->aData + pPage->pBt->usableSize) {
          return sqlite3CorruptError(77139);
        }
        if (sqlite3Get4byte(pCell) == iFrom) {
          sqlite3Put4byte(pCell, iTo);
          break;
        }
      }
    }

    if (i == nCell) {
      if (eType != 5 || sqlite3Get4byte(&pPage->aData[pPage->hdrOffset + 8]) != iFrom) {
        return sqlite3CorruptError(77151);
      }
      sqlite3Put4byte(&pPage->aData[pPage->hdrOffset + 8], iTo);
    }
  }
  return 0;
}

int indexCellCompare(MemPage *pPage, int idx, UnpackedRecord *pIdxKey, RecordCompare xRecordCompare) {
  int c;
  int nCell;
  u8 *pCell = ((pPage)->aDataOfst + ((pPage)->maskPage & __builtin_bswap16(*(u16 *)(&(pPage)->aCellIdx[2 * (idx)]))));

  nCell = pCell[0];
  if (nCell <= pPage->max1bytePayload) {

    if (pCell + nCell >= pPage->aDataEnd)
      return 99;
    c = xRecordCompare(nCell, (void *)&pCell[1], pIdxKey);
  } else if (!(pCell[1] & 0x80) && (nCell = ((nCell & 0x7f) << 7) + pCell[1]) <= pPage->maxLocal) {

    if (pCell + nCell >= pPage->aDataEnd)
      return 99;
    c = xRecordCompare(nCell, (void *)&pCell[2], pIdxKey);
  } else {

    c = 99;
  }
  return c;
}

void freePage(MemPage *pPage, int *pRC) {
  if ((*pRC) == 0) {
    *pRC = freePage2(pPage->pBt, pPage, pPage->pgno);
  }
}

__attribute__((noinline)) int clearCellOverflow(MemPage *pPage, unsigned char *pCell, CellInfo *pInfo) {
  BtShared *pBt;
  Pgno ovflPgno;
  int rc;
  int nOvfl;
  u32 ovflPageSize;

  ;
  ;
  if (pCell + pInfo->nSize > pPage->aDataEnd) {

    return sqlite3CorruptError(80221);
  }
  ovflPgno = sqlite3Get4byte(pCell + pInfo->nSize - 4);
  pBt = pPage->pBt;

  ovflPageSize = pBt->usableSize - 4;
  nOvfl = (pInfo->nPayload - pInfo->nLocal + ovflPageSize - 1) / ovflPageSize;

  while (nOvfl--) {
    Pgno iNext = 0;
    MemPage *pOvfl = 0;
    if (ovflPgno < 2 || ovflPgno > btreePagecount(pBt)) {

      return sqlite3CorruptError(80238);
    }
    if (nOvfl) {
      rc = getOverflowPage(pBt, ovflPgno, &pOvfl, &iNext);
      if (rc)
        return rc;
    }

    if ((pOvfl || ((pOvfl = btreePageLookup(pBt, ovflPgno)) != 0)) && sqlite3PagerPageRefcount(pOvfl->pDbPage) != 1) {

      rc = sqlite3CorruptError(80258);
    } else {
      rc = freePage2(pBt, pOvfl, ovflPgno);
    }

    if (pOvfl) {
      sqlite3PagerUnref(pOvfl->pDbPage);
    }
    if (rc)
      return rc;
    ovflPgno = iNext;
  }
  return 0;
}

int fillInCell(MemPage *pPage, unsigned char *pCell, const BtreePayload *pX, int *pnSize) {
  int nPayload;
  const u8 *pSrc;
  int nSrc, n, rc, mn;
  int spaceLeft;
  MemPage *pToRelease;
  unsigned char *pPrior;
  unsigned char *pPayload;
  BtShared *pBt;
  Pgno pgnoOvfl;
  int nHeader;

  nHeader = pPage->childPtrSize;
  if (pPage->intKey) {
    nPayload = pX->nData + pX->nZero;
    pSrc = pX->pData;
    nSrc = pX->nData;

    ((void)(0))

        ;
    nHeader += (u8)(((u32)(nPayload) < (u32)0x80) ? (*(&pCell[nHeader]) = (unsigned char)(nPayload)), 1 : sqlite3PutVarint((&pCell[nHeader]), (nPayload)));
    nHeader += sqlite3PutVarint(&pCell[nHeader], *(u64 *)&pX->nKey);
  } else {

    ((void)(0))

        ;
    nSrc = nPayload = (int)pX->nKey;
    pSrc = pX->pKey;
    nHeader += (u8)(((u32)(nPayload) < (u32)0x80) ? (*(&pCell[nHeader]) = (unsigned char)(nPayload)), 1 : sqlite3PutVarint((&pCell[nHeader]), (nPayload)));
  }

  pPayload = &pCell[nHeader];
  if (nPayload <= pPage->maxLocal) {

    n = nHeader + nPayload;
    ;
    ;
    if (n < 4) {
      n = 4;
      pPayload[nPayload] = 0;
    }
    *pnSize = n;

    ((void)(0))

        ;
    ;
    memcpy(pPayload, pSrc, nSrc);
    memset(pPayload + nSrc, 0, nPayload - nSrc);
    return 0;
  }

  mn = pPage->minLocal;
  n = mn + (nPayload - mn) % (pPage->pBt->usableSize - 4);
  ;
  ;
  if (n > pPage->maxLocal)
    n = mn;
  spaceLeft = n;
  *pnSize = n + nHeader + 4;
  pPrior = &pCell[nHeader + n];
  pToRelease = 0;
  pgnoOvfl = 0;
  pBt = pPage->pBt;

  while (1) {
    n = nPayload;
    if (n > spaceLeft)
      n = spaceLeft;

    ((void)(0))

        ;

    ((void)(0))

        ;

    if (nSrc >= n) {
      memcpy(pPayload, pSrc, n);
    } else if (nSrc > 0) {
      n = nSrc;
      memcpy(pPayload, pSrc, n);
    } else {
      memset(pPayload, 0, n);
    }
    nPayload -= n;
    if (nPayload <= 0)
      break;
    pPayload += n;
    pSrc += n;
    nSrc -= n;
    spaceLeft -= n;
    if (spaceLeft == 0) {
      MemPage *pOvfl = 0;

      Pgno pgnoPtrmap = pgnoOvfl;
      if (pBt->autoVacuum) {
        do {
          pgnoOvfl++;
        } while ((ptrmapPageno((pBt), (pgnoOvfl)) == (pgnoOvfl)) || pgnoOvfl == ((Pgno)((sqlite3PendingByte / ((pBt)->pageSize)) + 1)));
      }

      rc = allocateBtreePage(pBt, &pOvfl, &pgnoOvfl, pgnoOvfl, 0);

      if (pBt->autoVacuum && rc == 0) {
        u8 eType = (pgnoPtrmap ? 4 : 3);
        ptrmapPut(pBt, pgnoOvfl, eType, pgnoPtrmap, &rc);
        if (rc) {
          releasePage(pOvfl);
        }
      }

      if (rc) {
        releasePage(pToRelease);
        return rc;
      }

      ((void)(0))

          ;

      ((void)(0))

          ;

      sqlite3Put4byte(pPrior, pgnoOvfl);
      releasePage(pToRelease);
      pToRelease = pOvfl;
      pPrior = pOvfl->aData;
      sqlite3Put4byte(pPrior, 0);
      pPayload = &pOvfl->aData[4];
      spaceLeft = pBt->usableSize - 4;
    }
  }
  releasePage(pToRelease);
  return 0;
}

void dropCell(MemPage *pPage, int idx, int sz, int *pRC) {
  u32 pc;
  u8 *data;
  u8 *ptr;
  int rc;
  int hdr;

  if (*pRC)
    return;

  data = pPage->aData;
  ptr = &pPage->aCellIdx[2 * idx];

  pc = ((ptr)[0] << 8 | (ptr)[1]);
  hdr = pPage->hdrOffset;
  ;
  ;
  if (pc + sz > pPage->pBt->usableSize) {
    *pRC = sqlite3CorruptError(80514);
    return;
  }
  rc = freeSpace(pPage, pc, sz);
  if (rc) {
    *pRC = rc;
    return;
  }
  pPage->nCell--;
  if (pPage->nCell == 0) {
    memset(&data[hdr + 1], 0, 4);
    data[hdr + 7] = 0;
    ((&data[hdr + 5])[0] = (u8)((pPage->pBt->usableSize) >> 8), (&data[hdr + 5])[1] = (u8)(pPage->pBt->usableSize));
    pPage->nFree = pPage->pBt->usableSize - pPage->hdrOffset - pPage->childPtrSize - 8;
  } else {
    memmove(ptr, ptr + 2, 2 * (pPage->nCell - idx));
    ((&data[hdr + 3])[0] = (u8)((pPage->nCell) >> 8), (&data[hdr + 3])[1] = (u8)(pPage->nCell));
    pPage->nFree += 2;
  }
}

int insertCell(MemPage *pPage, int i, u8 *pCell, int sz, u8 *pTemp, Pgno iChild) {
  int idx = 0;
  int j;
  u8 *data;
  u8 *pIns;

  if (pPage->nOverflow || sz + 2 > pPage->nFree) {
    if (pTemp) {
      memcpy(pTemp, pCell, sz);
      pCell = pTemp;
    }
    sqlite3Put4byte(pCell, iChild);
    j = pPage->nOverflow++;

    ((void)(0))

        ;
    pPage->apOvfl[j] = pCell;
    pPage->aiOvfl[j] = (u16)i;

    ((void)(0))

        ;

    ((void)(0))

        ;
  } else {
    int rc = sqlite3PagerWrite(pPage->pDbPage);
    if ((rc != 0)) {
      return rc;
    }

    ((void)(0))

        ;
    data = pPage->aData;

    ((void)(0))

        ;
    rc = allocateSpace(pPage, sz, &idx);
    if (rc) {
      return rc;
    }

    ((void)(0))

        ;

    ((void)(0))

        ;

    ((void)(0))

        ;
    pPage->nFree -= (u16)(2 + sz);

    memcpy(&data[idx + 4], pCell + 4, sz - 4);
    sqlite3Put4byte(&data[idx], iChild);
    pIns = pPage->aCellIdx + i * 2;
    memmove(pIns + 2, pIns, 2 * (pPage->nCell - i));
    ((pIns)[0] = (u8)((idx) >> 8), (pIns)[1] = (u8)(idx));
    pPage->nCell++;

    if ((++data[pPage->hdrOffset + 4]) == 0)
      data[pPage->hdrOffset + 3]++;

    ((void)(0))

        ;

    if (pPage->pBt->autoVacuum) {
      int rc2 = 0;

      ptrmapPutOvflPtr(pPage, pPage, pCell, &rc2);
      if (rc2)
        return rc2;
    }
  }
  return 0;
}

int insertCellFast(MemPage *pPage, int i, u8 *pCell, int sz) {
  int idx = 0;
  int j;
  u8 *data;
  u8 *pIns;

  if (sz + 2 > pPage->nFree) {
    j = pPage->nOverflow++;

    ((void)(0))

        ;
    pPage->apOvfl[j] = pCell;
    pPage->aiOvfl[j] = (u16)i;

    ((void)(0))

        ;

    ((void)(0))

        ;
  } else {
    int rc = sqlite3PagerWrite(pPage->pDbPage);
    if (rc != 0) {
      return rc;
    }

    ((void)(0))

        ;
    data = pPage->aData;

    ((void)(0))

        ;
    rc = allocateSpace(pPage, sz, &idx);
    if (rc) {
      return rc;
    }

    ((void)(0))

        ;

    ((void)(0))

        ;

    ((void)(0))

        ;
    pPage->nFree -= (u16)(2 + sz);
    memcpy(&data[idx], pCell, sz);
    pIns = pPage->aCellIdx + i * 2;
    memmove(pIns + 2, pIns, 2 * (pPage->nCell - i));
    ((pIns)[0] = (u8)((idx) >> 8), (pIns)[1] = (u8)(idx));
    pPage->nCell++;

    if ((++data[pPage->hdrOffset + 4]) == 0)
      data[pPage->hdrOffset + 3]++;

    ((void)(0))

        ;

    if (pPage->pBt->autoVacuum) {
      int rc2 = 0;

      ptrmapPutOvflPtr(pPage, pPage, pCell, &rc2);
      if (rc2)
        return rc2;
    }
  }
  return 0;
}

int pageInsertArray(MemPage *pPg, u8 *pBegin, u8 **ppData, u8 *pCellptr, int iFirst, int nCell, CellArray *pCArray) {
  int i = iFirst;
  u8 *aData = pPg->aData;
  u8 *pData = *ppData;
  int iEnd = iFirst + nCell;
  int k;
  u8 *pEnd;

  if (iEnd <= iFirst)
    return 0;

  for (k = 0; pCArray->ixNx[k] <= i; k++) {
  }
  pEnd = pCArray->apEnd[k];
  while (1) {
    int sz, rc;
    u8 *pSlot;

    ((void)(0))

        ;
    sz = pCArray->szCell[i];
    if ((aData[1] == 0 && aData[2] == 0) || (pSlot = pageFindSlot(pPg, sz, &rc)) == 0) {
      if ((pData - pBegin) < sz)
        return 1;
      pData -= sz;
      pSlot = pData;
    }

    ((void)(0))

        ;
    if ((uptr)(pCArray->apCell[i] + sz) > (uptr)pEnd && (uptr)(pCArray->apCell[i]) < (uptr)pEnd) {

      ((void)(0))

          ;
      (void)sqlite3CorruptError(81002);
      return 1;
    }
    memmove(pSlot, pCArray->apCell[i], sz);
    ((pCellptr)[0] = (u8)(((pSlot - aData)) >> 8), (pCellptr)[1] = (u8)((pSlot - aData)));
    pCellptr += 2;
    i++;
    if (i >= iEnd)
      break;
    if (pCArray->ixNx[k] <= i) {
      k++;
      pEnd = pCArray->apEnd[k];
    }
  }
  *ppData = pData;
  return 0;
}

int pageFreeArray(MemPage *pPg, int iFirst, int nCell, CellArray *pCArray) {
  u8 *const aData = pPg->aData;
  u8 *const pEnd = &aData[pPg->pBt->usableSize];
  u8 *const pStart = &aData[pPg->hdrOffset + 8 + pPg->childPtrSize];
  int nRet = 0;
  int i, j;
  int iEnd = iFirst + nCell;
  int nFree = 0;
  int aOfst[10];
  int aAfter[10];

  for (i = iFirst; i < iEnd; i++) {
    u8 *pCell = pCArray->apCell[i];
    if ((((uptr)(pCell) >= (uptr)(pStart)) && ((uptr)(pCell) < (uptr)(pEnd)))) {
      int sz;
      int iAfter;
      int iOfst;

      sz = pCArray->szCell[i];

      ((void)(0))

          ;
      iOfst = (u16)(pCell - aData);
      iAfter = iOfst + sz;
      for (j = 0; j < nFree; j++) {
        if (aOfst[j] == iAfter) {
          aOfst[j] = iOfst;
          break;
        } else if (aAfter[j] == iOfst) {
          aAfter[j] = iAfter;
          break;
        }
      }
      if (j >= nFree) {
        if (nFree >= (int)(sizeof(aOfst) / sizeof(aOfst[0]))) {
          for (j = 0; j < nFree; j++) {
            freeSpace(pPg, aOfst[j], aAfter[j] - aOfst[j]);
          }
          nFree = 0;
        }
        aOfst[nFree] = iOfst;
        aAfter[nFree] = iAfter;
        if (&aData[iAfter] > pEnd)
          return 0;
        nFree++;
      }
      nRet++;
    }
  }
  for (j = 0; j < nFree; j++) {
    freeSpace(pPg, aOfst[j], aAfter[j] - aOfst[j]);
  }
  return nRet;
}

int editPage(MemPage *pPg, int iOld, int iNew, int nNew, CellArray *pCArray) {
  u8 *const aData = pPg->aData;
  const int hdr = pPg->hdrOffset;
  u8 *pBegin = &pPg->aCellIdx[nNew * 2];
  int nCell = pPg->nCell;
  u8 *pData;
  u8 *pCellptr;
  int i;
  int iOldEnd = iOld + pPg->nCell + pPg->nOverflow;
  int iNewEnd = iNew + nNew;

  if (iOld < iNew) {
    int nShift = pageFreeArray(pPg, iOld, iNew - iOld, pCArray);
    if ((nShift > nCell))
      return sqlite3CorruptError(81124);
    memmove(pPg->aCellIdx, &pPg->aCellIdx[nShift * 2], nCell * 2);
    nCell -= nShift;
  }
  if (iNewEnd < iOldEnd) {
    int nTail = pageFreeArray(pPg, iNewEnd, iOldEnd - iNewEnd, pCArray);

    ((void)(0))

        ;
    nCell -= nTail;
  }

  pData = &aData[((&aData[hdr + 5])[0] << 8 | (&aData[hdr + 5])[1])];
  if (pData < pBegin)
    goto editpage_fail;
  if ((pData > pPg->aDataEnd))
    goto editpage_fail;

  if (iNew < iOld) {
    int nAdd = ((nNew) < (iOld - iNew) ? (nNew) : (iOld - iNew));

    ((void)(0))

        ;

    ((void)(0))

        ;
    pCellptr = pPg->aCellIdx;
    memmove(&pCellptr[nAdd * 2], pCellptr, nCell * 2);
    if (pageInsertArray(pPg, pBegin, &pData, pCellptr, iNew, nAdd, pCArray))
      goto editpage_fail;
    nCell += nAdd;
  }

  for (i = 0; i < pPg->nOverflow; i++) {
    int iCell = (iOld + pPg->aiOvfl[i]) - iNew;
    if (iCell >= 0 && iCell < nNew) {
      pCellptr = &pPg->aCellIdx[iCell * 2];
      if (nCell > iCell) {
        memmove(&pCellptr[2], pCellptr, (nCell - iCell) * 2);
      }
      nCell++;
      cachedCellSize(pCArray, iCell + iNew);
      if (pageInsertArray(pPg, pBegin, &pData, pCellptr, iCell + iNew, 1, pCArray))
        goto editpage_fail;
    }
  }

  pCellptr = &pPg->aCellIdx[nCell * 2];
  if (pageInsertArray(pPg, pBegin, &pData, pCellptr, iNew + nCell, nNew - nCell, pCArray)) {
    goto editpage_fail;
  }

  pPg->nCell = (u16)nNew;
  pPg->nOverflow = 0;

  ((&aData[hdr + 3])[0] = (u8)((pPg->nCell) >> 8), (&aData[hdr + 3])[1] = (u8)(pPg->nCell));
  ((&aData[hdr + 5])[0] = (u8)((pData - aData) >> 8), (&aData[hdr + 5])[1] = (u8)(pData - aData));

  return 0;
editpage_fail:

  if (nNew < 1)
    return sqlite3CorruptError(81202);
  populateCellCache(pCArray, iNew, nNew);
  return rebuildPage(pCArray, iNew, nNew, pPg);
}

int balance_quick(MemPage *pParent, MemPage *pPage, u8 *pSpace) {
  BtShared *const pBt = pPage->pBt;
  MemPage *pNew;
  int rc;
  Pgno pgnoNew;

  if (pPage->nCell == 0)
    return sqlite3CorruptError(81242);

  rc = allocateBtreePage(pBt, &pNew, &pgnoNew, 0, 0);

  if (rc == 0) {

    u8 *pOut = &pSpace[4];
    u8 *pCell = pPage->apOvfl[0];
    u16 szCell = pPage->xCellSize(pPage, pCell);
    u8 *pStop;
    CellArray b;

    ((void)(0))

        ;

    ((void)(0))

        ;
    zeroPage(pNew, 0x01 | 0x04 | 0x08);
    b.nCell = 1;
    b.pRef = pPage;
    b.apCell = &pCell;
    b.szCell = &szCell;
    b.apEnd[0] = pPage->aDataEnd;
    b.ixNx[0] = 2;
    b.ixNx[3 * 2 - 1] = 0x7fffffff;
    rc = rebuildPage(&b, 0, 1, pNew);
    if ((rc)) {
      releasePage(pNew);
      return rc;
    }
    pNew->nFree = pBt->usableSize - pNew->cellOffset - 2 - szCell;

    if ((pBt->autoVacuum)) {
      ptrmapPut(pBt, pgnoNew, 5, pParent->pgno, &rc);
      if (szCell > pNew->minLocal) {
        ptrmapPutOvflPtr(pNew, pNew, pCell, &rc);
      }
    }

    pCell = ((pPage)->aData + ((pPage)->maskPage & __builtin_bswap16(*(u16 *)(&(pPage)->aCellIdx[2 * (pPage->nCell - 1)]))));
    pStop = &pCell[9];
    while ((*(pCell++) & 0x80) && pCell < pStop)
      ;
    pStop = &pCell[9];
    while (((*(pOut++) = *(pCell++)) & 0x80) && pCell < pStop)
      ;

    if (rc == 0) {
      rc = insertCell(pParent, pParent->nCell, pSpace, (int)(pOut - pSpace), 0, pPage->pgno);
    }

    sqlite3Put4byte(&pParent->aData[pParent->hdrOffset + 8], pgnoNew);

    releasePage(pNew);
  }

  return rc;
}

void copyNodeContent(MemPage *pFrom, MemPage *pTo, int *pRC) {
  if ((*pRC) == 0) {
    BtShared *const pBt = pFrom->pBt;
    u8 *const aFrom = pFrom->aData;
    u8 *const aTo = pTo->aData;
    int const iFromHdr = pFrom->hdrOffset;
    int const iToHdr = ((pTo->pgno == 1) ? 100 : 0);
    int rc;
    int iData;

    ((void)(0))

        ;

    ((void)(0))

        ;

    ((void)(0))

        ;

    iData = ((&aFrom[iFromHdr + 5])[0] << 8 | (&aFrom[iFromHdr + 5])[1]);
    memcpy(&aTo[iData], &aFrom[iData], pBt->usableSize - iData);
    memcpy(&aTo[iToHdr], &aFrom[iFromHdr], pFrom->cellOffset + 2 * pFrom->nCell);

    pTo->isInit = 0;
    rc = btreeInitPage(pTo);
    if (rc == 0)
      rc = btreeComputeFreeSpace(pTo);
    if (rc != 0) {
      *pRC = rc;
      return;
    }

    if ((pBt->autoVacuum)) {
      *pRC = setChildPtrmaps(pTo);
    }
  }
}

int balance_nonroot(MemPage *pParent, int iParentIdx, u8 *aOvflSpace, int isRoot, int bBulk) {
  BtShared *pBt;
  int nMaxCells = 0;
  int nNew = 0;
  int nOld;
  int i, j, k;
  int nxDiv;
  int rc = 0;
  u16 leafCorrection;
  int leafData;
  int usableSpace;
  int pageFlags;
  int iSpace1 = 0;
  int iOvflSpace = 0;
  u64 szScratch;
  MemPage *apOld[3];
  MemPage *apNew[3 + 2];
  u8 *pRight;
  u8 *apDiv[3 - 1];
  int cntNew[3 + 2];
  int cntOld[3 + 2];
  int szNew[3 + 2];
  u8 *aSpace1;
  Pgno pgno;
  u8 abDone[3 + 2];
  Pgno aPgno[3 + 2];
  CellArray b;

  memset(abDone, 0, sizeof(abDone));

  memset(&b, 0, sizeof(b) - sizeof(b.ixNx[0]));
  b.ixNx[3 * 2 - 1] = 0x7fffffff;
  pBt = pParent->pBt;

  if (!aOvflSpace) {
    return 7;
  }

  i = pParent->nOverflow + pParent->nCell;
  if (i < 2) {
    nxDiv = 0;
  } else {

    ((void)(0))

        ;
    if (iParentIdx == 0) {
      nxDiv = 0;
    } else if (iParentIdx == i) {
      nxDiv = i - 2 + bBulk;
    } else {
      nxDiv = iParentIdx - 1;
    }
    i = 2 - bBulk;
  }
  nOld = i + 1;
  if ((i + nxDiv - pParent->nOverflow) == pParent->nCell) {
    pRight = &pParent->aData[pParent->hdrOffset + 8];
  } else {
    pRight = ((pParent)->aData + ((pParent)->maskPage & __builtin_bswap16(*(u16 *)(&(pParent)->aCellIdx[2 * (i + nxDiv - pParent->nOverflow)]))));
  }
  pgno = sqlite3Get4byte(pRight);
  while (1) {
    if (rc == 0) {
      rc = getAndInitPage(pBt, pgno, &apOld[i], 0);
    }
    if (rc) {
      memset(apOld, 0, (i + 1) * sizeof(MemPage *));
      goto balance_cleanup;
    }
    if (apOld[i]->nFree < 0) {
      rc = btreeComputeFreeSpace(apOld[i]);
      if (rc) {
        memset(apOld, 0, (i) * sizeof(MemPage *));
        goto balance_cleanup;
      }
    }
    nMaxCells += apOld[i]->nCell + ((int)(sizeof(pParent->apOvfl) / sizeof(pParent->apOvfl[0])));
    if ((i--) == 0)
      break;

    if (pParent->nOverflow && i + nxDiv == pParent->aiOvfl[0]) {
      apDiv[i] = pParent->apOvfl[0];
      pgno = sqlite3Get4byte(apDiv[i]);
      szNew[i] = pParent->xCellSize(pParent, apDiv[i]);
      pParent->nOverflow = 0;
    } else {
      apDiv[i] = ((pParent)->aData + ((pParent)->maskPage & __builtin_bswap16(*(u16 *)(&(pParent)->aCellIdx[2 * (i + nxDiv - pParent->nOverflow)]))));
      pgno = sqlite3Get4byte(apDiv[i]);
      szNew[i] = pParent->xCellSize(pParent, apDiv[i]);

      if (pBt->btsFlags & 0x000c) {
        int iOff;

        iOff = ((int)(intptr_t)(apDiv[i])) - ((int)(intptr_t)(pParent->aData));
        if ((iOff + szNew[i]) <= (int)pBt->usableSize) {
          memcpy(&aOvflSpace[iOff], apDiv[i], szNew[i]);
          apDiv[i] = &aOvflSpace[apDiv[i] - pParent->aData];
        }
      }
      dropCell(pParent, i + nxDiv - pParent->nOverflow, szNew[i], &rc);
    }
  }

  nMaxCells = (nMaxCells + 3) & ~3;

  szScratch = nMaxCells * sizeof(u8 *) + nMaxCells * sizeof(u16) + pBt->pageSize;

  b.apCell = sqlite3DbMallocRaw(0, szScratch);
  if (b.apCell == 0) {
    rc = 7;
    goto balance_cleanup;
  }
  b.szCell = (u16 *)&b.apCell[nMaxCells];
  aSpace1 = (u8 *)&b.szCell[nMaxCells];

  b.pRef = apOld[0];
  leafCorrection = b.pRef->leaf * 4;
  leafData = b.pRef->intKeyLeaf;
  for (i = 0; i < nOld; i++) {
    MemPage *pOld = apOld[i];
    int limit = pOld->nCell;
    u8 *aData = pOld->aData;
    u16 maskPage = pOld->maskPage;
    u8 *piCell = aData + pOld->cellOffset;
    u8 *piEnd;

    if (pOld->aData[0] != apOld[0]->aData[0]) {
      rc = sqlite3CorruptError(81666);
      goto balance_cleanup;
    }

    memset(&b.szCell[b.nCell], 0, sizeof(b.szCell[0]) * (limit + pOld->nOverflow));
    if (pOld->nOverflow > 0) {
      if ((limit < pOld->aiOvfl[0])) {
        rc = sqlite3CorruptError(81690);
        goto balance_cleanup;
      }
      limit = pOld->aiOvfl[0];
      for (j = 0; j < limit; j++) {
        b.apCell[b.nCell] = aData + (maskPage & __builtin_bswap16(*(u16 *)(piCell)));
        piCell += 2;
        b.nCell++;
      }
      for (k = 0; k < pOld->nOverflow; k++) {

        ((void)(0))

            ;
        b.apCell[b.nCell] = pOld->apOvfl[k];
        b.nCell++;
      }
    }
    piEnd = aData + pOld->cellOffset + 2 * pOld->nCell;
    while (piCell < piEnd) {

      ((void)(0))

          ;
      b.apCell[b.nCell] = aData + (maskPage & __builtin_bswap16(*(u16 *)(piCell)));
      piCell += 2;
      b.nCell++;
    }

    ((void)(0))

        ;

    cntOld[i] = b.nCell;
    if (i < nOld - 1 && !leafData) {
      u16 sz = (u16)szNew[i];
      u8 *pTemp;

      ((void)(0))

          ;
      b.szCell[b.nCell] = sz;
      pTemp = &aSpace1[iSpace1];
      iSpace1 += sz;

      ((void)(0))

          ;

      ((void)(0))

          ;
      memcpy(pTemp, apDiv[i], sz);
      b.apCell[b.nCell] = pTemp + leafCorrection;

      ((void)(0))

          ;
      b.szCell[b.nCell] = b.szCell[b.nCell] - leafCorrection;
      if (!pOld->leaf) {

        ((void)(0))

            ;

        ((void)(0))

            ;

        memcpy(b.apCell[b.nCell], &pOld->aData[8], 4);
      } else {

        ((void)(0))

            ;
        while (b.szCell[b.nCell] < 4) {

          ((void)(0))

              ;

          ((void)(0))

              ;
          aSpace1[iSpace1++] = 0x00;
          b.szCell[b.nCell]++;
        }
      }
      b.nCell++;
    }
  }

  usableSpace = pBt->usableSize - 12 + leafCorrection;
  for (i = k = 0; i < nOld; i++, k++) {
    MemPage *p = apOld[i];
    b.apEnd[k] = p->aDataEnd;
    b.ixNx[k] = cntOld[i];
    if (k && b.ixNx[k] == b.ixNx[k - 1]) {
      k--;
    }
    if (!leafData) {
      k++;
      b.apEnd[k] = pParent->aDataEnd;
      b.ixNx[k] = cntOld[i] + 1;
    }

    ((void)(0))

        ;
    szNew[i] = usableSpace - p->nFree;
    for (j = 0; j < p->nOverflow; j++) {
      szNew[i] += 2 + p->xCellSize(p, p->apOvfl[j]);
    }
    cntNew[i] = cntOld[i];
  }
  k = nOld;
  for (i = 0; i < k; i++) {
    int sz;
    while (szNew[i] > usableSpace) {
      if (i + 1 >= k) {
        k = i + 2;
        if (k > 3 + 2) {
          rc = sqlite3CorruptError(81791);
          goto balance_cleanup;
        }
        szNew[k - 1] = 0;
        cntNew[k - 1] = b.nCell;
      }
      sz = 2 + cachedCellSize(&b, cntNew[i] - 1);
      szNew[i] -= sz;
      if (!leafData) {
        if (cntNew[i] < b.nCell) {
          sz = 2 + cachedCellSize(&b, cntNew[i]);
        } else {
          sz = 0;
        }
      }
      szNew[i + 1] += sz;
      cntNew[i]--;
    }
    while (cntNew[i] < b.nCell) {
      sz = 2 + cachedCellSize(&b, cntNew[i]);
      if (szNew[i] + sz > usableSpace)
        break;
      szNew[i] += sz;
      cntNew[i]++;
      if (!leafData) {
        if (cntNew[i] < b.nCell) {
          sz = 2 + cachedCellSize(&b, cntNew[i]);
        } else {
          sz = 0;
        }
      }
      szNew[i + 1] -= sz;
    }
    if (cntNew[i] >= b.nCell) {
      k = i + 1;
    } else if (cntNew[i] <= (i > 0 ? cntNew[i - 1] : 0)) {
      rc = sqlite3CorruptError(81824);
      goto balance_cleanup;
    }
  }

  for (i = k - 1; i > 0; i--) {
    int szRight = szNew[i];
    int szLeft = szNew[i - 1];
    int r;
    int d;

    r = cntNew[i - 1] - 1;
    d = r + 1 - leafData;
    (void)cachedCellSize(&b, d);
    do {
      int szR, szD;

      ((void)(0))

          ;

      ((void)(0))

          ;
      szR = cachedCellSize(&b, r);
      szD = b.szCell[d];
      if (szRight != 0 && (bBulk || szRight + szD + 2 > szLeft - (szR + (i == k - 1 ? 0 : 2)))) {
        break;
      }
      szRight += szD + 2;
      szLeft -= szR + 2;
      cntNew[i - 1] = r;
      r--;
      d--;
    } while (r >= 0);
    szNew[i] = szRight;
    szNew[i - 1] = szLeft;
    if (cntNew[i - 1] <= (i > 1 ? cntNew[i - 2] : 0)) {
      rc = sqlite3CorruptError(81868);
      goto balance_cleanup;
    }
  }

  ;

  pageFlags = apOld[0]->aData[0];
  for (i = 0; i < k; i++) {
    MemPage *pNew;
    if (i < nOld) {
      pNew = apNew[i] = apOld[i];
      apOld[i] = 0;
      rc = sqlite3PagerWrite(pNew->pDbPage);
      nNew++;
      if (sqlite3PagerPageRefcount(pNew->pDbPage) != 1 + (i == (iParentIdx - nxDiv)) && rc == 0) {
        rc = sqlite3CorruptError(81901);
      }
      if (rc)
        goto balance_cleanup;
    } else {

      ((void)(0))

          ;
      rc = allocateBtreePage(pBt, &pNew, &pgno, (bBulk ? 1 : pgno), 0);
      if (rc)
        goto balance_cleanup;
      zeroPage(pNew, pageFlags);
      apNew[i] = pNew;
      nNew++;
      cntOld[i] = b.nCell;

      if ((pBt->autoVacuum)) {
        ptrmapPut(pBt, pNew->pgno, 5, pParent->pgno, &rc);
        if (rc != 0) {
          goto balance_cleanup;
        }
      }
    }
  }

  for (i = 0; i < nNew; i++) {
    aPgno[i] = apNew[i]->pgno;

    ((void)(0))

        ;

    ((void)(0))

        ;
  }
  for (i = 0; i < nNew - 1; i++) {
    int iB = i;
    for (j = i + 1; j < nNew; j++) {
      if (apNew[j]->pgno < apNew[iB]->pgno)
        iB = j;
    }

    if (iB != i) {
      Pgno pgnoA = apNew[i]->pgno;
      Pgno pgnoB = apNew[iB]->pgno;
      Pgno pgnoTemp = (sqlite3PendingByte / pBt->pageSize) + 1;
      u16 fgA = apNew[i]->pDbPage->flags;
      u16 fgB = apNew[iB]->pDbPage->flags;
      sqlite3PagerRekey(apNew[i]->pDbPage, pgnoTemp, fgB);
      sqlite3PagerRekey(apNew[iB]->pDbPage, pgnoA, fgA);
      sqlite3PagerRekey(apNew[i]->pDbPage, pgnoB, fgB);
      apNew[i]->pgno = pgnoB;
      apNew[iB]->pgno = pgnoA;
    }
  }

  ;

  sqlite3Put4byte(pRight, apNew[nNew - 1]->pgno);

  if ((pageFlags & 0x08) == 0 && nOld != nNew) {
    MemPage *pOld;
    if (nNew > nOld) {
      pOld = apNew[nOld - 1];
    } else {
      pOld = apOld[nOld - 1];
    }
    memcpy(&apNew[nNew - 1]->aData[8], &pOld->aData[8], 4);
  }

  if ((pBt->autoVacuum)) {
    MemPage *pOld;
    MemPage *pNew = pOld = apNew[0];
    int cntOldNext = pNew->nCell + pNew->nOverflow;
    int iNew = 0;
    int iOld = 0;

    for (i = 0; i < b.nCell; i++) {
      u8 *pCell = b.apCell[i];
      while (i == cntOldNext) {
        iOld++;

        ((void)(0))

            ;

        ((void)(0))

            ;
        pOld = iOld < nNew ? apNew[iOld] : apOld[iOld];
        cntOldNext += pOld->nCell + pOld->nOverflow + !leafData;
      }
      if (i == cntNew[iNew]) {
        pNew = apNew[++iNew];
        if (!leafData)
          continue;
      }

      if (iOld >= nNew || pNew->pgno != aPgno[iOld] || !(((uptr)(pCell) >= (uptr)(pOld->aData)) && ((uptr)(pCell) < (uptr)(pOld->aDataEnd)))) {
        if (!leafCorrection) {
          ptrmapPut(pBt, sqlite3Get4byte(pCell), 5, pNew->pgno, &rc);
        }
        if (cachedCellSize(&b, i) > pNew->minLocal) {
          ptrmapPutOvflPtr(pNew, pOld, pCell, &rc);
        }
        if (rc)
          goto balance_cleanup;
      }
    }
  }

  for (i = 0; i < nNew - 1; i++) {
    u8 *pCell;
    u8 *pTemp;
    int sz;
    u8 *pSrcEnd;
    MemPage *pNew = apNew[i];
    j = cntNew[i];

    ((void)(0))

        ;

    ((void)(0))

        ;
    pCell = b.apCell[j];
    sz = b.szCell[j] + leafCorrection;
    pTemp = &aOvflSpace[iOvflSpace];
    if (!pNew->leaf) {
      memcpy(&pNew->aData[8], pCell, 4);
    } else if (leafData) {

      CellInfo info;
      j--;
      pNew->xParseCell(pNew, b.apCell[j], &info);
      pCell = pTemp;
      sz = 4 + sqlite3PutVarint(&pCell[4], info.nKey);
      pTemp = 0;
    } else {
      pCell -= 4;

      if (b.szCell[j] == 4) {

        ((void)(0))

            ;
        sz = pParent->xCellSize(pParent, pCell);
      }
    }
    iOvflSpace += sz;

    ((void)(0))

        ;

    ((void)(0))

        ;

    ((void)(0))

        ;
    for (k = 0; b.ixNx[k] <= j; k++) {
    }
    pSrcEnd = b.apEnd[k];
    if ((((uptr)(pCell) < (uptr)(pSrcEnd)) && ((uptr)(pCell + sz) > (uptr)(pSrcEnd)))) {
      rc = sqlite3CorruptError(82107);
      goto balance_cleanup;
    }
    rc = insertCell(pParent, nxDiv + i, pCell, sz, pTemp, pNew->pgno);
    if (rc != 0)
      goto balance_cleanup;

    ((void)(0))

        ;
  }

  for (i = 1 - nNew; i < nNew; i++) {
    int iPg = i < 0 ? -i : i;

    ((void)(0))

        ;

    ((void)(0))

        ;

    ((void)(0))

        ;
    if (abDone[iPg])
      continue;
    if (i >= 0 || cntOld[iPg - 1] >= cntNew[iPg - 1]) {
      int iNew;
      int iOld;
      int nNewCell;

      ((void)(0))

          ;

      ((void)(0))

          ;

      if (iPg == 0) {
        iNew = iOld = 0;
        nNewCell = cntNew[0];
      } else {
        iOld = iPg < nOld ? (cntOld[iPg - 1] + !leafData) : b.nCell;
        iNew = cntNew[iPg - 1] + !leafData;
        nNewCell = cntNew[iPg] - iNew;
      }

      rc = editPage(apNew[iPg], iOld, iNew, nNewCell, &b);
      if (rc)
        goto balance_cleanup;
      abDone[iPg]++;
      apNew[iPg]->nFree = usableSpace - szNew[iPg];

      ((void)(0))

          ;

      ((void)(0))

          ;
    }
  }

  if (isRoot && pParent->nCell == 0 && pParent->hdrOffset <= apNew[0]->nFree) {

    ((void)(0))

        ;
    rc = defragmentPage(apNew[0], -1);
    ;

    ((void)(0))

        ;
    copyNodeContent(apNew[0], pParent, &rc);
    freePage(apNew[0], &rc);
  } else if ((pBt->autoVacuum) && !leafCorrection) {

    for (i = 0; i < nNew; i++) {
      u32 key = sqlite3Get4byte(&apNew[i]->aData[8]);
      ptrmapPut(pBt, key, 5, apNew[i]->pgno, &rc);
    }
  }

  ;

  for (i = nNew; i < nOld; i++) {
    freePage(apOld[i], &rc);
  }

balance_cleanup:
  sqlite3DbFree(0, b.apCell);
  for (i = 0; i < nOld; i++) {
    releasePage(apOld[i]);
  }
  for (i = 0; i < nNew; i++) {
    releasePage(apNew[i]);
  }

  return rc;
}

int balance_deeper(MemPage *pRoot, MemPage **ppChild) {
  int rc;
  MemPage *pChild = 0;
  Pgno pgnoChild = 0;
  BtShared *pBt = pRoot->pBt;

  rc = sqlite3PagerWrite(pRoot->pDbPage);
  if (rc == 0) {
    rc = allocateBtreePage(pBt, &pChild, &pgnoChild, pRoot->pgno, 0);
    copyNodeContent(pRoot, pChild, &rc);
    if ((pBt->autoVacuum)) {
      ptrmapPut(pBt, pgnoChild, 5, pRoot->pgno, &rc);
    }
  }
  if (rc) {
    *ppChild = 0;
    releasePage(pChild);
    return rc;
  }

  ;

  memcpy(pChild->aiOvfl, pRoot->aiOvfl, pRoot->nOverflow * sizeof(pRoot->aiOvfl[0]));
  memcpy(pChild->apOvfl, pRoot->apOvfl, pRoot->nOverflow * sizeof(pRoot->apOvfl[0]));
  pChild->nOverflow = pRoot->nOverflow;

  zeroPage(pRoot, pChild->aData[0] & ~0x08);
  sqlite3Put4byte(&pRoot->aData[pRoot->hdrOffset + 8], pgnoChild);

  *ppChild = pChild;
  return 0;
}

int btreeOverwriteContent(MemPage *pPage, u8 *pDest, const BtreePayload *pX, int iOffset, int iAmt) {
  int nData = pX->nData - iOffset;
  if (nData <= 0) {

    int i;
    for (i = 0; i < iAmt && pDest[i] == 0; i++) {
    }
    if (i < iAmt) {
      int rc = sqlite3PagerWrite(pPage->pDbPage);
      if (rc)
        return rc;
      memset(pDest + i, 0, iAmt - i);
    }
  } else {
    if (nData < iAmt) {

      int rc = btreeOverwriteContent(pPage, pDest + nData, pX, iOffset + nData, iAmt - nData);
      if (rc)
        return rc;
      iAmt = nData;
    }
    if (memcmp(pDest, ((u8 *)pX->pData) + iOffset, iAmt) != 0) {
      int rc = sqlite3PagerWrite(pPage->pDbPage);
      if (rc)
        return rc;

      memmove(pDest, ((u8 *)pX->pData) + iOffset, iAmt);
    }
  }
  return 0;
}
