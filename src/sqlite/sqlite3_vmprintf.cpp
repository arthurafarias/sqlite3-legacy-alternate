#define _GNU_SOURCE 1
#include "sqlite/sqlite3_vmprintf.h"
#include "sqlite/StrAccum.h"
#include "sqlite/sqlite3.h"
char *sqlite3_vmprintf(const char *zFormat, va_list ap) {
  char *z;
  char zBase[70];
  StrAccum acc;

  if (sqlite3_initialize())
    return 0;

  sqlite3StrAccumInit(&acc, 0, zBase, sizeof(zBase), 1000000000);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  z = sqlite3StrAccumFinish(&acc);
  return z;
}
