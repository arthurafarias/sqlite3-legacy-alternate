
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/Cte.h"
  typedef struct SrcItem SrcItem;

  typedef struct With With;

  struct With {
    int nCte;
    int bView;
    With *pOuter;
    Cte a[];
  };

  struct Cte *searchWith(With * pWith, SrcItem * pItem, With * *ppContext);

#ifdef __cplusplus
}
#endif
