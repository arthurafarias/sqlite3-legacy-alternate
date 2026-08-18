
#pragma once

#include "sqlite/DbPage.h"
#include "sqlite/Pgno.h"
#include "sqlite/RecordCompare.h"
#include "sqlite/i64.h"
#include "sqlite/u16.h"
#include "sqlite/u8.h"
  struct BtreePayload;
  struct CellArray;
  struct CellInfo;


  struct MemPage {
    u8 isInit;
    u8 intKey;
    u8 intKeyLeaf;
    Pgno pgno;

    u8 leaf;
    u8 hdrOffset;
    u8 childPtrSize;
    u8 max1bytePayload;
    u8 nOverflow;
    u16 maxLocal;
    u16 minLocal;
    u16 cellOffset;
    int nFree;
    u16 nCell;
    u16 maskPage;
    u16 aiOvfl[4];

    u8 *apOvfl[4];
    BtShared *pBt;
    u8 *aData;
    u8 *aDataEnd;

    u8 *aCellIdx;
    u8 *aDataOfst;
    DbPage *pDbPage;
    u16 (*xCellSize)(MemPage *, u8 *);
    void (*xParseCell)(MemPage *, u8 *, CellInfo *);
  };

  void releasePage(MemPage * pPage);
  void releasePageOne(MemPage * pPage);
  void releasePageNotNull(MemPage * pPage);
  __attribute__((noinline)) void btreeParseCellAdjustSizeForOverflow(MemPage * pPage, u8 * pCell, CellInfo * pInfo);
  int btreePayloadToLocal(MemPage * pPage, i64 nPayload);
  void btreeParseCellPtrNoPayload(MemPage * pPage, u8 * pCell, CellInfo * pInfo);
  void btreeParseCellPtr(MemPage * pPage, u8 * pCell, CellInfo * pInfo);
  void btreeParseCellPtrIndex(MemPage * pPage, u8 * pCell, CellInfo * pInfo);
  void btreeParseCell(MemPage * pPage, int iCell, CellInfo *pInfo);
  u16 cellSizePtr(MemPage * pPage, u8 * pCell);
  u16 cellSizePtrIdxLeaf(MemPage * pPage, u8 * pCell);
  u16 cellSizePtrNoPayload(MemPage * pPage, u8 * pCell);
  u16 cellSizePtrTableLeaf(MemPage * pPage, u8 * pCell);
  void ptrmapPutOvflPtr(MemPage * pPage, MemPage * pSrc, u8 * pCell, int *pRC);
  int defragmentPage(MemPage * pPage, int nMaxFrag);
  u8 *pageFindSlot(MemPage * pPg, int nByte, int *pRc);
  int freeSpace(MemPage * pPage, int iStart, int iSize);
  int decodeFlags(MemPage * pPage, int flagByte);
  int btreeComputeFreeSpace(MemPage * pPage);
  __attribute__((noinline)) int btreeCellSizeCheck(MemPage * pPage);
  int btreeInitPage(MemPage * pPage);
  void zeroPage(MemPage * pPage, int flags);
  int setChildPtrmaps(MemPage * pPage);
  int modifyPagePointer(MemPage * pPage, Pgno iFrom, Pgno iTo, u8 eType);
  int indexCellCompare(MemPage * pPage, int idx, UnpackedRecord *pIdxKey, RecordCompare xRecordCompare);
  void freePage(MemPage * pPage, int *pRC);
  __attribute__((noinline)) int clearCellOverflow(MemPage * pPage, unsigned char *pCell, CellInfo *pInfo);
  int fillInCell(MemPage * pPage, unsigned char *pCell, const BtreePayload *pX, int *pnSize);
  void dropCell(MemPage * pPage, int idx, int sz, int *pRC);
  int insertCell(MemPage * pPage, int i, u8 *pCell, int sz, u8 *pTemp, Pgno iChild);
  int insertCellFast(MemPage * pPage, int i, u8 *pCell, int sz);
  int pageInsertArray(MemPage * pPg, u8 * pBegin, u8 * *ppData, u8 * pCellptr, int iFirst, int nCell,
                      CellArray *pCArray);
  int pageFreeArray(MemPage * pPg, int iFirst, int nCell, CellArray *pCArray);
  int editPage(MemPage * pPg, int iOld, int iNew, int nNew, CellArray *pCArray);
  int balance_quick(MemPage * pParent, MemPage * pPage, u8 * pSpace);
  void copyNodeContent(MemPage * pFrom, MemPage * pTo, int *pRC);
  int balance_nonroot(MemPage * pParent, int iParentIdx, u8 *aOvflSpace, int isRoot, int bBulk);
  int balance_deeper(MemPage * pRoot, MemPage * *ppChild);
  int btreeOverwriteContent(MemPage * pPage, u8 * pDest, const BtreePayload *pX, int iOffset, int iAmt);


