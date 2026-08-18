#include "test_common.h"

static int table_exists(sqlite3 *db, const char *name) {
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(
                  db, "SELECT 1 FROM sqlite_schema WHERE type='table' AND name=?",
                  -1, &stmt, NULL));
  sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
  int found = sqlite3_step(stmt) == SQLITE_ROW;
  sqlite3_finalize(stmt);
  return found;
}

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE widgets(id INTEGER PRIMARY KEY, name TEXT)");
  test_exec_ok(db, "INSERT INTO widgets VALUES (1, 'sprocket')");

  /* ADD COLUMN, with and without a default. */
  test_exec_ok(db, "ALTER TABLE widgets ADD COLUMN price REAL DEFAULT 0");
  test_exec_ok(db, "ALTER TABLE widgets ADD COLUMN notes TEXT");
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT price, notes FROM widgets WHERE id=1",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT(sqlite3_column_double(stmt, 0) == 0.0);
  TEST_ASSERT(sqlite3_column_type(stmt, 1) == SQLITE_NULL);
  sqlite3_finalize(stmt);

  /* RENAME COLUMN updates references transparently. */
  test_exec_ok(db, "ALTER TABLE widgets RENAME COLUMN notes TO description");
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT description FROM widgets WHERE id=1",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  sqlite3_finalize(stmt);

  /* DROP COLUMN. */
  test_exec_ok(db, "ALTER TABLE widgets DROP COLUMN description");
  int rc = sqlite3_prepare_v2(db, "SELECT description FROM widgets", -1, &stmt, NULL);
  TEST_ASSERT_MSG(rc != SQLITE_OK, "dropped column should no longer be selectable");
  if (rc == SQLITE_OK)
    sqlite3_finalize(stmt);

  /* RENAME TO renames the table itself, including in sqlite_schema. */
  TEST_ASSERT(table_exists(db, "widgets"));
  test_exec_ok(db, "ALTER TABLE widgets RENAME TO products");
  TEST_ASSERT(!table_exists(db, "widgets"));
  TEST_ASSERT(table_exists(db, "products"));

  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT name FROM products WHERE id=1", -1,
                                  &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 0), "sprocket");
  sqlite3_finalize(stmt);

  test_close_ok(db);
  TEST_PASS("test_alter_table");
  return 0;
}
