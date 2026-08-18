
#pragma once

#include "sqlite/CellInfo.h"
#include "sqlite/Pgno.h"
#include "sqlite/i64.h"
#include "sqlite/i8.h"
#include "sqlite/u16.h"
#include "sqlite/u8.h"
#include "sqlite/BitMask.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/u32.h"
struct Index;
struct KeyInfo;
typedef struct sqlite3_value Mem;

struct MemPage;
struct Btree;

struct BtCursor;
struct BtShared;
struct UnpackedRecord;
struct BtreePayload;

struct BtCursor {
  u8 eState;
  u8 curFlags;
  u8 curPagerFlags;
  u8 hints;
  int skipNext;

  Btree *pBtree;
  Pgno *aOverflow;
  void *pKey;

  BtShared *pBt;
  BtCursor *pNext;
  CellInfo info;
  i64 nKey;
  Pgno pgnoRoot;
  i8 iPage;
  u8 curIntKey;
  u16 ix;
  u16 aiIdx[20 - 1];
  struct KeyInfo *pKeyInfo;
  MemPage *pPage;
  MemPage *apPage[20 - 1];
};

BtCursor *sqlite3BtreeFakeValidCursor(void);
int sqlite3BtreeClearTableOfCursor(BtCursor *);
void sqlite3BtreeCursorZero(BtCursor *);
void sqlite3BtreeCursorHintFlags(BtCursor *, unsigned);
int sqlite3BtreeCloseCursor(BtCursor *);
int sqlite3BtreeTableMoveto(BtCursor *, i64 intKey, int bias, int *pRes);
int sqlite3BtreeIndexMoveto(BtCursor *, UnpackedRecord *pUnKey, int *pRes);
int sqlite3BtreeCursorHasMoved(BtCursor *);
int sqlite3BtreeCursorRestore(BtCursor *, int *);
int sqlite3BtreeDelete(BtCursor *, u8 flags);
int sqlite3BtreeInsert(BtCursor *, const BtreePayload *pPayload, int flags, int seekResult);
int sqlite3BtreeFirst(BtCursor *, int *pRes);
int sqlite3BtreeIsEmpty(BtCursor *pCur, int *pRes);
int sqlite3BtreeLast(BtCursor *, int *pRes);
int sqlite3BtreeNext(BtCursor *, int flags);
int sqlite3BtreeEof(BtCursor *);
int sqlite3BtreePrevious(BtCursor *, int flags);
i64 sqlite3BtreeIntegerKey(BtCursor *);
void sqlite3BtreeCursorPin(BtCursor *);
void sqlite3BtreeCursorUnpin(BtCursor *);
i64 sqlite3BtreeOffset(BtCursor *);
int sqlite3BtreePayload(BtCursor *, u32 offset, u32 amt, void *);
const void *sqlite3BtreePayloadFetch(BtCursor *, u32 *pAmt);
u32 sqlite3BtreePayloadSize(BtCursor *);
sqlite3_int64 sqlite3BtreeMaxRecordSize(BtCursor *);
i64 sqlite3BtreeRowCountEst(BtCursor *);
int sqlite3BtreePayloadChecked(BtCursor *, u32 offset, u32 amt, void *);
int sqlite3BtreePutData(BtCursor *, u32 offset, u32 amt, void *);
void sqlite3BtreeIncrblobCursor(BtCursor *);
void sqlite3BtreeClearCursor(BtCursor *);
int sqlite3BtreeCursorHasHint(BtCursor *, unsigned int mask);
int sqlite3BtreeCursorIsValidNN(BtCursor *);
int sqlite3BtreeTransferRow(BtCursor *, BtCursor *, i64);
void sqlite3BtreeEnterCursor(BtCursor *);
void sqlite3BtreeLeaveCursor(BtCursor *);

int sqlite3VdbeMemFromBtree(BtCursor *, u32, u32, Mem *);
int sqlite3VdbeMemFromBtreeZeroOffset(BtCursor *, u32, Mem *);
int sqlite3VdbeFindIndexKey(BtCursor *, Index *, UnpackedRecord *, int *, int);
void btreeReleaseAllCursorPages(BtCursor *pCur);
int saveCursorKey(BtCursor *pCur);
int saveCursorPosition(BtCursor *pCur);
int __attribute__((noinline)) saveCursorsOnList(BtCursor *, Pgno, BtCursor *);
int btreeMoveto(BtCursor *pCur, const void *pKey, i64 nKey, int bias, int *pRes);
int btreeRestoreCursorPosition(BtCursor *pCur);
__attribute__((noinline)) void getCellInfo(BtCursor *pCur);
int accessPayload(BtCursor *pCur, u32 offset, u32 amt, unsigned char *pBuf, int eOp);
__attribute__((noinline)) int accessPayloadChecked(BtCursor *pCur, u32 offset, u32 amt, void *pBuf);
const void *fetchPayload(BtCursor *pCur, u32 *pAmt);
int moveToChild(BtCursor *pCur, u32 newPgno);
void moveToParent(BtCursor *pCur);
int moveToRoot(BtCursor *pCur);
int moveToLeftmost(BtCursor *pCur);
int moveToRightmost(BtCursor *pCur);
__attribute__((noinline)) int btreeLast(BtCursor *pCur, int *pRes);
int cursorOnLastPage(BtCursor *pCur);
__attribute__((noinline)) int btreeNext(BtCursor *pCur);
__attribute__((noinline)) int btreePrevious(BtCursor *pCur);
int anotherValidCursor(BtCursor *pCur);
int balance(BtCursor *pCur);
__attribute__((noinline)) int btreeOverwriteOverflowCell(BtCursor *pCur, const BtreePayload *pX);
int btreeOverwriteCell(BtCursor *pCur, const BtreePayload *pX);
int vdbeIsMatchingIndexKey(BtCursor *pCur, int bInt, Bitmask mask, UnpackedRecord *p, int *piRes);
