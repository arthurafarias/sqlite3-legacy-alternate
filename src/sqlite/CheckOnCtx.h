
#pragma once

  struct SrcList;
  struct CheckOnCtx;
  struct CheckOnCtx {
    SrcList *pSrc;
    int iJoin;
    int bFuncArg;
    CheckOnCtx *pParent;
  };


