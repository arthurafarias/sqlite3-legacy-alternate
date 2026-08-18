#include "test_common.h"

static int query_plan_mentions(sqlite3 *db, const char *sql, const char *needle) {
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, sql, -1, &stmt, NULL));
  int found = 0;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const char *detail = (const char *)sqlite3_column_text(stmt, 3);
    if (detail && strstr(detail, needle))
      found = 1;
  }
  sqlite3_finalize(stmt);
  return found;
}

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE t(id INTEGER PRIMARY KEY, status TEXT, "
                   "email TEXT, amount INTEGER)");
  test_exec_ok(db, "WITH RECURSIVE seq(v) AS "
                   "(SELECT 1 UNION ALL SELECT v+1 FROM seq WHERE v < 5000) "
                   "INSERT INTO t SELECT v, CASE WHEN v % 100 = 0 THEN 'archived' "
                   "ELSE 'active' END, 'user' || v || '@example.com', v * 3 "
                   "FROM seq");
  test_exec_ok(db, "ANALYZE");

  /* A partial index only covers rows matching its WHERE clause -- much
   * smaller than a full index when the predicate is selective. */
  test_exec_ok(db, "CREATE INDEX idx_archived ON t(id) WHERE status = 'archived'");
  TEST_ASSERT_MSG(
      query_plan_mentions(db, "EXPLAIN QUERY PLAN SELECT id FROM t WHERE status='archived'",
                          "idx_archived"),
      "planner should choose the partial index for the matching predicate");

  /* An expression index lets the planner use an index for a computed
   * predicate it couldn't otherwise use efficiently. */
  test_exec_ok(db, "CREATE INDEX idx_lower_email ON t(lower(email))");
  TEST_ASSERT(query_plan_mentions(
      db, "EXPLAIN QUERY PLAN SELECT id FROM t WHERE lower(email) = 'user1@example.com'",
      "idx_lower_email"));

  /* A covering index (all selected/filtered columns present in the index
   * itself) lets SQLite skip visiting the underlying table rows. */
  test_exec_ok(db, "CREATE INDEX idx_covering ON t(status, amount)");
  TEST_ASSERT(query_plan_mentions(
      db, "EXPLAIN QUERY PLAN SELECT amount FROM t WHERE status='active'",
      "COVERING INDEX idx_covering"));

  /* likely()/unlikely()/likelihood() are planner hints, not predicates --
   * they don't change results, only (possibly) the chosen plan. */
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(
                  db, "SELECT COUNT(*) FROM t WHERE likely(status = 'active')",
                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 4950);
  sqlite3_finalize(stmt);

  TEST_OK(db, sqlite3_prepare_v2(
                  db, "SELECT COUNT(*) FROM t WHERE unlikely(status = 'archived')",
                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 50);
  sqlite3_finalize(stmt);

  TEST_OK(db, sqlite3_prepare_v2(
                  db, "SELECT COUNT(*) FROM t WHERE likelihood(status = 'active', 0.9)",
                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 4950);
  sqlite3_finalize(stmt);

  test_close_ok(db);
  TEST_PASS("test_index_features");
  return 0;
}
