
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/u8.h"
  typedef struct Expr Expr;
  typedef struct ExprList ExprList;
  typedef struct FuncDef FuncDef;
  typedef struct Parse Parse;
  typedef struct Select Select;
  typedef struct Window Window;

  struct Window {
    char *zName;
    char *zBase;
    ExprList *pPartition;
    ExprList *pOrderBy;
    u8 eFrmType;
    u8 eStart;
    u8 eEnd;
    u8 bImplicitFrame;
    u8 eExclude;
    Expr *pStart;
    Expr *pEnd;
    Window **ppThis;
    Window *pNextWin;
    Expr *pFilter;
    FuncDef *pWFunc;
    int iEphCsr;
    int regAccum;
    int regResult;
    int csrApp;
    int regApp;
    int regPart;
    Expr *pOwner;
    int nBufferCol;
    int iArgCol;
    int regOne;
    int regStartRowid;
    int regEndRowid;
    u8 bExprArgs;
  };

  void sqlite3WindowUnlinkFromSelect(Window *);
  void sqlite3WindowLink(Select * pSel, Window * pWin);
  int sqlite3WindowCompare(const Parse *, const Window *, const Window *, int);
  void sqlite3WindowFunctions(void);

  int windowArgCount(Window * pWin);
  int windowCacheFrame(Window * pMWin);

#ifdef __cplusplus
}
#endif
