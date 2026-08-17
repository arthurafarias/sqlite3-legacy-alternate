
#pragma once
#ifdef __cplusplus
extern C {
#endif

  typedef struct ExprList ExprList;
  typedef struct IdList IdList;

  struct IdList {
    int nId;
    struct IdList_item {
      char *zName;
    } a[];
  };

  int sqlite3IdListIndex(IdList *, const char *);
  int checkColumnOverlap(IdList * pIdList, ExprList * pEList);

#ifdef __cplusplus
}
#endif
