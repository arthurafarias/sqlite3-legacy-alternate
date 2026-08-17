
#pragma once
#ifdef __cplusplus
extern C {
#endif

  typedef struct ExprList ExprList;
  typedef struct Select Select;
  typedef struct SrcList SrcList;
  typedef struct Table Table;
  typedef struct Window Window;
  typedef struct WindowRewrite WindowRewrite;
  struct WindowRewrite {
    Window *pWin;
    SrcList *pSrc;
    ExprList *pSub;
    Table *pTab;
    Select *pSubSelect;
  };

#ifdef __cplusplus
}
#endif
