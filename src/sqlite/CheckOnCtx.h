
#pragma once
#ifdef __cplusplus
extern C {
#endif

  typedef struct SrcList SrcList;
  typedef struct CheckOnCtx CheckOnCtx;
  struct CheckOnCtx {
    SrcList *pSrc;
    int iJoin;
    int bFuncArg;
    CheckOnCtx *pParent;
  };

#ifdef __cplusplus
}
#endif
