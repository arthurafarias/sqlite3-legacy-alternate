
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/YYMINORTYPE.h"
  typedef struct yyStackEntry yyStackEntry;

  struct yyStackEntry {
    unsigned short int stateno;
    unsigned short int major;

    YYMINORTYPE minor;
  };

#ifdef __cplusplus
}
#endif
