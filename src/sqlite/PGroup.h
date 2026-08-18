
#pragma once

#include "sqlite/PgHdr1.h"
  typedef struct sqlite3_mutex sqlite3_mutex;

  typedef struct PGroup PGroup;
  struct PGroup {
    sqlite3_mutex *mutex;
    unsigned int nMaxPage;
    unsigned int nMinPage;
    unsigned int mxPinned;
    unsigned int nPurgeable;
    PgHdr1 lru;
  };


