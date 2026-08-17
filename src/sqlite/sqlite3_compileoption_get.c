#define _GNU_SOURCE 1

#include "sqlite/sqlite3_compileoption_get.h"

#include "sqlite/sqlite3.h"

const char *sqlite3_compileoption_get(int N) {
  int nOpt;
  const char **azCompileOpt;
  azCompileOpt = sqlite3CompileOptions(&nOpt);
  if (N >= 0 && N < nOpt) {
    return azCompileOpt[N];
  }
  return 0;
}
