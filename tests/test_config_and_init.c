#include "test_common.h"

/* sqlite3_config() is process-global and may only be called before the
 * library is initialized (or after an explicit sqlite3_shutdown()). Every
 * other test in this suite opens a database as its very first SQLite call,
 * which auto-initializes the library with default settings -- so this
 * config-focused test is deliberately kept in its own process/executable
 * and does the shutdown/config/init dance before touching anything else. */

int main(void) {
  TEST_ASSERT(sqlite3_shutdown() == SQLITE_OK);

  /* Enabling memory status tracking must happen before (re-)initializing. */
  TEST_ASSERT(sqlite3_config(SQLITE_CONFIG_MEMSTATUS, 1) == SQLITE_OK);
  TEST_ASSERT(sqlite3_initialize() == SQLITE_OK);

  /* Soft/hard heap limits: get-or-set via the same call (negative size
   * queries without changing the limit). */
  sqlite3_int64 prev_soft = sqlite3_soft_heap_limit64(-1);
  TEST_ASSERT(prev_soft >= 0);
  sqlite3_int64 set_result = sqlite3_soft_heap_limit64(64 * 1024 * 1024);
  TEST_ASSERT_MSG(set_result == prev_soft,
                   "soft_heap_limit64 should return the *previous* limit");
  TEST_ASSERT(sqlite3_soft_heap_limit64(-1) == 64 * 1024 * 1024);
  sqlite3_soft_heap_limit64(prev_soft); /* restore */

  sqlite3_int64 prev_hard = sqlite3_hard_heap_limit64(-1);
  sqlite3_hard_heap_limit64(prev_hard); /* exercise the call; leave unchanged */

  /* Now do ordinary database work to confirm the library is fully usable
   * after this init dance, and that memory tracking is live. */
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE t(x)");
  test_exec_ok(db, "INSERT INTO t VALUES (1), (2), (3)");

  sqlite3_int64 cur = 0, hi = 0;
  TEST_OK(db, sqlite3_status64(SQLITE_STATUS_MEMORY_USED, &cur, &hi, 0));
  TEST_ASSERT_MSG(cur > 0, "SQLITE_CONFIG_MEMSTATUS was enabled; expected live tracking");

  test_close_ok(db);
  TEST_PASS("test_config_and_init");
  return 0;
}
