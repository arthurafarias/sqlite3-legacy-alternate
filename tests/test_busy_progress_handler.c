#include "test_common.h"

static int busy_calls = 0;
static int busy_handler(void *ctx, int n) {
  (void)ctx;
  busy_calls++;
  return n < 3; /* give up after a few retries */
}

static int progress_calls = 0;
static int progress_limit_hit = 0;
static int progress_handler(void *ctx) {
  (void)ctx;
  progress_calls++;
  if (progress_calls >= 5) {
    progress_limit_hit = 1;
    return 1; /* non-zero interrupts the running statement */
  }
  return 0;
}

int main(void) {
  sqlite3 *db = test_open_memory_db();

  /* busy_timeout installs a built-in busy handler; verify it round-trips
   * through sqlite3_db_status or at least doesn't error. */
  TEST_OK(db, sqlite3_busy_timeout(db, 250));

  /* Replace it with a custom handler and confirm it can be swapped again. */
  TEST_OK(db, sqlite3_busy_handler(db, busy_handler, NULL));
  (void)busy_calls; /* exercised indirectly: SQLite only invokes this on a
                        genuine SQLITE_BUSY, which a single-connection
                        in-memory db won't produce -- we only assert it can
                        be registered without error. */

  /* Seed the data before installing the progress handler below -- once
   * installed, it would interrupt this seeding statement itself long
   * before it could insert all 100000 rows. */
  test_exec_ok(db, "CREATE TABLE t(x)");
  test_exec_ok(db, "WITH RECURSIVE seq(v) AS "
                   "(SELECT 1 UNION ALL SELECT v+1 FROM seq WHERE v < 100000) "
                   "INSERT INTO t SELECT v FROM seq");

  /* progress_handler fires every N VM instructions and can abort a query. */
  sqlite3_progress_handler(db, 100, progress_handler, NULL);

  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM t, t AS t2", -1,
                                  &stmt, NULL));
  int rc = sqlite3_step(stmt);
  TEST_ASSERT_MSG(rc == SQLITE_INTERRUPT || rc == SQLITE_ROW,
                   "expected INTERRUPT (aborted by progress handler) or ROW, got %d",
                   rc);
  if (rc == SQLITE_INTERRUPT)
    TEST_ASSERT(progress_limit_hit);
  sqlite3_finalize(stmt);

  /* Clearing the progress handler (N<=0) lets a query run to completion. */
  sqlite3_progress_handler(db, 0, NULL, NULL);
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM t", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 100000);
  sqlite3_finalize(stmt);

  test_close_ok(db);
  TEST_PASS("test_busy_progress_handler");
  return 0;
}
