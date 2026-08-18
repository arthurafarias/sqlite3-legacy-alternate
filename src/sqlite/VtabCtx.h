
#pragma once

  typedef struct Table Table;
  typedef struct VTable VTable;
  typedef struct VtabCtx VtabCtx;
  struct VtabCtx {
    VTable *pVTable;
    Table *pTab;
    VtabCtx *pPrior;
    int bDeclared;
  };


