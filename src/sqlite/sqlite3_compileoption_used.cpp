#define _GNU_SOURCE 1
#include "sqlite/sqlite3_compileoption_used.h"
#include "sqlite/sqlite3.h"
int sqlite3_compileoption_used(const char *zOptName) {
  int i, n;
  int nOpt;
  const char **azCompileOpt;

  azCompileOpt = sqlite3CompileOptions(&nOpt);

  if (sqlite3_strnicmp(zOptName, "SQLITE_", 7) == 0)
    zOptName += 7;
  n = sqlite3Strlen30(zOptName);

  for (i = 0; i < nOpt; i++) {
    if (sqlite3_strnicmp(zOptName, azCompileOpt[i], n) == 0 &&
        sqlite3IsIdChar((unsigned char)azCompileOpt[i][n]) == 0) {
      return 1;
    }
  }
  return 0;
}
