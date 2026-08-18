
#pragma once

  struct RenameToken;
  struct Table;
  struct RenameCtx;
  struct RenameCtx {
    RenameToken *pList;
    int nList;
    int iCol;
    Table *pTab;
    const char *zOld;
  };

  RenameToken *renameColumnTokenNext(RenameCtx * pCtx);


