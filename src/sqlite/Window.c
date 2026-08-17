#define _GNU_SOURCE 1
#include "sqlite/Window.h"
#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/FuncDef.h"
#include "sqlite/WindowFunctionNames.h"
#include "sqlite/i16.h"
#include "sqlite/sqlite3_context.h"
#include "sqlite/u32.h"
static const char last_valueName[] = "last_value";

void sqlite3WindowFunctions(void) {
  static FuncDef aWindowFuncs[] = {
      {0,
       (0x00800000 | 1 | 0x00010000 | 0),
       0,
       0,
       row_numberStepFunc,
       row_numberValueFunc,
       row_numberValueFunc,
       noopStepFunc,
       row_numberName,
       {0}},
      {0,
       (0x00800000 | 1 | 0x00010000 | 0),
       0,
       0,
       dense_rankStepFunc,
       dense_rankValueFunc,
       dense_rankValueFunc,
       noopStepFunc,
       dense_rankName,
       {0}},
      {0,
       (0x00800000 | 1 | 0x00010000 | 0),
       0,
       0,
       rankStepFunc,
       rankValueFunc,
       rankValueFunc,
       noopStepFunc,
       rankName,
       {0}},
      {0,
       (0x00800000 | 1 | 0x00010000 | 0),
       0,
       0,
       percent_rankStepFunc,
       percent_rankValueFunc,
       percent_rankValueFunc,
       percent_rankInvFunc,
       percent_rankName,
       {0}},
      {0,
       (0x00800000 | 1 | 0x00010000 | 0),
       0,
       0,
       cume_distStepFunc,
       cume_distValueFunc,
       cume_distValueFunc,
       cume_distInvFunc,
       cume_distName,
       {0}},
      {1,
       (0x00800000 | 1 | 0x00010000 | 0),
       0,
       0,
       ntileStepFunc,
       ntileValueFunc,
       ntileValueFunc,
       ntileInvFunc,
       ntileName,
       {0}},
      {1,
       (0x00800000 | 1 | 0x00010000 | 0),
       0,
       0,
       last_valueStepFunc,
       last_valueFinalizeFunc,
       last_valueValueFunc,
       last_valueInvFunc,
       last_valueName,
       {0}},
      {2,
       (0x00800000 | 1 | 0x00010000 | 0),
       0,
       0,
       nth_valueStepFunc,
       nth_valueFinalizeFunc,
       noopValueFunc,
       noopStepFunc,
       nth_valueName,
       {0}},
      {1,
       (0x00800000 | 1 | 0x00010000 | 0),
       0,
       0,
       first_valueStepFunc,
       first_valueFinalizeFunc,
       noopValueFunc,
       noopStepFunc,
       first_valueName,
       {0}},
      {1,
       (0x00800000 | 1 | 0x00010000 | 0),
       0,
       0,
       noopStepFunc,
       noopValueFunc,
       noopValueFunc,
       noopStepFunc,
       leadName,
       {0}},
      {2,
       (0x00800000 | 1 | 0x00010000 | 0),
       0,
       0,
       noopStepFunc,
       noopValueFunc,
       noopValueFunc,
       noopStepFunc,
       leadName,
       {0}},
      {3,
       (0x00800000 | 1 | 0x00010000 | 0),
       0,
       0,
       noopStepFunc,
       noopValueFunc,
       noopValueFunc,
       noopStepFunc,
       leadName,
       {0}},
      {1,
       (0x00800000 | 1 | 0x00010000 | 0),
       0,
       0,
       noopStepFunc,
       noopValueFunc,
       noopValueFunc,
       noopStepFunc,
       lagName,
       {0}},
      {2,
       (0x00800000 | 1 | 0x00010000 | 0),
       0,
       0,
       noopStepFunc,
       noopValueFunc,
       noopValueFunc,
       noopStepFunc,
       lagName,
       {0}},
      {3,
       (0x00800000 | 1 | 0x00010000 | 0),
       0,
       0,
       noopStepFunc,
       noopValueFunc,
       noopValueFunc,
       noopStepFunc,
       lagName,
       {0}},
  };
  sqlite3InsertBuiltinFuncs(aWindowFuncs, ((int)(sizeof(aWindowFuncs) / sizeof(aWindowFuncs[0]))));
}

void sqlite3WindowUnlinkFromSelect(Window *p) {
  if (p->ppThis) {
    *p->ppThis = p->pNextWin;
    if (p->pNextWin)
      p->pNextWin->ppThis = p->ppThis;
    p->ppThis = 0;
  }
}

int windowArgCount(Window *pWin) {
  const ExprList *pList;

  pList = pWin->pOwner->x.pList;
  return (pList ? pList->nExpr : 0);
}

int windowCacheFrame(Window *pMWin) {
  Window *pWin;
  if (pMWin->regStartRowid)
    return 1;
  for (pWin = pMWin; pWin; pWin = pWin->pNextWin) {
    FuncDef *pFunc = pWin->pWFunc;
    if ((pFunc->zName == nth_valueName) || (pFunc->zName == first_valueName) || (pFunc->zName == leadName) ||
        (pFunc->zName == lagName)) {
      return 1;
    }
  }
  return 0;
}