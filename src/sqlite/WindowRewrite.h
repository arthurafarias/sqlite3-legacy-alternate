
#pragma once

  struct ExprList;
  struct Select;
  struct SrcList;
  struct Table;
  struct Window;
  struct WindowRewrite;
  struct WindowRewrite {
    Window *pWin;
    SrcList *pSrc;
    ExprList *pSub;
    Table *pTab;
    Select *pSubSelect;
  };


