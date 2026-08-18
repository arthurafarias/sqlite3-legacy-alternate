
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
  typedef struct sqlite3 sqlite3;
  typedef struct ParseCleanup ParseCleanup;

  struct ParseCleanup {
    ParseCleanup *pNext;
    void *pPtr;
    void (*xCleanup)(sqlite3 *, void *);
  };

#ifdef __cplusplus
}
#endif
