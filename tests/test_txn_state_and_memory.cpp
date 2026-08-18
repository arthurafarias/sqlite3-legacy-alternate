#include "test_common.h"

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE t(x)");

  /* sqlite3_txn_state: none outside a transaction, read/write once one is
   * active and has (or hasn't yet) taken a write lock. */
  TEST_ASSERT_EQ_INT(sqlite3_txn_state(db, "main"), SQLITE_TXN_NONE);

  /* A plain (deferred) BEGIN doesn't take any lock by itself -- SQLite
   * defers that until the first statement actually touches the schema --
   * so the state only becomes READ once something runs. */
  test_exec_ok(db, "BEGIN");
  TEST_ASSERT_EQ_INT(sqlite3_txn_state(db, "main"), SQLITE_TXN_NONE);
  test_exec_ok(db, "SELECT * FROM t");
  TEST_ASSERT_EQ_INT(sqlite3_txn_state(db, "main"), SQLITE_TXN_READ);
  test_exec_ok(db, "INSERT INTO t VALUES (1)");
  TEST_ASSERT_EQ_INT(sqlite3_txn_state(db, "main"), SQLITE_TXN_WRITE);
  test_exec_ok(db, "COMMIT");
  TEST_ASSERT_EQ_INT(sqlite3_txn_state(db, "main"), SQLITE_TXN_NONE);

  /* NULL schema name reports the "worst" state across all attached dbs. */
  TEST_ASSERT_EQ_INT(sqlite3_txn_state(db, NULL), SQLITE_TXN_NONE);
  test_exec_ok(db, "BEGIN");
  test_exec_ok(db, "SELECT * FROM t");
  TEST_ASSERT_EQ_INT(sqlite3_txn_state(db, NULL), SQLITE_TXN_READ);
  test_exec_ok(db, "COMMIT");

  /* An unknown schema name is reported as -1. */
  TEST_ASSERT_EQ_INT(sqlite3_txn_state(db, "no_such_schema"), -1);

  /* sqlite3_db_cacheflush forces dirty pages out without a full commit's
   * worth of bookkeeping -- a no-op here since nothing is dirty, but the
   * call itself must succeed. */
  TEST_ASSERT(sqlite3_db_cacheflush(db) == SQLITE_OK);

  /* sqlite3_db_release_memory / sqlite3_release_memory: best-effort cache
   * trimming. Return values vary by build/allocator; the calls simply must
   * not crash and must return a non-negative byte count. */
  int freed = sqlite3_db_release_memory(db);
  TEST_ASSERT(freed >= 0);
  int freed_global = sqlite3_release_memory(1024);
  TEST_ASSERT(freed_global >= 0);

  test_close_ok(db);
  TEST_PASS("test_txn_state_and_memory");
  return 0;
}
