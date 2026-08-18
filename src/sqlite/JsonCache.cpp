#define _GNU_SOURCE 1
#include "sqlite/JsonCache.h"
#include "sqlite/JsonParse.h"
#include "sqlite/sqlite3.h"
void jsonCacheDelete(JsonCache *p) {
  int i;
  for (i = 0; i < p->nUsed; i++) {
    jsonParseFree(p->a[i]);
  }
  sqlite3DbFree(p->db, p);
}

void jsonCacheDeleteGeneric(void *p) {
  jsonCacheDelete((JsonCache *)p);
}
