#include "test_common.h"

static long long count_rows(sqlite3 *db) {
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM t", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  long long n = sqlite3_column_int64(stmt, 0);
  sqlite3_finalize(stmt);
  return n;
}

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE t(x)");

  TEST_ASSERT_MSG(sqlite3_get_autocommit(db) != 0, "fresh connection is autocommit");

  test_exec_ok(db, "BEGIN");
  TEST_ASSERT(sqlite3_get_autocommit(db) == 0);
  test_exec_ok(db, "INSERT INTO t VALUES (1)");
  test_exec_ok(db, "COMMIT");
  TEST_ASSERT(sqlite3_get_autocommit(db) != 0);
  TEST_ASSERT_EQ_INT(count_rows(db), 1);

  test_exec_ok(db, "BEGIN");
  test_exec_ok(db, "INSERT INTO t VALUES (2)");
  test_exec_ok(db, "ROLLBACK");
  TEST_ASSERT_EQ_INT(count_rows(db), 1);

  /* Nested savepoints: an inner rollback must not undo the outer insert. */
  test_exec_ok(db, "BEGIN");
  test_exec_ok(db, "INSERT INTO t VALUES (3)");
  test_exec_ok(db, "SAVEPOINT sp1");
  test_exec_ok(db, "INSERT INTO t VALUES (4)");
  test_exec_ok(db, "SAVEPOINT sp2");
  test_exec_ok(db, "INSERT INTO t VALUES (5)");
  test_exec_ok(db, "ROLLBACK TO sp2");
  test_exec_ok(db, "RELEASE sp2");
  TEST_ASSERT_EQ_INT(count_rows(db), 3); /* rows 1, 3, 4 */
  test_exec_ok(db, "ROLLBACK TO sp1");
  TEST_ASSERT_EQ_INT(count_rows(db), 2); /* rows 1, 3 */
  test_exec_ok(db, "RELEASE sp1");
  test_exec_ok(db, "COMMIT");
  TEST_ASSERT_EQ_INT(count_rows(db), 2);

  /* A constraint failure inside an explicit transaction should not
   * auto-rollback the whole transaction (unlike a raised exception in
   * some other databases) -- SQLite requires the app to decide. */
  test_exec_ok(db, "CREATE TABLE u(x UNIQUE)");
  test_exec_ok(db, "INSERT INTO u VALUES (1)");
  test_exec_ok(db, "BEGIN");
  test_exec_ok(db, "INSERT INTO u VALUES (2)");
  char *errmsg = NULL;
  int rc = sqlite3_exec(db, "INSERT INTO u VALUES (1)", NULL, NULL, &errmsg);
  TEST_ASSERT(rc == SQLITE_CONSTRAINT);
  sqlite3_free(errmsg);
  TEST_ASSERT_MSG(sqlite3_get_autocommit(db) == 0,
                   "transaction should still be open after a constraint error");
  test_exec_ok(db, "COMMIT");

  test_close_ok(db);
  TEST_PASS("test_transactions_savepoints");
  return 0;
}
