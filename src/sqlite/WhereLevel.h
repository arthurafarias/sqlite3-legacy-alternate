
#pragma once

#include "sqlite/BitMask.h"
#include "sqlite/InLoop.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
struct Index;
struct WhereLoop;
struct WhereRightJoin;
struct WhereTerm;

struct WhereLevel;
struct WhereLevel {
  int iLeftJoin;
  int iTabCur;
  int iIdxCur;
  int addrBrk;
  int addrHalt;
  int addrNxt;
  int addrSkip;
  int addrCont;
  int addrFirst;
  int addrBody;
  int regBignull;
  int addrBignull;

  u32 iLikeRepCntr;
  int addrLikeRep;

  int regFilter;
  WhereRightJoin *pRJ;
  u8 iFrom;
  u8 op, p3, p5;
  int p1, p2;
  union {
    struct {
      int nIn;
      InLoop *aInLoop;
    } in;
    Index *pCoveringIdx;
  } u;
  struct WhereLoop *pWLoop;
  Bitmask notReady;
};

void disableTerm(WhereLevel *pLevel, WhereTerm *pTerm);


