#define _GNU_SOURCE 1

#include "sqlite/sqlite3_snprintf.h"

#include <stdarg.h>

#include "sqlite/StrAccum.h"

char *sqlite3_snprintf(int n, char *zBuf, const char *zFormat, ...) {
  StrAccum acc;
  va_list ap;
  if (n <= 0)
    return zBuf;

  sqlite3StrAccumInit(&acc, 0, zBuf, n, 0);

  va_start(

      ap, zFormat

  )

      ;
  sqlite3_str_vappendf(&acc, zFormat, ap);

  va_end(

      ap

  )

      ;
  zBuf[acc.nChar] = 0;
  return zBuf;
}
