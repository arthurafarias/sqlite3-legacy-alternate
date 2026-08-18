
#pragma once

#include "sqlite/Token.h"
  typedef struct RenameToken RenameToken;
  struct RenameToken {
    const void *p;
    Token t;
    RenameToken *pNext;
  };


