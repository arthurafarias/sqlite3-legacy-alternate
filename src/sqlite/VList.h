
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/i32.h"
  typedef i32 VList;

  const char *sqlite3VListNumToName(VList *, int);
  int sqlite3VListNameToNum(VList *, const char *, int);

#ifdef __cplusplus
}
#endif
