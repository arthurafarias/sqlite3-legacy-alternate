#define _GNU_SOURCE 1

#include "sqlite/With.h"

#include "sqlite/Cte.h"
#include "sqlite/SrcItem.h"
#include "sqlite/sqlite3.h"
struct Cte *searchWith(With *pWith, SrcItem *pItem, With **ppContext) {
  const char *zName = pItem->zName;
  With *p;

  for (p = pWith; p; p = p->pOuter) {
    int i;
    for (i = 0; i < p->nCte; i++) {
      if (sqlite3StrICmp(zName, p->a[i].zName) == 0) {
        *ppContext = p;
        return &p->a[i];
      }
    }
    if (p->bView)
      break;
  }
  return 0;
}
