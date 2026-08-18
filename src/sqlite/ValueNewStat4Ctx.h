
#pragma once

#include "sqlite/Index.h"
#include "sqlite/Parse.h"
#include "sqlite/UnpackedRecord.h"
struct ValueNewStat4Ctx;

struct ValueNewStat4Ctx {
  Parse *pParse;
  Index *pIdx;
  UnpackedRecord **ppRec;
  int iVal;
};


