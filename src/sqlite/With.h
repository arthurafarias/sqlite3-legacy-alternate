
#pragma once

#include "sqlite/Cte.h"
  struct SrcItem;

  struct With;

  struct With {
    int nCte;
    int bView;
    With *pOuter;
    Cte a[1];
  };

  struct Cte *searchWith(With * pWith, SrcItem * pItem, With * *ppContext);


