#define _GNU_SOURCE 1

#include <string.h>

#include "sqlite/DbFixer.h"

#include "sqlite/Db.h"
#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/Parse.h"
#include "sqlite/Schema.h"
#include "sqlite/Select.h"
#include "sqlite/SrcList.h"
#include "sqlite/Token.h"
#include "sqlite/TriggerStep.h"
#include "sqlite/Upsert.h"
#include "sqlite/Walker.h"
#include "sqlite/sqlite3.h"
#include "sqlite/u16.h"
#include "sqlite/u8.h"
void sqlite3FixInit(DbFixer *pFix, Parse *pParse, int iDb, const char *zType, const Token *pName) {
  sqlite3 *db = pParse->db;

  pFix->pParse = pParse;
  pFix->zDb = db->aDb[iDb].zDbSName;
  pFix->pSchema = db->aDb[iDb].pSchema;
  pFix->zType = zType;
  pFix->pName = pName;
  pFix->bTemp = (iDb == 1);
  pFix->w.pParse = pParse;
  pFix->w.xExprCallback = fixExprCb;
  pFix->w.xSelectCallback = fixSelectCb;
  pFix->w.xSelectCallback2 = sqlite3WalkWinDefnDummyCallback;
  pFix->w.walkerDepth = 0;
  pFix->w.eCode = 0;
  pFix->w.u.pFix = pFix;
}

int sqlite3FixSrcList(DbFixer *pFix, SrcList *pList) {
  int res = 0;
  if (pList) {
    Select s;
    memset(&s, 0, sizeof(s));
    s.pSrc = pList;
    res = sqlite3WalkSelect(&pFix->w, &s);
  }
  return res;
}

int sqlite3FixSelect(DbFixer *pFix, Select *pSelect) { return sqlite3WalkSelect(&pFix->w, pSelect); }

int sqlite3FixExpr(DbFixer *pFix, Expr *pExpr) { return sqlite3WalkExpr(&pFix->w, pExpr); }

int sqlite3FixTriggerStep(DbFixer *pFix, TriggerStep *pStep) {
  while (pStep) {
    if (sqlite3WalkSelect(&pFix->w, pStep->pSelect) || sqlite3WalkExpr(&pFix->w, pStep->pWhere) || sqlite3WalkExprList(&pFix->w, pStep->pExprList) || sqlite3FixSrcList(pFix, pStep->pSrc)) {
      return 1;
    }

    {
      Upsert *pUp;
      for (pUp = pStep->pUpsert; pUp; pUp = pUp->pNextUpsert) {
        if (sqlite3WalkExprList(&pFix->w, pUp->pUpsertTarget) || sqlite3WalkExpr(&pFix->w, pUp->pUpsertTargetWhere) || sqlite3WalkExprList(&pFix->w, pUp->pUpsertSet) || sqlite3WalkExpr(&pFix->w, pUp->pUpsertWhere)) {
          return 1;
        }
      }
    }

    pStep = pStep->pNext;
  }

  return 0;
}
