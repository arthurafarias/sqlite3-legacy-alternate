#include "test_common.h"

static long long row_count(sqlite3 *db) {
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM t", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  long long n = sqlite3_column_int64(stmt, 0);
  sqlite3_finalize(stmt);
  return n;
}

int main(void) {
  sqlite3 *db = test_open_memory_db();

  /* A table-level ON CONFLICT clause baked into the column definition. */
  test_exec_ok(db, "CREATE TABLE t(id INTEGER PRIMARY KEY, "
                   "name TEXT UNIQUE ON CONFLICT IGNORE)");
  test_exec_ok(db, "INSERT INTO t(name) VALUES ('a')");
  test_exec_ok(db, "INSERT INTO t(name) VALUES ('a')"); /* silently ignored */
  TEST_ASSERT_EQ_INT(row_count(db), 1);

  /* INSERT OR REPLACE deletes the conflicting row and inserts the new one. */
  test_exec_ok(db, "CREATE TABLE u(id INTEGER PRIMARY KEY, name TEXT UNIQUE)");
  test_exec_ok(db, "INSERT INTO u VALUES (1, 'x')");
  test_exec_ok(db, "INSERT OR REPLACE INTO u VALUES (2, 'x')");
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT id FROM u WHERE name='x'", -1,
                                  &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 2); /* row 1 replaced */
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_DONE);
  sqlite3_finalize(stmt);

  /* INSERT OR IGNORE skips the conflicting row without erroring. */
  int rc = sqlite3_exec(db, "INSERT OR IGNORE INTO u VALUES (3, 'x')", NULL,
                         NULL, NULL);
  TEST_ASSERT(rc == SQLITE_OK);
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM u", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 1);
  sqlite3_finalize(stmt);

  /* INSERT OR ABORT (the default) rolls back only the failed statement,
   * not the whole surrounding transaction. */
  test_exec_ok(db, "BEGIN");
  test_exec_ok(db, "INSERT INTO u VALUES (4, 'y')");
  rc = sqlite3_exec(db, "INSERT OR ABORT INTO u VALUES (5, 'y')", NULL, NULL, NULL);
  TEST_ASSERT(rc == SQLITE_CONSTRAINT);
  TEST_ASSERT(sqlite3_get_autocommit(db) == 0); /* txn still open */
  test_exec_ok(db, "COMMIT");
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM u WHERE name='y'",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 1); /* row 4 survived */
  sqlite3_finalize(stmt);

  /* INSERT OR FAIL stops at the failing row but keeps earlier effects of
   * the same statement (a multi-row VALUES list). */
  test_exec_ok(db, "CREATE TABLE v(id INTEGER PRIMARY KEY, name TEXT UNIQUE)");
  test_exec_ok(db, "INSERT INTO v VALUES (1, 'dup')");
  rc = sqlite3_exec(db,
                     "INSERT OR FAIL INTO v VALUES (2, 'ok'), (3, 'dup'), (4, 'ok2')",
                     NULL, NULL, NULL);
  TEST_ASSERT(rc == SQLITE_CONSTRAINT);
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM v", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 2); /* id=1 and id=2 only */
  sqlite3_finalize(stmt);

  test_close_ok(db);
  TEST_PASS("test_conflict_clauses");
  return 0;
}
