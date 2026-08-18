#pragma once

typedef struct Table Table;
typedef struct Expr Expr;

typedef struct AggInfo_col AggInfo_col;
struct AggInfo_col {
  Table *pTab;
  Expr *pCExpr;
  int iTable;
  int iColumn;
  int iSorterColumn;
};


