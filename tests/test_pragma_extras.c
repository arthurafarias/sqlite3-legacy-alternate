#include "test_common.h"

static void exec_and_drain(sqlite3 *db, const char *sql) {
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, sql, -1, &stmt, NULL));
  while (sqlite3_step(stmt) == SQLITE_ROW) {
  }
  sqlite3_finalize(stmt);
}

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE parent(id INTEGER PRIMARY KEY)");
  test_exec_ok(db, "CREATE TABLE child(id INTEGER PRIMARY KEY, "
                   "parent_id INTEGER REFERENCES parent(id))");
  test_exec_ok(db, "PRAGMA foreign_keys = ON");
  test_exec_ok(db, "INSERT INTO parent VALUES (1)");

  /* foreign_key_check scans for existing violations (there are none yet);
   * foreign_key_list describes the constraint itself. */
  exec_and_drain(db, "PRAGMA foreign_key_check");
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "PRAGMA foreign_key_list(child)", -1,
                                  &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 2) /* table */, "parent");
  sqlite3_finalize(stmt);

  /* index_list / index_info / index_xinfo: schema introspection for indexes. */
  test_exec_ok(db, "CREATE INDEX idx_parent_id ON child(parent_id)");
  TEST_OK(db, sqlite3_prepare_v2(db, "PRAGMA index_list(child)", -1, &stmt, NULL));
  int found_idx = 0;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    if (strcmp((const char *)sqlite3_column_text(stmt, 1), "idx_parent_id") == 0)
      found_idx = 1;
  }
  TEST_ASSERT(found_idx);
  sqlite3_finalize(stmt);

  TEST_OK(db, sqlite3_prepare_v2(db, "PRAGMA index_info(idx_parent_id)", -1,
                                  &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 2), "parent_id");
  sqlite3_finalize(stmt);

  TEST_OK(db, sqlite3_prepare_v2(db, "PRAGMA index_xinfo(idx_parent_id)", -1,
                                  &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  sqlite3_finalize(stmt);

  /* function_list / module_list / pragma_list / compile_options: catalogs
   * of what this build actually supports. */
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM pragma_function_list",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT(sqlite3_column_int(stmt, 0) > 20);
  sqlite3_finalize(stmt);

  /* This build registers no loadable virtual table modules (no FTS5,
   * RTREE, dbstat, ...) -- pragma_module_list only reports itself, the
   * pragma-backed vtab that services this very query. */
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM pragma_module_list",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT(sqlite3_column_int(stmt, 0) >= 1);
  sqlite3_finalize(stmt);

  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM pragma_pragma_list",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT(sqlite3_column_int(stmt, 0) > 20);
  sqlite3_finalize(stmt);

  TEST_OK(db, sqlite3_prepare_v2(db, "PRAGMA compile_options", -1, &stmt, NULL));
  int found_math_opt = 0;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const char *opt = (const char *)sqlite3_column_text(stmt, 0);
    if (strstr(opt, "ENABLE_MATH_FUNCTIONS"))
      found_math_opt = 1;
  }
  TEST_ASSERT(found_math_opt);
  sqlite3_finalize(stmt);

  /* Tuning pragmas: cache_size (negative = KB, positive = pages), mmap_size,
   * wal_autocheckpoint -- each just a get/set integer, but each touches a
   * different subsystem's configuration path. */
  test_exec_ok(db, "PRAGMA cache_size = -4000"); /* 4MB cache */
  test_exec_ok(db, "PRAGMA cache_size = 500");   /* 500 pages */
  test_exec_ok(db, "PRAGMA mmap_size = 1048576");
  test_exec_ok(db, "PRAGMA wal_autocheckpoint = 500");
  test_exec_ok(db, "PRAGMA temp_store = MEMORY");
  test_exec_ok(db, "PRAGMA optimize");
  test_exec_ok(db, "PRAGMA shrink_memory");

  test_close_ok(db);
  TEST_PASS("test_pragma_extras");
  return 0;
}
