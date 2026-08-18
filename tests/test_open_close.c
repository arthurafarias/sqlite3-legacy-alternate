#include "test_common.h"

int main(void) {
  sqlite3 *db = NULL;
  int rc;

  /* Opening a path in a non-existent directory without CREATE must fail. */
  rc = sqlite3_open_v2("/nonexistent_dir_xyz/db.sqlite", &db,
                        SQLITE_OPEN_READWRITE, NULL);
  TEST_ASSERT_MSG(rc != SQLITE_OK,
                   "open without CREATE on a missing path should fail");
  sqlite3_close(db);
  db = NULL;

  /* In-memory database via explicit flags. */
  rc = sqlite3_open_v2(":memory:", &db,
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
  TEST_OK(db, rc);
  TEST_ASSERT(db != NULL);
  test_exec_ok(db, "CREATE TABLE t(x)");

  /* sqlite3_close refuses to run while a prepared statement is unfinalized. */
  sqlite3_stmt *stmt = NULL;
  rc = sqlite3_prepare_v2(db, "SELECT * FROM t", -1, &stmt, NULL);
  TEST_OK(db, rc);
  rc = sqlite3_close(db);
  TEST_ASSERT_MSG(rc == SQLITE_BUSY,
                   "close with an open stmt should return SQLITE_BUSY, got %d",
                   rc);

  /* close_v2 defers actual teardown until the last statement is finalized. */
  rc = sqlite3_close_v2(db);
  TEST_ASSERT_MSG(rc == SQLITE_OK,
                   "close_v2 should report OK even with a pending stmt, got %d",
                   rc);
  sqlite3_finalize(stmt);

  /* "" opens a private on-disk temporary database. */
  rc = sqlite3_open("", &db);
  TEST_OK(db, rc);
  test_exec_ok(db, "CREATE TABLE temp_t(x)");
  test_close_ok(db);

  /* Read-only open of a database that doesn't exist should fail. */
  rc = sqlite3_open_v2("./this_file_should_not_exist.db", &db,
                        SQLITE_OPEN_READONLY, NULL);
  TEST_ASSERT(rc != SQLITE_OK);
  sqlite3_close(db);

  TEST_PASS("test_open_close");
  return 0;
}
