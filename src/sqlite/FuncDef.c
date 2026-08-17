#define _GNU_SOURCE 1

#include "sqlite/FuncDef.h"

#include "sqlite/FuncDefHash.h"
#include "sqlite/i16.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_context.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
const FuncDef statInitFuncdef = {4, 1, 0, 0, statInit, 0, 0, 0, "stat_init", {0}};

const FuncDef statPushFuncdef = {2 + 0, 1, 0, 0, statPush, 0, 0, 0, "stat_push", {0}};

const FuncDef statGetFuncdef = {1 + 0, 1, 0, 0, statGet, 0, 0, 0, "stat_get", {0}};

int matchQuality(FuncDef *p, int nArg, u8 enc) {
  int match;

  if (p->nArg != nArg) {
    if (nArg == (-2))
      return p->xSFunc == 0 ? 0 : 6;
    if (p->nArg >= 0)
      return 0;

    if (p->nArg < (-2) && nArg < (-2 - p->nArg))
      return 0;
  }

  if (p->nArg == nArg) {
    match = 4;
  } else {
    match = 1;
  }

  if (enc == (p->funcFlags & 0x0003)) {
    match += 2;
  } else if ((enc & p->funcFlags & 2) != 0) {
    match += 1;
  }

  return match;
}

void sqlite3InsertBuiltinFuncs(FuncDef *aDef, int nDef) {
  int i;
  for (i = 0; i < nDef; i++) {
    FuncDef *pOther;
    const char *zName = aDef[i].zName;
    int nName = sqlite3Strlen30(zName);
    int h = (((zName[0]) + (nName)) % 23);


    pOther = sqlite3FunctionSearch(h, zName);
    if (pOther) {


      aDef[i].pNext = pOther->pNext;
      pOther->pNext = &aDef[i];
    } else {
      aDef[i].pNext = 0;
      aDef[i].u.pHash = sqlite3BuiltinFunctions.a[h];
      sqlite3BuiltinFunctions.a[h] = &aDef[i];
    }
  }
}
