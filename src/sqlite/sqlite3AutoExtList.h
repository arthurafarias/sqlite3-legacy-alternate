
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/u32.h"
  typedef struct sqlite3AutoExtList sqlite3AutoExtList;
  struct sqlite3AutoExtList {
    u32 nExt;
    void (**aExt)(void);
  };

  extern sqlite3AutoExtList sqlite3Autoext;

#ifdef __cplusplus
}
#endif
