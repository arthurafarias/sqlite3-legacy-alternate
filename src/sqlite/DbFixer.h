
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/u8.h"
#include "sqlite/Walker.h"
  typedef struct Expr Expr;
  typedef struct Select Select;
  typedef struct SrcList SrcList;
  typedef struct TriggerStep TriggerStep;

  typedef struct DbFixer DbFixer;
  typedef struct Parse Parse;
  typedef struct Walker Walker;
  typedef struct Schema Schema;
  typedef struct Token Token;

  struct DbFixer {
    Parse *pParse;
    Walker w;
    Schema *pSchema;
    u8 bTemp;
    const char *zDb;
    const char *zType;
    const Token *pName;
  };

  void sqlite3FixInit(DbFixer *, Parse *, int, const char *, const Token *);
  int sqlite3FixSrcList(DbFixer *, SrcList *);
  int sqlite3FixSelect(DbFixer *, Select *);
  int sqlite3FixExpr(DbFixer *, Expr *);
  int sqlite3FixTriggerStep(DbFixer *, TriggerStep *);

#ifdef __cplusplus
}
#endif
