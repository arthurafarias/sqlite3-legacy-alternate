#include "test_common.h"

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE t(id INTEGER PRIMARY KEY, v TEXT)");

  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "INSERT INTO t(v) VALUES (?)", -1, &stmt,
                                  NULL));
  for (int i = 0; i < 1000; i++) {
    sqlite3_bind_text(stmt, 1, "payload", -1, SQLITE_STATIC);
    TEST_ASSERT(sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_reset(stmt);
  }

  /* Per-statement VM step / re-prepare counters. */
  int vm_steps = sqlite3_stmt_status(stmt, SQLITE_STMTSTATUS_VM_STEP, 0);
  TEST_ASSERT_MSG(vm_steps > 0, "expected some VM steps to be recorded, got %d",
                   vm_steps);
  int run_count = sqlite3_stmt_status(stmt, SQLITE_STMTSTATUS_RUN, 1 /*reset*/);
  TEST_ASSERT_EQ_INT(run_count, 1000);
  int run_count_after_reset =
      sqlite3_stmt_status(stmt, SQLITE_STMTSTATUS_RUN, 0);
  TEST_ASSERT_EQ_INT(run_count_after_reset, 0);
  sqlite3_finalize(stmt);

  /* Per-connection cache/lookaside counters. */
  int cur, hi;
  TEST_OK(db, sqlite3_db_status(db, SQLITE_DBSTATUS_LOOKASIDE_USED, &cur, &hi, 0));
  TEST_ASSERT(cur >= 0 && hi >= 0);

  TEST_OK(db, sqlite3_db_status(db, SQLITE_DBSTATUS_CACHE_USED, &cur, &hi, 0));
  TEST_ASSERT_MSG(cur > 0, "expect nonzero page cache usage after 1000 inserts");

  /* Process-wide allocator counters (SQLITE_ENABLE_MEMSYS... not required;
   * sqlite3_memory_used tracks every malloc/free the library makes). */
  sqlite3_int64 mem_used = sqlite3_memory_used();
  TEST_ASSERT_MSG(mem_used > 0, "expect nonzero live allocation total");
  sqlite3_int64 mem_high = sqlite3_memory_highwater(0);
  TEST_ASSERT(mem_high >= mem_used);

  /* Process-wide status via sqlite3_status64 mirrors the same counters. */
  sqlite3_int64 status_cur = 0, status_hi = 0;
  TEST_OK(db, sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &status_cur, &status_hi, 0));
  TEST_ASSERT(status_cur > 0);

  test_close_ok(db);
  TEST_PASS("test_status_counters");
  return 0;
}
