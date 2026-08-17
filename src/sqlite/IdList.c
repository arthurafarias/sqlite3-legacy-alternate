#define _GNU_SOURCE 1
#include "sqlite/IdList.h"
#include "sqlite/ExprList.h"
#include "sqlite/sqlite3.h"
int sqlite3IdListIndex(IdList *pList, const char *zName) {
  int i;

  for (i = 0; i < pList->nId; i++) {
    if (sqlite3StrICmp(pList->a[i].zName, zName) == 0)
      return i;
  }
  return -1;
}

int checkColumnOverlap(IdList *pIdList, ExprList *pEList) {
  int e;
  if (pIdList == 0 || (pEList == 0))
    return 1;
  for (e = 0; e < pEList->nExpr; e++) {
    if (sqlite3IdListIndex(pIdList, pEList->a[e].zEName) >= 0)
      return 1;
  }
  return 0;
}
