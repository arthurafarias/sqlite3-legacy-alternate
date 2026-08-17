#define _GNU_SOURCE 1
#include "sqlite/sqlite3_vsnprintf.h"
#include "sqlite/StrAccum.h"
char *sqlite3_vsnprintf(int n, char *zBuf, const char *zFormat, va_list ap) {
  StrAccum acc;
  if (n <= 0)
    return zBuf;

  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);
  sqlite3_str_vappendf(&acc, zFormat, ap);
  zBuf[acc.nChar] = 0;
  return zBuf;
}
