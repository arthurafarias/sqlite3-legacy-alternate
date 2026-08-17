#define _GNU_SOURCE 1

#include <string.h>

#include "sqlite/CollSeq.h"
int binCollFunc(void *NotUsed, int nKey1, const void *pKey1, int nKey2, const void *pKey2) {
  int rc, n;
  (void)(NotUsed);
  n = nKey1 < nKey2 ? nKey1 : nKey2;

  rc = memcmp(pKey1, pKey2, n);
  if (rc == 0) {
    rc = nKey1 - nKey2;
  }
  return rc;
}


int sqlite3IsBinary(const CollSeq *p) { return p == 0 || p->xCmp == binCollFunc; }