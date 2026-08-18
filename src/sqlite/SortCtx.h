
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/u8.h"
  typedef struct ExprList ExprList;
  typedef struct RowLoadInfo RowLoadInfo;
  typedef struct SortCtx SortCtx;
  struct SortCtx {
    ExprList *pOrderBy;
    int nOBSat;
    int iECursor;
    int regReturn;
    int labelBkOut;
    int addrSortIndex;
    int labelDone;
    int labelOBLopt;
    u8 sortFlags;
    struct RowLoadInfo *pDeferredRowLoad;
  };

#ifdef __cplusplus
}
#endif
