#include "test_common.h"

/* SAMPLE_EXTENSION_PATH is injected by CMake as the built path of the
 * sample_extension shared library (tests/extensions/sample_extension.c). */
#ifndef SAMPLE_EXTENSION_PATH
#error "SAMPLE_EXTENSION_PATH must be defined by the build"
#endif

int main(void) {
  sqlite3 *db = test_open_memory_db();

  /* Loading is off by default; sqlite3_load_extension() itself should
   * fail until explicitly enabled. */
  char *errmsg = NULL;
  int rc = sqlite3_load_extension(db, SAMPLE_EXTENSION_PATH, NULL, &errmsg);
  TEST_ASSERT_MSG(rc != SQLITE_OK, "load_extension should be disabled by default");
  sqlite3_free(errmsg);

  TEST_OK(db, sqlite3_enable_load_extension(db, 1));
  errmsg = NULL;
  rc = sqlite3_load_extension(db, SAMPLE_EXTENSION_PATH, NULL, &errmsg);
  TEST_ASSERT_MSG(rc == SQLITE_OK, "load_extension failed: %s",
                   errmsg ? errmsg : "(no message)");
  sqlite3_free(errmsg);

  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT ext_double(21)", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT(sqlite3_column_double(stmt, 0) == 42.0);
  sqlite3_finalize(stmt);

  /* The SQL-visible load_extension() function is gated by the same switch.
   * Exercise it on a *fresh* connection: SQLite refuses to redefine a UDF
   * while any statement on that connection is active, and the very call to
   * `SELECT load_extension(...)` is itself such an active statement, so
   * re-registering ext_double on the already-loaded `db` from here would
   * fail with "unable to delete/modify user-function due to active
   * statements" -- a real, distinct error path worth keeping separate
   * rather than papering over by loading only once. */
  sqlite3 *db2 = test_open_memory_db();
  TEST_OK(db2, sqlite3_enable_load_extension(db2, 1));
  TEST_OK(db2, sqlite3_prepare_v2(db2, "SELECT load_extension(?)", -1, &stmt, NULL));
  sqlite3_bind_text(stmt, 1, SAMPLE_EXTENSION_PATH, -1, SQLITE_STATIC);
  rc = sqlite3_step(stmt);
  TEST_ASSERT_MSG(rc == SQLITE_ROW, "SQL-level load_extension() failed: %s",
                   sqlite3_errmsg(db2));
  sqlite3_finalize(stmt);
  TEST_OK(db2, sqlite3_prepare_v2(db2, "SELECT ext_double(10)", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT(sqlite3_column_double(stmt, 0) == 20.0);
  sqlite3_finalize(stmt);
  test_close_ok(db2);

  /* Disabling it again blocks further loads (but doesn't unload what's
   * already registered). */
  TEST_OK(db, sqlite3_enable_load_extension(db, 0));
  errmsg = NULL;
  rc = sqlite3_load_extension(db, SAMPLE_EXTENSION_PATH, NULL, &errmsg);
  TEST_ASSERT_MSG(rc != SQLITE_OK, "load_extension should be blocked once disabled again");
  sqlite3_free(errmsg);

  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT ext_double(2)", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT(sqlite3_column_double(stmt, 0) == 4.0); /* still registered */
  sqlite3_finalize(stmt);

  test_close_ok(db);
  TEST_PASS("test_load_extension");
  return 0;
}
