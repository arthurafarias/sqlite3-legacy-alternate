#define _GNU_SOURCE 1

#include <string.h>

#include "sqlite/VList.h"
const char *sqlite3VListNumToName(VList *pIn, int iVal) {
  int i, mx;
  if (pIn == 0)
    return 0;
  mx = pIn[1];
  i = 2;
  do {
    if (pIn[i] == iVal)
      return (char *)&pIn[i + 2];
    i += pIn[i + 1];
  } while (i < mx);
  return 0;
}

int sqlite3VListNameToNum(VList *pIn, const char *zName, int nName) {
  int i, mx;
  if (pIn == 0)
    return 0;
  mx = pIn[1];
  i = 2;
  do {
    const char *z = (const char *)&pIn[i + 2];
    if (strncmp(z, zName, nName) == 0 && z[nName] == 0)
      return pIn[i];
    i += pIn[i + 1];
  } while (i < mx);
  return 0;
}
