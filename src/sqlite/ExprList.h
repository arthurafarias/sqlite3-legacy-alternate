
#pragma once
#ifdef __cplusplus
extern C {
#endif

#include "sqlite/Expr.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
  typedef struct Table Table;

  typedef struct ExprList ExprList;

  struct ExprList {
    int nExpr;
    int nAlloc;
    struct ExprList_item {
      Expr *pExpr;
      char *zEName;
      struct {
        u8 sortFlags;
        unsigned eEName : 2;
        unsigned done : 1;
        unsigned reusable : 1;
        unsigned bSorterRef : 1;
        unsigned bNulls : 1;
        unsigned bUsed : 1;
        unsigned bUsingTerm : 1;
        unsigned bNoExpand : 1;

      } fg;
      union {
        struct {
          u16 iOrderByCol;
          u16 iAlias;
        } x;
        int iConstExprReg;

      } u;
    } a[];
  };

  void sqlite3ExprListSetSortOrder(ExprList *, int, int);
  u32 sqlite3ExprListFlags(const ExprList *);
  int sqlite3ExprListCompare(const ExprList *, const ExprList *, int);
  int sqlite3MatchEName(const struct ExprList_item *, const char *, const char *, const char *, int *);

  __attribute__((noinline)) void resolveSetExprSubtypeArg(ExprList * pList);
  void heightOfExprList(const ExprList *p, int *pnHeight);
  void renameSetENames(ExprList * pEList, int val);
  int sqlite3CopySortOrder(ExprList * p1, ExprList * p2);
  void sqlite3ProcessReturningSubqueries(ExprList * pEList, Table * pTab);
  void adjustOrderByCol(ExprList * pOrderBy, ExprList * pEList);

#ifdef __cplusplus
}
#endif
