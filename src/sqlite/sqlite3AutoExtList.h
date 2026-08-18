
#pragma once

#include "sqlite/u32.h"
  typedef struct sqlite3AutoExtList sqlite3AutoExtList;
  struct sqlite3AutoExtList {
    u32 nExt;
    void (**aExt)(void);
  };

  extern sqlite3AutoExtList sqlite3Autoext;


