#include "test_common.h"

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE t(id INTEGER PRIMARY KEY, name TEXT NOT NULL, "
                   "score REAL DEFAULT 0)");
  test_exec_ok(db, "CREATE INDEX idx_name ON t(name)");

  /* PRAGMA table_info exposes column metadata as an ordinary result set. */
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "PRAGMA table_info(t)", -1, &stmt, NULL));
  int seen_notnull_name = 0, ncols = 0;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    ncols++;
    /* columns: cid, name, type, notnull, dflt_value, pk */
    const char *cname = (const char *)sqlite3_column_text(stmt, 1);
    if (strcmp(cname, "name") == 0)
      seen_notnull_name = sqlite3_column_int(stmt, 3);
  }
  TEST_ASSERT_EQ_INT(ncols, 3);
  TEST_ASSERT_MSG(seen_notnull_name, "table_info should report name as NOT NULL");
  sqlite3_finalize(stmt);

  /* user_version / application_id are free 32-bit integers stored in the
   * database header, useful for app-level schema versioning. */
  test_exec_ok(db, "PRAGMA user_version = 42");
  TEST_OK(db, sqlite3_prepare_v2(db, "PRAGMA user_version", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 42);
  sqlite3_finalize(stmt);

  test_exec_ok(db, "PRAGMA application_id = 1234");
  TEST_OK(db, sqlite3_prepare_v2(db, "PRAGMA application_id", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 1234);
  sqlite3_finalize(stmt);

  /* sqlite3_table_column_metadata: programmatic column introspection. */
  int notnull = 0, primarykey = 0, autoinc = 0;
  const char *datatype = NULL, *collseq = NULL;
  TEST_OK(db, sqlite3_table_column_metadata(db, "main", "t", "id", &datatype,
                                             &collseq, &notnull, &primarykey,
                                             &autoinc));
  TEST_ASSERT(primarykey);

  /* integrity_check should report "ok" on a healthy database. */
  TEST_OK(db, sqlite3_prepare_v2(db, "PRAGMA integrity_check", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 0), "ok");
  sqlite3_finalize(stmt);

  /* Readonly introspection. */
  TEST_ASSERT_EQ_INT(sqlite3_db_readonly(db, "main"), 0);
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT * FROM t", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_stmt_readonly(stmt));
  sqlite3_finalize(stmt);
  TEST_OK(db, sqlite3_prepare_v2(db, "INSERT INTO t(name) VALUES ('x')", -1,
                                  &stmt, NULL));
  TEST_ASSERT(!sqlite3_stmt_readonly(stmt));
  sqlite3_finalize(stmt);

  TEST_ASSERT_EQ_STR(sqlite3_db_filename(db, "main"), "");

  test_close_ok(db);
  TEST_PASS("test_pragma_and_meta");
  return 0;
}
