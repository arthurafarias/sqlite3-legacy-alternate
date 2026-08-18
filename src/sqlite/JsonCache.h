
#pragma once

  struct JsonParse;
  struct sqlite3;
  struct JsonCache;
  struct JsonCache {
    sqlite3 *db;
    int nUsed;
    JsonParse *a[4];
  };

  void jsonCacheDelete(JsonCache * p);
  void jsonCacheDeleteGeneric(void *p);


