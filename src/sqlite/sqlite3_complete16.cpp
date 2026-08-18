#define _GNU_SOURCE 1
#include "sqlite/sqlite3_complete16.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_complete.h"
#include "sqlite/sqlite3_destructor_type.h"
#include "sqlite/SqliteTextEncoding.h"
int sqlite3_complete16(const void *zSql) {
  sqlite3_value *pVal;
  char const *zSql8;
  int rc;

  rc = sqlite3_initialize();
  if (rc)
    return rc;

  pVal = sqlite3ValueNew(0);
  sqlite3ValueSetStr(pVal, -1, zSql, 2, ((sqlite3_destructor_type)0));
  zSql8 = (const char*)(sqlite3ValueText(pVal, SQLITE_UTF8));
  if (zSql8) {
    rc = sqlite3_complete(zSql8);
  } else {
    rc = 7;
  }
  sqlite3ValueFree(pVal);
  return rc & 0xff;
}
