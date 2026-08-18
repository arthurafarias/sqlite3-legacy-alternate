
#pragma once

#include "sqlite/IdList_item.h"
struct ExprList;
struct IdList;

struct IdList {
  int nId;
  IdList_item a[1];
};

int sqlite3IdListIndex(IdList *, const char *);
int checkColumnOverlap(IdList *pIdList, ExprList *pEList);


