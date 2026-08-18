
#pragma once

  struct Table;
  struct VTable;
  struct VtabCtx;
  struct VtabCtx {
    VTable *pVTable;
    Table *pTab;
    VtabCtx *pPrior;
    int bDeclared;
  };


