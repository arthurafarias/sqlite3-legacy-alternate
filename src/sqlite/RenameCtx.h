
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
  typedef struct RenameToken RenameToken;
  typedef struct Table Table;
  typedef struct RenameCtx RenameCtx;
  struct RenameCtx {
    RenameToken *pList;
    int nList;
    int iCol;
    Table *pTab;
    const char *zOld;
  };

  RenameToken *renameColumnTokenNext(RenameCtx * pCtx);

#ifdef __cplusplus
}
#endif
