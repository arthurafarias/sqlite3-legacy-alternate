#include "test_common.h"

static void expect_text(sqlite3 *db, const char *sql, const char *expected) {
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, sql, -1, &stmt, NULL));
  TEST_ASSERT_MSG(sqlite3_step(stmt) == SQLITE_ROW, "no row for: %s", sql);
  TEST_ASSERT_MSG(strcmp((const char *)sqlite3_column_text(stmt, 0), expected) == 0,
                   "%s => \"%s\", expected \"%s\"", sql,
                   (const char *)sqlite3_column_text(stmt, 0), expected);
  sqlite3_finalize(stmt);
}

int main(void) {
  sqlite3 *db = test_open_memory_db();

  /* printf()/format(): %q escapes embedded quotes for safe re-quoting, %Q
   * quotes-and-escapes a whole value (or emits NULL literally), %w escapes
   * identifiers, %! is deprecated but still parsed. */
  expect_text(db, "SELECT printf('%q', 'O''Brien')", "O''Brien");
  expect_text(db, "SELECT printf('%Q', 'it''s')", "'it''s'");
  expect_text(db, "SELECT printf('%Q', NULL)", "NULL");
  expect_text(db, "SELECT printf('%d-%05d', 7, 42)", "7-00042");
  expect_text(db, "SELECT printf('%.2f', 3.14159)", "3.14");
  expect_text(db, "SELECT printf('%x', 255)", "ff");
  expect_text(db, "SELECT format('%s has %d items', 'cart', 3)", "cart has 3 items");

  /* hex()/unhex(): binary <-> hex-text round trip. */
  expect_text(db, "SELECT hex('AB')", "4142");
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT unhex('4142')", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_bytes(stmt, 0), 2);
  TEST_ASSERT(memcmp(sqlite3_column_blob(stmt, 0), "AB", 2) == 0);
  sqlite3_finalize(stmt);
  /* unhex() on an odd-length or non-hex string is NULL, not an error. */
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT unhex('xyz')", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT(sqlite3_column_type(stmt, 0) == SQLITE_NULL);
  sqlite3_finalize(stmt);

  /* X'..' blob literal syntax parses directly, independent of hex()/unhex(). */
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT X'DEADBEEF'", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT(sqlite3_column_type(stmt, 0) == SQLITE_BLOB);
  TEST_ASSERT_EQ_INT(sqlite3_column_bytes(stmt, 0), 4);
  static const unsigned char expected_bytes[] = {0xDE, 0xAD, 0xBE, 0xEF};
  TEST_ASSERT(memcmp(sqlite3_column_blob(stmt, 0), expected_bytes, 4) == 0);
  sqlite3_finalize(stmt);

  /* CAST across every storage class, including the lossy/truncating cases. */
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT CAST('42abc' AS INTEGER)", -1,
                                  &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 42); /* leading numeric prefix */
  sqlite3_finalize(stmt);
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT CAST(3.9 AS INTEGER)", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 3); /* truncates, doesn't round */
  sqlite3_finalize(stmt);
  expect_text(db, "SELECT CAST(42 AS TEXT)", "42");
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT CAST('3.5' AS REAL)", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT(sqlite3_column_double(stmt, 0) == 3.5);
  sqlite3_finalize(stmt);

  /* Built-in collations: BINARY (default, byte-for-byte), NOCASE (ASCII
   * case-insensitive), RTRIM (ignores trailing whitespace only). */
  test_exec_ok(db, "CREATE TABLE t(a TEXT COLLATE BINARY, b TEXT COLLATE NOCASE, "
                   "c TEXT COLLATE RTRIM)");
  test_exec_ok(db, "INSERT INTO t VALUES ('X', 'X', 'X  ')");
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT a='x', b='x', c='X' FROM t", -1,
                                  &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 0); /* BINARY: 'X' != 'x' */
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 1), 1); /* NOCASE: 'X' == 'x' */
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 2), 1); /* RTRIM: 'X  ' == 'X' */
  sqlite3_finalize(stmt);

  test_close_ok(db);
  TEST_PASS("test_format_and_literals");
  return 0;
}
