#include "test_common.h"

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE t(id INTEGER PRIMARY KEY, category TEXT, v INTEGER)");
  test_exec_ok(db, "CREATE INDEX idx_category ON t(category)");
  test_exec_ok(db, "WITH RECURSIVE seq(n) AS "
                   "(SELECT 1 UNION ALL SELECT n+1 FROM seq WHERE n < 2000) "
                   "INSERT INTO t SELECT n, 'cat' || (n % 10), n FROM seq");
  test_exec_ok(db, "ANALYZE");

  /* EXPLAIN QUERY PLAN describes how the optimizer will run a query, as an
   * ordinary result set (id, parent, notused, detail). A query filtering on
   * the indexed column should mention the index by name. */
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(
                  db, "EXPLAIN QUERY PLAN SELECT * FROM t WHERE category = 'cat3'",
                  -1, &stmt, NULL));
  int found_index_scan = 0;
  int rows = 0;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    rows++;
    const char *detail = (const char *)sqlite3_column_text(stmt, 3);
    if (strstr(detail, "idx_category") != NULL)
      found_index_scan = 1;
  }
  TEST_ASSERT(rows > 0);
  TEST_ASSERT_MSG(found_index_scan,
                   "expected the query plan to mention idx_category");
  sqlite3_finalize(stmt);

  /* Plain EXPLAIN dumps the raw VDBE bytecode program -- a much longer,
   * lower-level result set (addr, opcode, p1..p5, comment). */
  TEST_OK(db, sqlite3_prepare_v2(db, "EXPLAIN SELECT * FROM t WHERE category = 'cat3'",
                                  -1, &stmt, NULL));
  int opcode_rows = 0;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    opcode_rows++;
    TEST_ASSERT(sqlite3_column_text(stmt, 1) != NULL); /* opcode name */
  }
  TEST_ASSERT_MSG(opcode_rows > rows,
                   "raw bytecode listing should have far more rows than the query plan");
  sqlite3_finalize(stmt);

  test_close_ok(db);
  TEST_PASS("test_explain_query_plan");
  return 0;
}
