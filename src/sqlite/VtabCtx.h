
#pragma once
#ifdef __cplusplus
extern C {
#endif
  typedef struct Table Table;
  typedef struct VTable VTable;
  typedef struct VtabCtx VtabCtx;
  struct VtabCtx {
    VTable *pVTable;
    Table *pTab;
    VtabCtx *pPrior;
    int bDeclared;
  };

#ifdef __cplusplus
}
#endif
