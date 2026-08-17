
#pragma once
#ifdef __cplusplus
extern C {
#endif

  typedef struct JsonParse JsonParse;
  typedef struct sqlite3 sqlite3;
  typedef struct JsonCache JsonCache;
  struct JsonCache {
    sqlite3 *db;
    int nUsed;
    JsonParse *a[4];
  };

  void jsonCacheDelete(JsonCache * p);
  void jsonCacheDeleteGeneric(void *p);

#ifdef __cplusplus
}
#endif
