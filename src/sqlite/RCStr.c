#define _GNU_SOURCE 1

#include "sqlite/RCStr.h"

#include "sqlite/sqlite3.h"
#include "sqlite/u64.h"
char *sqlite3RCStrRef(char *z) {
  RCStr *p = (RCStr *)z;

  p--;
  p->nRCRef++;
  return z;
}

void sqlite3RCStrUnref(void *z) {
  RCStr *p = (RCStr *)z;

  p--;

  if (p->nRCRef >= 2) {
    p->nRCRef--;
  } else {
    sqlite3_free(p);
  }
}

char *sqlite3RCStrNew(u64 N) {
  RCStr *p = sqlite3_malloc64(N + sizeof(*p) + 1);
  if (p == 0)
    return 0;
  p->nRCRef = 1;
  return (char *)&p[1];
}

char *sqlite3RCStrResize(char *z, u64 N) {
  RCStr *p = (RCStr *)z;
  RCStr *pNew;

  p--;

  pNew = sqlite3_realloc64(p, N + sizeof(RCStr) + 1);
  if (pNew == 0) {
    sqlite3_free(p);
    return 0;
  } else {
    return (char *)&pNew[1];
  }
}
