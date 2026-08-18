#include "test_common.h"

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE sales(id INTEGER PRIMARY KEY, rep TEXT, amount INTEGER)");
  test_exec_ok(db, "INSERT INTO sales(rep, amount) VALUES "
                   "('alice', 100), ('alice', 200), ('alice', 150), "
                   "('bob', 300), ('bob', 50)");

  /* Set operations: UNION dedups, UNION ALL doesn't, INTERSECT/EXCEPT
   * combine two row sets. */
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(
                  db, "SELECT rep FROM sales UNION SELECT rep FROM sales", -1,
                  &stmt, NULL));
  int n = 0;
  while (sqlite3_step(stmt) == SQLITE_ROW)
    n++;
  TEST_ASSERT_EQ_INT(n, 2); /* deduped to alice/bob */
  sqlite3_finalize(stmt);

  TEST_OK(db, sqlite3_prepare_v2(
                  db, "SELECT rep FROM sales UNION ALL SELECT rep FROM sales", -1,
                  &stmt, NULL));
  n = 0;
  while (sqlite3_step(stmt) == SQLITE_ROW)
    n++;
  TEST_ASSERT_EQ_INT(n, 10); /* not deduped */
  sqlite3_finalize(stmt);

  TEST_OK(db, sqlite3_prepare_v2(
                  db,
                  "SELECT rep FROM sales WHERE amount > 100 "
                  "INTERSECT SELECT rep FROM sales WHERE amount < 100",
                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW); /* bob has both a >100 and a <100 row */
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 0), "bob");
  sqlite3_finalize(stmt);

  TEST_OK(db, sqlite3_prepare_v2(
                  db, "SELECT DISTINCT rep FROM sales EXCEPT SELECT 'bob'", -1,
                  &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 0), "alice");
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_DONE);
  sqlite3_finalize(stmt);

  /* A recursive CTE with the MATERIALIZED hint (a query-planner directive,
   * not a semantic change) computing a running Fibonacci-like sequence. */
  TEST_OK(db, sqlite3_prepare_v2(
                  db,
                  "WITH RECURSIVE fib(a, b) AS MATERIALIZED "
                  "(SELECT 0, 1 UNION ALL SELECT b, a+b FROM fib WHERE b < 50) "
                  "SELECT a FROM fib ORDER BY a",
                  -1, &stmt, NULL));
  int last = -1, count = 0;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    int v = sqlite3_column_int(stmt, 0);
    TEST_ASSERT(v >= last);
    last = v;
    count++;
  }
  TEST_ASSERT(count >= 5);
  sqlite3_finalize(stmt);

  /* Window functions: PARTITION BY + built-ins (ROW_NUMBER, RANK, LAG). */
  TEST_OK(db, sqlite3_prepare_v2(
                  db,
                  "SELECT rep, amount, "
                  "ROW_NUMBER() OVER (PARTITION BY rep ORDER BY amount DESC) AS rn, "
                  "LAG(amount) OVER (PARTITION BY rep ORDER BY amount) AS prev "
                  "FROM sales ORDER BY rep, amount",
                  -1, &stmt, NULL));
  int rows = 0;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    rows++;
    TEST_ASSERT(sqlite3_column_int(stmt, 2) >= 1); /* rn */
  }
  TEST_ASSERT_EQ_INT(rows, 5);
  sqlite3_finalize(stmt);

  /* A RANGE frame with explicit bounds, computing a moving sum. */
  TEST_OK(db, sqlite3_prepare_v2(
                  db,
                  "SELECT amount, SUM(amount) OVER "
                  "(ORDER BY amount ROWS BETWEEN 1 PRECEDING AND 1 FOLLOWING) "
                  "FROM sales ORDER BY amount",
                  -1, &stmt, NULL));
  rows = 0;
  while (sqlite3_step(stmt) == SQLITE_ROW)
    rows++;
  TEST_ASSERT_EQ_INT(rows, 5);
  sqlite3_finalize(stmt);

  test_close_ok(db);
  TEST_PASS("test_cte_setops_window");
  return 0;
}
