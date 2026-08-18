#include "test_common.h"

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE t(id INTEGER PRIMARY KEY, score REAL)");
  test_exec_ok(db, "INSERT INTO t VALUES (1, 9.5)");

  /* sqlite3_column_decltype reports the schema-declared type, independent
   * of the runtime type of the value actually produced. */
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT id, score, id + 1, 'lit' FROM t",
                                  -1, &stmt, NULL));
  TEST_ASSERT_EQ_STR(sqlite3_column_decltype(stmt, 0), "INTEGER");
  TEST_ASSERT_EQ_STR(sqlite3_column_decltype(stmt, 1), "REAL");
  TEST_ASSERT(sqlite3_column_decltype(stmt, 2) == NULL); /* computed expr: no decltype */
  TEST_ASSERT(sqlite3_column_decltype(stmt, 3) == NULL); /* literal: no decltype */

  /* SQLITE_ENABLE_COLUMN_METADATA (provenance: which database, table, and
   * source column a result column was read from via
   * sqlite3_column_database_name/table_name/origin_name) isn't part of
   * this build -- those symbols are declared but not defined. */

  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);

  /* sqlite3_data_count: number of columns in the *current row*, which is 0
   * once the statement is exhausted (unlike column_count, which reflects
   * the statement's shape regardless of row position). */
  TEST_ASSERT_EQ_INT(sqlite3_data_count(stmt), 4);
  TEST_ASSERT_EQ_INT(sqlite3_column_count(stmt), 4);

  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_DONE);
  TEST_ASSERT_EQ_INT(sqlite3_data_count(stmt), 0);
  TEST_ASSERT_EQ_INT(sqlite3_column_count(stmt), 4); /* unchanged */

  sqlite3_finalize(stmt);
  test_close_ok(db);
  TEST_PASS("test_column_metadata_extras");
  return 0;
}
