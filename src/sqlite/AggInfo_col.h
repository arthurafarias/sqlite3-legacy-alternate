#pragma once

struct Table;
struct Expr;

struct AggInfo_col;
struct AggInfo_col {
  Table *pTab;
  Expr *pCExpr;
  int iTable;
  int iColumn;
  int iSorterColumn;
};


