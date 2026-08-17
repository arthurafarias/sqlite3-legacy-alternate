
#pragma once
#ifdef __cplusplus
extern C {
#endif

#include "sqlite/Token.h"

  typedef struct RenameToken RenameToken;
  struct RenameToken {
    const void *p;
    Token t;
    RenameToken *pNext;
  };

#ifdef __cplusplus
}
#endif
