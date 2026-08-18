#include "test_common.h"

int main(void) {
  sqlite3 *db = test_open_memory_db();

  /* WITHOUT ROWID: the primary key IS the clustered index key, and there's
   * no separate hidden rowid to fall back on. */
  test_exec_ok(db, "CREATE TABLE kv(k TEXT PRIMARY KEY, v TEXT) WITHOUT ROWID");
  test_exec_ok(db, "INSERT INTO kv VALUES ('a', '1'), ('b', '2'), ('c', '3')");

  int rc = sqlite3_exec(db, "SELECT rowid FROM kv", NULL, NULL, NULL);
  TEST_ASSERT_MSG(rc != SQLITE_OK, "WITHOUT ROWID tables should have no rowid column");

  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT v FROM kv WHERE k='b'", -1, &stmt,
                                  NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 0), "2");
  sqlite3_finalize(stmt);

  /* STRICT tables enforce declared column types at insert time instead of
   * SQLite's usual dynamic typing. */
  test_exec_ok(db, "CREATE TABLE typed(id INTEGER PRIMARY KEY, n INTEGER, "
                   "s TEXT) STRICT");
  test_exec_ok(db, "INSERT INTO typed(n, s) VALUES (42, 'ok')");

  rc = sqlite3_exec(db, "INSERT INTO typed(n, s) VALUES ('not a number', 'x')",
                     NULL, NULL, NULL);
  TEST_ASSERT_MSG(rc != SQLITE_OK,
                   "STRICT table should reject a non-numeric value for an INTEGER column");

  /* A table can be both STRICT and WITHOUT ROWID at once. */
  test_exec_ok(db, "CREATE TABLE both_flags(id INTEGER PRIMARY KEY, v REAL) "
                   "STRICT, WITHOUT ROWID");
  test_exec_ok(db, "INSERT INTO both_flags VALUES (1, 3.5)");
  rc = sqlite3_exec(db, "INSERT INTO both_flags VALUES (2, 'nope')", NULL, NULL, NULL);
  TEST_ASSERT(rc != SQLITE_OK);

  /* GENERATED ALWAYS AS ... STORED persists the computed value on disk,
   * unlike the VIRTUAL generated columns already covered by the JSON test. */
  test_exec_ok(db, "CREATE TABLE rect(w REAL, h REAL, "
                   "area REAL GENERATED ALWAYS AS (w * h) STORED)");
  test_exec_ok(db, "INSERT INTO rect(w, h) VALUES (3, 4)");
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT area FROM rect", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT(sqlite3_column_double(stmt, 0) == 12.0);
  sqlite3_finalize(stmt);
  rc = sqlite3_exec(db, "UPDATE rect SET area = 99", NULL, NULL, NULL);
  TEST_ASSERT_MSG(rc != SQLITE_OK, "a STORED generated column should reject direct writes");

  test_close_ok(db);
  TEST_PASS("test_without_rowid_strict");
  return 0;
}
