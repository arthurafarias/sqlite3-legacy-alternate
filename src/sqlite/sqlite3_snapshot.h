
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
  typedef struct sqlite3_snapshot sqlite3_snapshot;
  struct sqlite3_snapshot {
    unsigned char hidden[48];
  };
  void sqlite3_snapshot_free(sqlite3_snapshot *);
  int sqlite3_snapshot_cmp(sqlite3_snapshot * p1, sqlite3_snapshot * p2);
#ifdef __cplusplus
}
#endif
