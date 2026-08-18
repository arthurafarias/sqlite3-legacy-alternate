
#pragma once

#include "sqlite/DbPage.h"
#include "sqlite/StrAccum.h"
#include "sqlite/sqlite3_file.h"
#include "sqlite/sqlite3_hard_heap.h"
#include "sqlite/sqlite3_libversion.h"
#include "sqlite/sqlite3_libversion_number.h"
#include "sqlite/sqlite3_soft_heap.h"
#include "sqlite/sqlite3_sourceid.h"
#include "sqlite/yDbMask.h"
#include "sqlite/ynVar.h"
  typedef struct DbPath DbPath;
  struct DbPath {
    int rc;
    int nSymlink;
    char *zOut;
    int nOut;
    int nUsed;
  };

  void appendAllPathElements(DbPath *, const char *);
  void appendOnePathElement(DbPath * pPath, const char *zName, int nName);


