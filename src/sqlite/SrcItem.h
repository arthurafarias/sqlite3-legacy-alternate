
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/BitMask.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
  typedef struct CteUse CteUse;
  typedef struct Expr Expr;
  typedef struct ExprList ExprList;
  typedef struct IdList IdList;
  typedef struct Index Index;
  typedef struct Schema Schema;
  typedef struct SrcList SrcList;
  typedef struct Subquery Subquery;
  typedef struct Table Table;

  typedef struct SrcItem SrcItem;

  struct SrcItem {
    char *zName;
    char *zAlias;
    Table *pSTab;
    struct {
      u8 jointype;
      unsigned notIndexed : 1;
      unsigned isIndexedBy : 1;
      unsigned isSubquery : 1;
      unsigned isTabFunc : 1;
      unsigned isCorrelated : 1;
      unsigned isMaterialized : 1;
      unsigned viaCoroutine : 1;
      unsigned isRecursive : 1;
      unsigned fromDDL : 1;
      unsigned isCte : 1;
      unsigned notCte : 1;
      unsigned isUsing : 1;
      unsigned isOn : 1;
      unsigned isSynthUsing : 1;
      unsigned isNestedFrom : 1;
      unsigned rowidUsed : 1;
      unsigned fixedSchema : 1;
      unsigned hadSchema : 1;
      unsigned fromExists : 1;
    } fg;
    int iCursor;
    Bitmask colUsed;
    union {
      char *zIndexedBy;
      ExprList *pFuncArg;
      u32 nRow;
    } u1;
    union {
      Index *pIBIndex;
      CteUse *pCteUse;
    } u2;
    union {
      Expr *pOn;
      IdList *pUsing;
    } u3;
    union {
      Schema *pSchema;
      char *zDatabase;
      Subquery *pSubq;
    } u4;
  };

  void sqlite3SrcItemColumnUsed(SrcItem *, int);
  int disableUnusedSubqueryResultColumns(SrcItem * pItem);
  int sameSrcAlias(SrcItem * p0, SrcList * pSrc);

#ifdef __cplusplus
}
#endif
