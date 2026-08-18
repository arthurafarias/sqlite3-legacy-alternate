
#pragma once

#include "sqlite/u8.h"
#include "sqlite/Walker.h"
  struct TriggerStep;

  struct Schema;
  struct Token;

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


