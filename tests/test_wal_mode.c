#include "test_common.h"
#include <unistd.h>

/* WAL mode requires a real file on disk -- an in-memory database always
 * reports "memory" for journal_mode regardless of what's requested. Each
 * test runs with CTest's working directory set to its own build subdir, so
 * a relative filename is safe and self-cleaning. */
#define DB_PATH "./test_wal_mode_scratch.db"

static void remove_db_files(void) {
  remove(DB_PATH);
  remove(DB_PATH "-wal");
  remove(DB_PATH "-shm");
  remove(DB_PATH "-journal");
}

int main(void) {
  remove_db_files();

  sqlite3 *db = NULL;
  int rc = sqlite3_open(DB_PATH, &db);
  TEST_OK(db, rc);

  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "PRAGMA journal_mode=WAL", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 0), "wal");
  sqlite3_finalize(stmt);

  test_exec_ok(db, "CREATE TABLE t(x INTEGER)");
  test_exec_ok(db, "INSERT INTO t VALUES (1), (2), (3)");

  /* After a write in WAL mode, a *-wal file should exist alongside the db. */
  TEST_ASSERT_MSG(access(DB_PATH "-wal", F_OK) == 0,
                   "expected a -wal file to be created");

  /* Explicit checkpoint folds the WAL back into the main database file. */
  int nLog = -1, nCkpt = -1;
  TEST_OK(db, sqlite3_wal_checkpoint_v2(db, "main", SQLITE_CHECKPOINT_FULL,
                                         &nLog, &nCkpt));
  TEST_ASSERT(nLog >= 0);
  TEST_ASSERT(nCkpt >= 0);
  TEST_ASSERT_EQ_INT(nLog, nCkpt); /* FULL checkpoint copies every frame */

  sqlite3_stmt *count_stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM t", -1, &count_stmt,
                                  NULL));
  TEST_ASSERT(sqlite3_step(count_stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(count_stmt, 0), 3);
  sqlite3_finalize(count_stmt);

  /* Switch back to a rollback journal for good measure. */
  TEST_OK(db, sqlite3_prepare_v2(db, "PRAGMA journal_mode=DELETE", -1, &stmt,
                                  NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 0), "delete");
  sqlite3_finalize(stmt);

  test_close_ok(db);
  remove_db_files();
  TEST_PASS("test_wal_mode");
  return 0;
}
