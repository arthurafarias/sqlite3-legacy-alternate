
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/Bitvec.h"
#include "sqlite/sqlite3_libversion.h"
#include "sqlite/sqlite3_libversion_number.h"
#include "sqlite/sqlite3_sourceid.h"
#include "sqlite/Pgno.h"
#include "sqlite/i64.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
  typedef struct BtLock BtLock;
  typedef struct Btree Btree;

  typedef struct Pager Pager;
  typedef struct sqlite3 sqlite3;
  typedef struct BtCursor BtCursor;
  typedef struct MemPage MemPage;

  typedef struct BtShared BtShared;
  typedef struct sqlite3_mutex sqlite3_mutex;
  typedef struct Bitvec Bitvec;

  struct BtShared {
    Pager *pPager;
    sqlite3 *db;
    BtCursor *pCursor;
    MemPage *pPage1;
    u8 openFlags;

    u8 autoVacuum;
    u8 incrVacuum;
    u8 bDoTruncate;

    u8 inTransaction;
    u8 max1bytePayload;
    u8 nReserveWanted;
    u16 btsFlags;
    u16 maxLocal;
    u16 minLocal;
    u16 maxLeaf;
    u16 minLeaf;
    u32 pageSize;
    u32 usableSize;
    int nTransaction;
    u32 nPage;
    void *pSchema;
    void (*xFreeSchema)(void *);
    sqlite3_mutex *mutex;
    Bitvec *pHasContent;

    int nRef;
    BtShared *pNext;
    BtLock *pLock;
    Btree *pWriter;

    u8 *pTmpSpace;
    int nPreformatSize;
  };

  extern BtShared *sqlite3SharedCacheList;
  void invalidateAllOverflowCache(BtShared * pBt);
  int btreeSetHasContent(BtShared * pBt, Pgno pgno);
  int btreeGetHasContent(BtShared * pBt, Pgno pgno);
  void btreeClearHasContent(BtShared * pBt);
  int saveAllCursors(BtShared * pBt, Pgno iRoot, BtCursor * pExcept);
  Pgno ptrmapPageno(BtShared * pBt, Pgno pgno);
  void ptrmapPut(BtShared * pBt, Pgno key, u8 eType, Pgno parent, int *pRC);
  int ptrmapGet(BtShared * pBt, Pgno key, u8 * pEType, Pgno * pPgno);
  int btreeGetPage(BtShared * pBt, Pgno pgno, MemPage * *ppPage, int flags);
  MemPage *btreePageLookup(BtShared * pBt, Pgno pgno);
  Pgno btreePagecount(BtShared * pBt);
  int getAndInitPage(BtShared * pBt, Pgno pgno, MemPage * *ppPage, int bReadOnly);
  int btreeGetUnusedPage(BtShared * pBt, Pgno pgno, MemPage * *ppPage, int flags);
  int removeFromSharingList(BtShared * pBt);
  __attribute__((noinline)) int allocateTempSpace(BtShared * pBt);
  void freeTempSpace(BtShared * pBt);
  int newDatabase(BtShared *);
  int lockBtree(BtShared * pBt);
  void unlockBtreeIfUnused(BtShared * pBt);
  int relocatePage(BtShared * pBt, MemPage * pDbPage, u8 eType, Pgno iPtrPage, Pgno iFreePage, int isCommit);
  int allocateBtreePage(BtShared *, MemPage **, Pgno *, Pgno, u8);
  int incrVacuumStep(BtShared * pBt, Pgno nFin, Pgno iLastPg, int bCommit);
  Pgno finalDbSize(BtShared * pBt, Pgno nOrig, Pgno nFree);
  void btreeSetNPage(BtShared * pBt, MemPage * pPage1);
  int getOverflowPage(BtShared * pBt, Pgno ovfl, MemPage * *ppPage, Pgno * pPgnoNext);
  int freePage2(BtShared * pBt, MemPage * pMemPage, Pgno iPage);
  int clearDatabasePage(BtShared * pBt, Pgno pgno, int freePageFlag, i64 *pnChange);

#ifdef __cplusplus
}
#endif
