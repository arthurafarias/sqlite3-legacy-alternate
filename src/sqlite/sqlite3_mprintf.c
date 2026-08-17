#define _GNU_SOURCE 1

#include "sqlite/sqlite3_mprintf.h"

#include <stdarg.h>

#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_vmprintf.h"

char *sqlite3_mprintf(const char *zFormat, ...) {
  va_list ap;
  char *z;

  if (sqlite3_initialize())
    return 0;

  va_start(

      ap, zFormat

  )

      ;
  z = sqlite3_vmprintf(zFormat, ap);

  va_end(

      ap

  )

      ;
  return z;
}
