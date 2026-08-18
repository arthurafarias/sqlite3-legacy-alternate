
#pragma once

#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_value.h"
#include "sqlite/u16.h"
  struct BtreePayload;

  struct BtreePayload {
    const void *pKey;
    sqlite3_int64 nKey;
    const void *pData;
    sqlite3_value *aMem;
    u16 nMem;
    int nData;
    int nZero;
  };


