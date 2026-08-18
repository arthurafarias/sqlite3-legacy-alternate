
#pragma once

  typedef struct sqlite3 sqlite3;
  typedef struct ParseCleanup ParseCleanup;

  struct ParseCleanup {
    ParseCleanup *pNext;
    void *pPtr;
    void (*xCleanup)(sqlite3 *, void *);
  };


