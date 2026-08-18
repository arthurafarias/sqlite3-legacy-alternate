#include "test_common.h"

static long long count_children(sqlite3 *db) {
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM child", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  long long n = sqlite3_column_int64(stmt, 0);
  sqlite3_finalize(stmt);
  return n;
}

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "PRAGMA foreign_keys = ON");

  /* ON DELETE CASCADE removes dependent rows automatically. */
  test_exec_ok(db, "CREATE TABLE parent(id INTEGER PRIMARY KEY)");
  test_exec_ok(db, "CREATE TABLE child(id INTEGER PRIMARY KEY, "
                   "parent_id INTEGER REFERENCES parent(id) ON DELETE CASCADE)");
  test_exec_ok(db, "INSERT INTO parent VALUES (1), (2)");
  test_exec_ok(db, "INSERT INTO child VALUES (1, 1), (2, 1), (3, 2)");
  test_exec_ok(db, "DELETE FROM parent WHERE id = 1");
  TEST_ASSERT_EQ_INT(count_children(db), 1); /* only child id=3 (parent 2) remains */

  /* ON DELETE SET NULL detaches instead of deleting. */
  test_exec_ok(db, "CREATE TABLE parent2(id INTEGER PRIMARY KEY)");
  test_exec_ok(db, "CREATE TABLE child2(id INTEGER PRIMARY KEY, "
                   "parent_id INTEGER REFERENCES parent2(id) ON DELETE SET NULL)");
  test_exec_ok(db, "INSERT INTO parent2 VALUES (1)");
  test_exec_ok(db, "INSERT INTO child2 VALUES (1, 1)");
  test_exec_ok(db, "DELETE FROM parent2 WHERE id = 1");
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT parent_id FROM child2 WHERE id=1",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT(sqlite3_column_type(stmt, 0) == SQLITE_NULL);
  sqlite3_finalize(stmt);

  /* ON UPDATE CASCADE propagates a parent key change to dependents. */
  test_exec_ok(db, "CREATE TABLE parent3(id INTEGER PRIMARY KEY)");
  test_exec_ok(db, "CREATE TABLE child3(id INTEGER PRIMARY KEY, "
                   "parent_id INTEGER REFERENCES parent3(id) ON UPDATE CASCADE)");
  test_exec_ok(db, "INSERT INTO parent3 VALUES (1)");
  test_exec_ok(db, "INSERT INTO child3 VALUES (1, 1)");
  test_exec_ok(db, "UPDATE parent3 SET id = 99 WHERE id = 1");
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT parent_id FROM child3 WHERE id=1",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 99);
  sqlite3_finalize(stmt);

  /* DEFERRABLE INITIALLY DEFERRED: the FK is only checked at COMMIT, so a
   * temporarily-dangling reference inside a transaction is allowed. */
  test_exec_ok(db, "CREATE TABLE parent4(id INTEGER PRIMARY KEY)");
  test_exec_ok(db, "CREATE TABLE child4(id INTEGER PRIMARY KEY, "
                   "parent_id INTEGER REFERENCES parent4(id) "
                   "DEFERRABLE INITIALLY DEFERRED)");
  test_exec_ok(db, "BEGIN");
  test_exec_ok(db, "INSERT INTO child4 VALUES (1, 42)"); /* parent 42 doesn't exist yet */
  test_exec_ok(db, "INSERT INTO parent4 VALUES (42)");   /* fixed before commit */
  test_exec_ok(db, "COMMIT");
  TEST_ASSERT_EQ_INT(count_children(db), 1); /* unrelated to child4; sanity that db is alive */

  /* If left dangling at COMMIT time, it fails there instead of at INSERT. */
  test_exec_ok(db, "BEGIN");
  test_exec_ok(db, "INSERT INTO child4 VALUES (2, 12345)"); /* never satisfied */
  int rc = sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
  TEST_ASSERT_MSG(rc == SQLITE_CONSTRAINT,
                   "COMMIT should fail on an unresolved deferred FK, got %d", rc);
  test_exec_ok(db, "ROLLBACK");

  test_close_ok(db);
  TEST_PASS("test_foreign_key_actions");
  return 0;
}
