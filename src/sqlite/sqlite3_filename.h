
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/sqlite3_int64.h"
  typedef const char *sqlite3_filename;

  const char *sqlite3_uri_parameter(sqlite3_filename z, const char *zParam);
  int sqlite3_uri_boolean(sqlite3_filename z, const char *zParam, int bDefault);
  sqlite3_int64 sqlite3_uri_int64(sqlite3_filename, const char *, sqlite3_int64);
  const char *sqlite3_uri_key(sqlite3_filename z, int N);
  const char *sqlite3_filename_database(sqlite3_filename);
  const char *sqlite3_filename_journal(sqlite3_filename);
  const char *sqlite3_filename_wal(sqlite3_filename);
  sqlite3_filename sqlite3_create_filename(const char *zDatabase, const char *zJournal, const char *zWal, int nParam,
                                           const char **azParam);
  void sqlite3_free_filename(sqlite3_filename);

#ifdef __cplusplus
}
#endif
