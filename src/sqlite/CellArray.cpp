#define _GNU_SOURCE 1
#include <string.h>
#include "sqlite/CellArray.h"
#include "sqlite/BtShared.h"
#include "sqlite/MemPage.h"
#include "sqlite/Pager.h"
#include "sqlite/sqlite3.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
#include "sqlite/uptr.h"
#include "sqlite/SqliteResultCode.h"
void populateCellCache(CellArray *p, int idx, int N) {
  MemPage *pRef = p->pRef;
  u16 *szCell = p->szCell;

  while (N > 0) {
    if (szCell[idx] == 0) {
      szCell[idx] = pRef->xCellSize(pRef, p->apCell[idx]);
    } else {
    }
    idx++;
    N--;
  }
}

__attribute__((noinline)) u16 computeCellSize(CellArray *p, int N) {
  p->szCell[N] = p->pRef->xCellSize(p->pRef, p->apCell[N]);
  return p->szCell[N];
}

u16 cachedCellSize(CellArray *p, int N) {
  if (p->szCell[N])
    return p->szCell[N];
  return computeCellSize(p, N);
}

int rebuildPage(CellArray *pCArray, int iFirst, int nCell, MemPage *pPg) {
  const int hdr = pPg->hdrOffset;
  u8 *const aData = pPg->aData;
  const int usableSize = pPg->pBt->usableSize;
  u8 *const pEnd = &aData[usableSize];
  int i = iFirst;
  u32 j;
  int iEnd = i + nCell;
  u8 *pCellptr = pPg->aCellIdx;
  u8 *pTmp = (u8*)(sqlite3PagerTempSpace(pPg->pBt->pPager));
  u8 *pData;
  int k;
  u8 *pSrcEnd;

  j = ((&aData[hdr + 5])[0] << 8 | (&aData[hdr + 5])[1]);
  if (j > (u32)usableSize) {
    j = 0;
  }
  memcpy(&pTmp[j], &aData[j], usableSize - j);

  for (k = 0; pCArray->ixNx[k] <= i; k++) {
  }
  pSrcEnd = pCArray->apEnd[k];

  pData = pEnd;
  while (1) {
    u8 *pCell = pCArray->apCell[i];
    u16 sz = pCArray->szCell[i];

    if ((((uptr)(pCell) >= (uptr)(aData + j)) && ((uptr)(pCell) < (uptr)(pEnd)))) {
      if (((uptr)(pCell + sz)) > (uptr)pEnd)
        return sqlite3CorruptError(80904);
      pCell = &pTmp[pCell - aData];
    } else if ((uptr)(pCell + sz) > (uptr)pSrcEnd && (uptr)(pCell) < (uptr)pSrcEnd) {
      return sqlite3CorruptError(80909);
    }

    pData -= sz;
    ((pCellptr)[0] = (u8)(((pData - aData)) >> 8), (pCellptr)[1] = (u8)((pData - aData)));
    pCellptr += 2;
    if (pData < pCellptr)
      return sqlite3CorruptError(80915);
    memmove(pData, pCell, sz);

    i++;
    if (i >= iEnd)
      break;
    if (pCArray->ixNx[k] <= i) {
      k++;
      pSrcEnd = pCArray->apEnd[k];
    }
  }

  pPg->nCell = (u16)nCell;
  pPg->nOverflow = 0;

  ((&aData[hdr + 1])[0] = (u8)((0) >> 8), (&aData[hdr + 1])[1] = (u8)(0));
  ((&aData[hdr + 3])[0] = (u8)((pPg->nCell) >> 8), (&aData[hdr + 3])[1] = (u8)(pPg->nCell));
  ((&aData[hdr + 5])[0] = (u8)((pData - aData) >> 8), (&aData[hdr + 5])[1] = (u8)(pData - aData));
  aData[hdr + 7] = 0x00;
  return SQLITE_OK;
}
