
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/DbPage.h"
#include "sqlite/InitData.h"
#include "sqlite/StrAccum.h"
#include "sqlite/sqlite3_file.h"
#include "sqlite/sqlite3_hard_heap.h"
#include "sqlite/sqlite3_libversion.h"
#include "sqlite/sqlite3_libversion_number.h"
#include "sqlite/sqlite3_soft_heap.h"
#include "sqlite/sqlite3_sourceid.h"
#include "sqlite/u8.h"
#include "sqlite/yDbMask.h"
#include "sqlite/ynVar.h"
  typedef struct DistinctCtx DistinctCtx;
  struct DistinctCtx {
    u8 isTnct;
    u8 eTnctType;
    int tabTnct;
    int addrTnct;
  };

#ifdef __cplusplus
}
#endif
