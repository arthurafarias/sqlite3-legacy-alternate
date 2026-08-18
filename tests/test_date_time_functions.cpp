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

  /* date()/time()/datetime() format a fixed timestamp deterministically --
   * none of these depend on the current wall clock, so they're safe to
   * assert on exactly. */
  expect_text(db, "SELECT date('2024-03-15 10:30:00')", "2024-03-15");
  expect_text(db, "SELECT time('2024-03-15 10:30:00')", "10:30:00");
  expect_text(db, "SELECT datetime('2024-03-15 10:30:00')", "2024-03-15 10:30:00");

  /* julianday()/unixepoch(): numeric time representations. */
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT julianday('2000-01-01 12:00:00')",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT(sqlite3_column_double(stmt, 0) > 2451544 &&
              sqlite3_column_double(stmt, 0) < 2451546);
  sqlite3_finalize(stmt);

  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT unixepoch('1970-01-01 00:00:10')",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 10);
  sqlite3_finalize(stmt);

  /* Modifiers: relative offsets and "start of" truncation. */
  expect_text(db, "SELECT date('2024-03-15', '+1 day')", "2024-03-16");
  expect_text(db, "SELECT date('2024-03-15', '-1 month')", "2024-02-15");
  expect_text(db, "SELECT date('2024-03-15', 'start of month')", "2024-03-01");
  expect_text(db, "SELECT date('2024-03-15', 'start of year')", "2024-01-01");
  expect_text(db, "SELECT date('2024-03-15', 'weekday 0')", "2024-03-17"); /* next Sunday */
  expect_text(db, "SELECT datetime('2024-03-15 10:00:00', '+90 minutes')",
              "2024-03-15 11:30:00");

  /* strftime(): custom format specifiers. */
  expect_text(db, "SELECT strftime('%Y-%m-%d', '2024-03-15')", "2024-03-15");
  expect_text(db, "SELECT strftime('%W', '2024-03-15')", "11"); /* ISO-ish week number */
  expect_text(db, "SELECT strftime('%j', '2024-01-31')", "031"); /* day of year */
  expect_text(db, "SELECT strftime('%s', '1970-01-01 00:01:00')", "60"); /* unix epoch */

  /* Feb 29 on a leap year normalizes correctly across a +1 year shift. */
  expect_text(db, "SELECT date('2024-02-29', '+1 year')", "2025-03-01");

  /* timediff(): the signed ISO-8601 duration between two timestamps
   * (added in SQLite 3.43). */
  expect_text(db, "SELECT timediff('2024-01-02', '2024-01-01')", "+0000-00-01 00:00:00.000");

  /* An invalid timestamp propagates as SQL NULL, not an error. */
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT date('not-a-date')", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT(sqlite3_column_type(stmt, 0) == SQLITE_NULL);
  sqlite3_finalize(stmt);

  test_close_ok(db);
  TEST_PASS("test_date_time_functions");
  return 0;
}
