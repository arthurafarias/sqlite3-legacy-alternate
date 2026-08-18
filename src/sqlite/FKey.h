
#pragma once

#include "sqlite/Trigger.h"
#include "sqlite/sColMap.h"
#include "sqlite/u8.h"
typedef struct Table Table;
typedef struct FKey FKey;

struct FKey {
  Table *pFrom;
  FKey *pNextFrom;
  char *zTo;
  FKey *pNextTo;
  FKey *pPrevTo;
  int nCol;

  u8 isDeferred;
  u8 aAction[2];
  Trigger *apTrigger[2];
  sColMap aCol[1];
};


