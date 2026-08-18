#include "test_common.h"

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE orders(id INTEGER PRIMARY KEY, customer TEXT, "
                   "amount REAL, status TEXT)");
  test_exec_ok(db, "INSERT INTO orders VALUES "
                   "(1, 'alice', 50.0, 'open'), "
                   "(2, 'alice', 25.0, 'closed'), "
                   "(3, 'bob', 75.0, 'open')");

  /* A plain (read-only) view. */
  test_exec_ok(db, "CREATE VIEW open_orders AS "
                   "SELECT id, customer, amount FROM orders WHERE status='open'");
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*), SUM(amount) FROM open_orders",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 2);
  TEST_ASSERT(sqlite3_column_double(stmt, 1) == 125.0);
  sqlite3_finalize(stmt);

  /* A view is not directly writable... */
  int rc = sqlite3_exec(db, "INSERT INTO open_orders VALUES (4, 'eve', 10.0)",
                         NULL, NULL, NULL);
  TEST_ASSERT(rc != SQLITE_OK);

  /* ...unless it has an INSTEAD OF trigger that redirects the write. */
  test_exec_ok(db, "CREATE TRIGGER trg_insert_open_order "
                   "INSTEAD OF INSERT ON open_orders "
                   "BEGIN "
                   "  INSERT INTO orders(id, customer, amount, status) "
                   "  VALUES (NEW.id, NEW.customer, NEW.amount, 'open'); "
                   "END");
  test_exec_ok(db, "INSERT INTO open_orders VALUES (4, 'eve', 10.0)");
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM orders WHERE id=4 AND status='open'",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 1);
  sqlite3_finalize(stmt);

  /* A view over a GROUP BY aggregate, queried through a join. */
  test_exec_ok(db, "CREATE VIEW customer_totals AS "
                   "SELECT customer, SUM(amount) AS total FROM orders GROUP BY customer");
  TEST_OK(db, sqlite3_prepare_v2(
                  db, "SELECT customer, total FROM customer_totals ORDER BY customer",
                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 0), "alice");
  TEST_ASSERT(sqlite3_column_double(stmt, 1) == 75.0);
  sqlite3_finalize(stmt);

  test_exec_ok(db, "DROP VIEW customer_totals");
  rc = sqlite3_exec(db, "SELECT * FROM customer_totals", NULL, NULL, NULL);
  TEST_ASSERT(rc != SQLITE_OK);

  test_close_ok(db);
  TEST_PASS("test_views");
  return 0;
}
