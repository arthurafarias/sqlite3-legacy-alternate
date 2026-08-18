
#pragma once

#include "sqlite/Token.h"
  struct RenameToken;
  struct RenameToken {
    const void *p;
    Token t;
    RenameToken *pNext;
  };


