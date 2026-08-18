#include "test_common.h"

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE counters(name TEXT PRIMARY KEY, hits INTEGER NOT NULL)");

  /* INSERT ... ON CONFLICT DO UPDATE (upsert), keeping a running counter. */
  const char *upsert_sql =
      "INSERT INTO counters(name, hits) VALUES (?, 1) "
      "ON CONFLICT(name) DO UPDATE SET hits = hits + 1";
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, upsert_sql, -1, &stmt, NULL));
  for (int i = 0; i < 5; i++) {
    sqlite3_bind_text(stmt, 1, "page_views", -1, SQLITE_STATIC);
    TEST_ASSERT(sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_reset(stmt);
  }
  sqlite3_finalize(stmt);

  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT hits FROM counters WHERE name='page_views'",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 5);
  sqlite3_finalize(stmt);

  /* ON CONFLICT DO NOTHING silently skips the duplicate. */
  TEST_OK(db, sqlite3_prepare_v2(
                  db, "INSERT INTO counters VALUES ('page_views', 999) ON CONFLICT DO NOTHING",
                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_DONE);
  sqlite3_finalize(stmt);
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT hits FROM counters WHERE name='page_views'",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 5); /* unchanged */
  sqlite3_finalize(stmt);

  /* RETURNING clause hands back the affected row without a follow-up
   * SELECT, for INSERT, UPDATE, and DELETE alike. */
  TEST_OK(db, sqlite3_prepare_v2(
                  db, "INSERT INTO counters VALUES ('signups', 1) RETURNING name, hits",
                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 0), "signups");
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 1), 1);
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_DONE);
  sqlite3_finalize(stmt);

  TEST_OK(db, sqlite3_prepare_v2(
                  db, "UPDATE counters SET hits = hits * 10 WHERE name='signups' RETURNING hits",
                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 10);
  sqlite3_finalize(stmt);

  TEST_OK(db, sqlite3_prepare_v2(
                  db, "DELETE FROM counters WHERE name='signups' RETURNING name",
                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 0), "signups");
  sqlite3_finalize(stmt);

  test_close_ok(db);
  TEST_PASS("test_upsert_returning");
  return 0;
}
