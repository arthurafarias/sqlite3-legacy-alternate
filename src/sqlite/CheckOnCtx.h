
#pragma once

  typedef struct SrcList SrcList;
  typedef struct CheckOnCtx CheckOnCtx;
  struct CheckOnCtx {
    SrcList *pSrc;
    int iJoin;
    int bFuncArg;
    CheckOnCtx *pParent;
  };


