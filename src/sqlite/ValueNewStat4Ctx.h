
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "sqlite/Index.h"
#include "sqlite/Parse.h"
#include "sqlite/UnpackedRecord.h"

typedef struct ValueNewStat4Ctx ValueNewStat4Ctx;

struct ValueNewStat4Ctx {
  Parse *pParse;
  Index *pIdx;
  UnpackedRecord **ppRec;
  int iVal;
};

#ifdef __cplusplus
}
#endif
