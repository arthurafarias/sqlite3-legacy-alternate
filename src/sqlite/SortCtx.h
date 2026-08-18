
#pragma once

#include "sqlite/u8.h"
  struct ExprList;
  struct RowLoadInfo;
  struct SortCtx;
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


