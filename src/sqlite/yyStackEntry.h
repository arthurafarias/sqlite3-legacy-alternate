
#pragma once

#include "sqlite/YYMINORTYPE.h"
  struct yyStackEntry;

  struct yyStackEntry {
    unsigned short int stateno;
    unsigned short int major;

    YYMINORTYPE minor;
  };


