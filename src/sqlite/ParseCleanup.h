
#pragma once

  struct sqlite3;
  struct ParseCleanup;

  struct ParseCleanup {
    ParseCleanup *pNext;
    void *pPtr;
    void (*xCleanup)(sqlite3 *, void *);
  };


