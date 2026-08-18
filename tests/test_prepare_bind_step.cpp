#include "test_common.h"

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db,
               "CREATE TABLE t(i INTEGER, r REAL, t TEXT, b BLOB, n)");

  sqlite3_stmt *stmt = NULL;
  const char *tail = NULL;
  TEST_OK(db, sqlite3_prepare_v2(
                  db, "INSERT INTO t VALUES (:i, :r, :t, :b, :n)", -1, &stmt,
                  &tail));

  TEST_ASSERT_EQ_INT(sqlite3_bind_parameter_count(stmt), 5);
  int idx_i = sqlite3_bind_parameter_index(stmt, ":i");
  TEST_ASSERT_EQ_INT(idx_i, 1);
  TEST_ASSERT_EQ_STR(sqlite3_bind_parameter_name(stmt, 1), ":i");

  static const unsigned char blob_data[] = {0xDE, 0xAD, 0xBE, 0xEF};
  TEST_OK(db, sqlite3_bind_int64(stmt, 1, 9223372036854775807LL));
  TEST_OK(db, sqlite3_bind_double(stmt, 2, 3.14159265358979));
  TEST_OK(db, sqlite3_bind_text(stmt, 3, "hello, world", -1, SQLITE_TRANSIENT));
  TEST_OK(db, sqlite3_bind_blob(stmt, 4, blob_data, sizeof(blob_data),
                                 SQLITE_STATIC));
  TEST_OK(db, sqlite3_bind_null(stmt, 5));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_DONE);

  /* Reset + clear bindings, then insert an all-NULL row via zeroblob. */
  TEST_OK(db, sqlite3_reset(stmt));
  TEST_OK(db, sqlite3_clear_bindings(stmt));
  TEST_OK(db, sqlite3_bind_int(stmt, 1, 0));
  TEST_OK(db, sqlite3_bind_null(stmt, 2));
  TEST_OK(db, sqlite3_bind_null(stmt, 3));
  TEST_OK(db, sqlite3_bind_zeroblob(stmt, 4, 8));
  TEST_OK(db, sqlite3_bind_null(stmt, 5));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_DONE);
  sqlite3_finalize(stmt);

  /* Read the first row back and walk every sqlite3_column_* accessor. */
  TEST_OK(db, sqlite3_prepare_v3(db, "SELECT i, r, t, b, n FROM t WHERE i > 0",
                                  -1, 0, &stmt, NULL));
  TEST_ASSERT_EQ_INT(sqlite3_column_count(stmt), 5);
  TEST_ASSERT_EQ_STR(sqlite3_column_name(stmt, 0), "i");

  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT(sqlite3_column_type(stmt, 0) == SQLITE_INTEGER);
  TEST_ASSERT_EQ_INT(sqlite3_column_int64(stmt, 0), 9223372036854775807LL);
  TEST_ASSERT(sqlite3_column_type(stmt, 1) == SQLITE_FLOAT);
  TEST_ASSERT(sqlite3_column_double(stmt, 1) > 3.14 &&
              sqlite3_column_double(stmt, 1) < 3.15);
  TEST_ASSERT(sqlite3_column_type(stmt, 2) == SQLITE_TEXT);
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 2), "hello, world");
  TEST_ASSERT_EQ_INT(sqlite3_column_bytes(stmt, 2), (int)strlen("hello, world"));
  TEST_ASSERT(sqlite3_column_type(stmt, 3) == SQLITE_BLOB);
  TEST_ASSERT_EQ_INT(sqlite3_column_bytes(stmt, 3), sizeof(blob_data));
  TEST_ASSERT(memcmp(sqlite3_column_blob(stmt, 3), blob_data,
                      sizeof(blob_data)) == 0);
  TEST_ASSERT(sqlite3_column_type(stmt, 4) == SQLITE_NULL);

  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_DONE);
  sqlite3_finalize(stmt);

  test_close_ok(db);
  TEST_PASS("test_prepare_bind_step");
  return 0;
}
