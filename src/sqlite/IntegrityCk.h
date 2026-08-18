
#pragma once
#include "sqlite/Pgno.h"

#include "sqlite/StrAccum.h"
#include "sqlite/i64.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
  struct BtShared;
  struct Pager;

  struct IntegrityCk;
  struct IntegrityCk {
    BtShared *pBt;
    Pager *pPager;
    u8 *aPgRef;
    Pgno nCkPage;
    int mxErr;
    int nErr;
    int rc;
    u32 nStep;
    const char *zPfx;
    Pgno v0;
    Pgno v1;
    int v2;
    StrAccum errMsg;
    u32 *heap;
    sqlite3 *db;
    i64 nRow;
  };

  void checkOom(IntegrityCk * pCheck);
  void checkProgress(IntegrityCk * pCheck);
  void checkAppendMsg(IntegrityCk * pCheck, const char *zFormat, ...);
  int getPageReferenced(IntegrityCk * pCheck, Pgno iPg);
  void setPageReferenced(IntegrityCk * pCheck, Pgno iPg);
  int checkRef(IntegrityCk * pCheck, Pgno iPage);
  void checkPtrmap(IntegrityCk * pCheck, Pgno iChild, u8 eType, Pgno iParent);
  void checkList(IntegrityCk * pCheck, int isFreeList, Pgno iPage, u32 N);
  int checkTreePage(IntegrityCk * pCheck, Pgno iPage, i64 * piMinKey, i64 maxKey);


