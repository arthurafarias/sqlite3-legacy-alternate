
#pragma once

#include "sqlite/PgHdr1.h"
  struct sqlite3_mutex;

  struct PGroup;
  struct PGroup {
    sqlite3_mutex *mutex;
    unsigned int nMaxPage;
    unsigned int nMinPage;
    unsigned int mxPinned;
    unsigned int nPurgeable;
    PgHdr1 lru;
  };


