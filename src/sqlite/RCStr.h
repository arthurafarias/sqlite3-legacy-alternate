
#pragma once

#include "sqlite/u64.h"
  struct RCStr;

  struct RCStr {
    u64 nRCRef;
  };

  char *sqlite3RCStrRef(char *);
  void sqlite3RCStrUnref(void *);
  char *sqlite3RCStrNew(u64);
  char *sqlite3RCStrResize(char *, u64);


