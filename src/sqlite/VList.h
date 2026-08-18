
#pragma once

#include "sqlite/i32.h"
  typedef i32 VList;

  const char *sqlite3VListNumToName(VList *, int);
  int sqlite3VListNameToNum(VList *, const char *, int);


