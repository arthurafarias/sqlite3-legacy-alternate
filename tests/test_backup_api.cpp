#include "test_common.h"

int main(void) {
  sqlite3 *src = test_open_memory_db();
  /* padding makes each row large enough that 5000 of them span well over
   * 10 database pages, so the chunked backup below actually needs more
   * than one sqlite3_backup_step() call to finish. */
  test_exec_ok(src, "CREATE TABLE t(x INTEGER, padding TEXT)");
  test_exec_ok(src,
               "WITH RECURSIVE seq(v) AS "
               "(SELECT 1 UNION ALL SELECT v+1 FROM seq WHERE v < 5000) "
               "INSERT INTO t SELECT v, hex(randomblob(200)) FROM seq");

  sqlite3 *dst = test_open_memory_db();

  sqlite3_backup *backup = sqlite3_backup_init(dst, "main", src, "main");
  TEST_ASSERT_MSG(backup != NULL, "backup_init failed: %s", sqlite3_errmsg(dst));

  int rc;
  int steps = 0;
  do {
    rc = sqlite3_backup_step(backup, 10); /* copy in small chunks */
    steps++;
    TEST_ASSERT(sqlite3_backup_pagecount(backup) >= sqlite3_backup_remaining(backup));
  } while (rc == SQLITE_OK || rc == SQLITE_BUSY || rc == SQLITE_LOCKED);
  TEST_ASSERT_MSG(rc == SQLITE_DONE, "backup_step should finish with DONE, got %d", rc);
  TEST_ASSERT_MSG(steps > 1, "expected multiple backup steps at pagesize 10, got %d", steps);
  TEST_ASSERT_EQ_INT(sqlite3_backup_remaining(backup), 0);

  TEST_OK(dst, sqlite3_backup_finish(backup));

  sqlite3_stmt *stmt;
  TEST_OK(dst, sqlite3_prepare_v2(dst, "SELECT COUNT(*), SUM(x) FROM t", -1,
                                   &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 5000);
  TEST_ASSERT_EQ_INT(sqlite3_column_int64(stmt, 1), 5000LL * 5001 / 2);
  sqlite3_finalize(stmt);

  test_close_ok(src);
  test_close_ok(dst);
  TEST_PASS("test_backup_api");
  return 0;
}
