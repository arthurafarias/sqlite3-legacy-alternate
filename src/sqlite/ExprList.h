
#pragma once

#include "sqlite/ExprList_item.h"
#include "sqlite/u32.h"
struct Table;

struct ExprList;

struct ExprList {
  int nExpr;
  int nAlloc;
  ExprList_item a[1];
};

void sqlite3ExprListSetSortOrder(ExprList *, int, int);
u32 sqlite3ExprListFlags(const ExprList *);
int sqlite3ExprListCompare(const ExprList *, const ExprList *, int);
int sqlite3MatchEName(const struct ExprList_item *, const char *, const char *, const char *, int *);

__attribute__((noinline)) void resolveSetExprSubtypeArg(ExprList *pList);
void heightOfExprList(const ExprList *p, int *pnHeight);
void renameSetENames(ExprList *pEList, int val);
int sqlite3CopySortOrder(ExprList *p1, ExprList *p2);
void sqlite3ProcessReturningSubqueries(ExprList *pEList, Table *pTab);
void adjustOrderByCol(ExprList *pOrderBy, ExprList *pEList);


