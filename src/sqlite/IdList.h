
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/IdList_item.h"
typedef struct ExprList ExprList;
typedef struct IdList IdList;

struct IdList {
  int nId;
  IdList_item a[1];
};

int sqlite3IdListIndex(IdList *, const char *);
int checkColumnOverlap(IdList *pIdList, ExprList *pEList);

#ifdef __cplusplus
}
#endif
