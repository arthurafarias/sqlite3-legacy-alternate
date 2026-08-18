#include "test_common.h"

static void expect_scalar_text(sqlite3 *db, const char *sql, const char *expected) {
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, sql, -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 0), expected);
  sqlite3_finalize(stmt);
}

int main(void) {
  sqlite3 *db = test_open_memory_db();

  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT json_valid('{\"a\":1}'), json_valid('not json')",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 1);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 1), 0);
  sqlite3_finalize(stmt);

  expect_scalar_text(db, "SELECT json_extract('{\"a\":{\"b\":42}}', '$.a.b')", "42");
  expect_scalar_text(db, "SELECT json_array(1,2,3)", "[1,2,3]");
  expect_scalar_text(db, "SELECT json_object('x', 1, 'y', 'z')", "{\"x\":1,\"y\":\"z\"}");
  expect_scalar_text(db, "SELECT json_type('{\"a\":1}', '$.a')", "integer");
  expect_scalar_text(db, "SELECT json_remove('{\"a\":1,\"b\":2}', '$.a')", "{\"b\":2}");
  expect_scalar_text(db, "SELECT json_insert('{\"a\":1}', '$.b', 2)", "{\"a\":1,\"b\":2}");

  /* json_each / json_tree: table-valued functions that flatten JSON. */
  test_exec_ok(db, "CREATE TABLE docs(id INTEGER PRIMARY KEY, blob TEXT)");
  test_exec_ok(db, "INSERT INTO docs(blob) VALUES "
                   "('{\"tags\":[\"x\",\"y\",\"z\"]}')");

  TEST_OK(db, sqlite3_prepare_v2(
                  db,
                  "SELECT je.value FROM docs, "
                  "json_each(docs.blob, '$.tags') AS je ORDER BY je.key",
                  -1, &stmt, NULL));
  const char *expected[] = {"x", "y", "z"};
  for (int i = 0; i < 3; i++) {
    TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
    TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 0), expected[i]);
  }
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_DONE);
  sqlite3_finalize(stmt);

  TEST_OK(db, sqlite3_prepare_v2(
                  db, "SELECT COUNT(*) FROM docs, json_tree(docs.blob) AS jt",
                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT(sqlite3_column_int(stmt, 0) > 3); /* root + array + 3 elements */
  sqlite3_finalize(stmt);

  /* A generated column driven entirely by JSON functions. */
  test_exec_ok(db, "ALTER TABLE docs ADD COLUMN tag_count INTEGER "
                   "GENERATED ALWAYS AS (json_array_length(blob, '$.tags')) VIRTUAL");
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT tag_count FROM docs", -1, &stmt,
                                  NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 3);
  sqlite3_finalize(stmt);

  test_close_ok(db);
  TEST_PASS("test_json_functions");
  return 0;
}
