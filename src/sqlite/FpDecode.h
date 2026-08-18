
#pragma once

#include "sqlite/DbPage.h"
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
#include "sqlite/u64.h"
  struct FpDecode;

  struct FpDecode {
    int n;
    int iDP;
    char *z;
    char zBuf[20 + 1];
    char sign;
    char isSpecial;
  };

  void sqlite3FpDecode(FpDecode *, double, int, int);

  int countLeadingZeros(u64 m);


