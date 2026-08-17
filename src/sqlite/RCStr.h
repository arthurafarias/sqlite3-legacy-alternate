
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/u64.h"
  typedef struct RCStr RCStr;

  struct RCStr {
    u64 nRCRef;
  };

  char *sqlite3RCStrRef(char *);
  void sqlite3RCStrUnref(void *);
  char *sqlite3RCStrNew(u64);
  char *sqlite3RCStrResize(char *, u64);

#ifdef __cplusplus
}
#endif
