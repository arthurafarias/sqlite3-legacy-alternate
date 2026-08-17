
#pragma once
#ifdef __cplusplus
extern C {
#endif
  typedef struct sqlite3 sqlite3;
  typedef struct analysisInfo analysisInfo;

  struct analysisInfo {
    sqlite3 *db;
    const char *zDatabase;
  };

#ifdef __cplusplus
}
#endif
