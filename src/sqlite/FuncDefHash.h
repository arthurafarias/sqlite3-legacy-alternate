
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/AggInfo.h"
#include "sqlite/DbPage.h"
#include "sqlite/FuncDef.h"
#include "sqlite/InitData.h"
#include "sqlite/StrAccum.h"
#include "sqlite/sqlite3_file.h"
#include "sqlite/sqlite3_hard_heap.h"
#include "sqlite/sqlite3_libversion.h"
#include "sqlite/sqlite3_libversion_number.h"
#include "sqlite/sqlite3_soft_heap.h"
#include "sqlite/sqlite3_sourceid.h"
#include "sqlite/yDbMask.h"
#include "sqlite/ynVar.h"
  typedef struct FuncDefHash FuncDefHash;

  struct FuncDefHash {
    FuncDef *a[23];
  };

  extern FuncDefHash sqlite3BuiltinFunctions;

#ifdef __cplusplus
}
#endif
