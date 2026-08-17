#define _GNU_SOURCE 1

#include "sqlite/BenignMallocHooks.h"
BenignMallocHooks sqlite3Hooks = {0, 0};

void sqlite3BenignMallocHooks(void (*xBenignBegin)(void), void (*xBenignEnd)(void)) {
  ;
  sqlite3Hooks.xBenignBegin = xBenignBegin;
  sqlite3Hooks.xBenignEnd = xBenignEnd;
}
