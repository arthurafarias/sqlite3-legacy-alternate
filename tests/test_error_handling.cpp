#include "test_common.h"

int main(void) {
  sqlite3 *db = test_open_memory_db();
  sqlite3_extended_result_codes(db, 1);

  test_exec_ok(db, "CREATE TABLE t(id INTEGER PRIMARY KEY, "
                   "name TEXT NOT NULL UNIQUE, "
                   "age INTEGER CHECK(age >= 0), "
                   "parent INTEGER REFERENCES t(id))");
  test_exec_ok(db, "PRAGMA foreign_keys = ON");
  test_exec_ok(db, "INSERT INTO t(name, age) VALUES ('alice', 30)");

  /* NOT NULL violation */
  int rc = sqlite3_exec(db, "INSERT INTO t(name, age) VALUES (NULL, 1)", NULL,
                         NULL, NULL);
  TEST_ASSERT_MSG((rc & 0xff) == SQLITE_CONSTRAINT, "expected CONSTRAINT, got %d", rc);
  TEST_ASSERT_MSG(rc == SQLITE_CONSTRAINT_NOTNULL,
                   "expected extended code CONSTRAINT_NOTNULL, got %d", rc);

  /* UNIQUE violation */
  rc = sqlite3_exec(db, "INSERT INTO t(name, age) VALUES ('alice', 31)", NULL,
                     NULL, NULL);
  TEST_ASSERT(rc == SQLITE_CONSTRAINT_UNIQUE);

  /* CHECK violation */
  rc = sqlite3_exec(db, "INSERT INTO t(name, age) VALUES ('bob', -1)", NULL,
                     NULL, NULL);
  TEST_ASSERT(rc == SQLITE_CONSTRAINT_CHECK);

  /* FOREIGN KEY violation */
  rc = sqlite3_exec(db, "INSERT INTO t(name, age, parent) VALUES ('eve', 1, 999)",
                     NULL, NULL, NULL);
  TEST_ASSERT(rc == SQLITE_CONSTRAINT_FOREIGNKEY);

  /* sqlite3_errcode / errmsg / errstr agree after the last failure. With
   * extended result codes enabled, sqlite3_errcode() itself returns the
   * extended code, same as sqlite3_extended_errcode(). */
  TEST_ASSERT(sqlite3_errcode(db) == SQLITE_CONSTRAINT_FOREIGNKEY);
  TEST_ASSERT(sqlite3_extended_errcode(db) == SQLITE_CONSTRAINT_FOREIGNKEY);
  TEST_ASSERT(sqlite3_errmsg(db) != NULL);
  TEST_ASSERT_EQ_STR(sqlite3_errstr(SQLITE_BUSY), "database is locked");
  TEST_ASSERT_EQ_STR(sqlite3_errstr(SQLITE_OK), "not an error");

  /* Syntax error path via sqlite3_prepare_v2. */
  sqlite3_stmt *stmt = NULL;
  rc = sqlite3_prepare_v2(db, "NOT VALID SQL", -1, &stmt, NULL);
  TEST_ASSERT(rc == SQLITE_ERROR);
  TEST_ASSERT(stmt == NULL);

  /* A statement with no matching rows steps straight to SQLITE_DONE, and
   * stepping it again without a reset keeps returning SQLITE_DONE. */
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT 1 WHERE 0", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_DONE);
  int rc_again = sqlite3_step(stmt);
  TEST_ASSERT_MSG(rc_again == SQLITE_DONE || rc_again == SQLITE_MISUSE,
                   "stepping past DONE should be DONE again or flagged MISUSE, got %d",
                   rc_again);
  sqlite3_finalize(stmt);

  test_close_ok(db);
  TEST_PASS("test_error_handling");
  return 0;
}
