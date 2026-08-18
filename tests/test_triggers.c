#include "test_common.h"

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE accounts(id INTEGER PRIMARY KEY, balance INTEGER)");
  test_exec_ok(db, "CREATE TABLE audit(id INTEGER PRIMARY KEY AUTOINCREMENT, "
                   "action TEXT, account_id INTEGER, old_balance INTEGER, "
                   "new_balance INTEGER)");
  test_exec_ok(db, "INSERT INTO accounts VALUES (1, 100), (2, 50)");

  /* AFTER INSERT trigger */
  test_exec_ok(db, "CREATE TRIGGER trg_after_insert AFTER INSERT ON accounts "
                   "BEGIN "
                   "  INSERT INTO audit(action, account_id, new_balance) "
                   "  VALUES ('insert', NEW.id, NEW.balance); "
                   "END");
  test_exec_ok(db, "INSERT INTO accounts VALUES (3, 200)");

  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM audit WHERE action='insert'",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 1);
  sqlite3_finalize(stmt);

  /* BEFORE UPDATE trigger with a WHEN clause and OLD/NEW references,
   * enforcing a business rule (no negative balances) by raising an error. */
  test_exec_ok(db, "CREATE TRIGGER trg_before_update BEFORE UPDATE OF balance "
                   "ON accounts WHEN NEW.balance < 0 "
                   "BEGIN "
                   "  SELECT RAISE(ABORT, 'balance cannot go negative'); "
                   "END");
  int rc = sqlite3_exec(db, "UPDATE accounts SET balance = -5 WHERE id = 1",
                         NULL, NULL, NULL);
  TEST_ASSERT_MSG(rc == SQLITE_CONSTRAINT, "RAISE(ABORT,...) should map to SQLITE_CONSTRAINT, got %d", rc);

  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT balance FROM accounts WHERE id=1",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 100); /* unchanged */
  sqlite3_finalize(stmt);

  /* AFTER UPDATE trigger that logs the transition. */
  test_exec_ok(db, "CREATE TRIGGER trg_after_update AFTER UPDATE OF balance "
                   "ON accounts "
                   "BEGIN "
                   "  INSERT INTO audit(action, account_id, old_balance, new_balance) "
                   "  VALUES ('update', OLD.id, OLD.balance, NEW.balance); "
                   "END");
  test_exec_ok(db, "UPDATE accounts SET balance = balance + 10 WHERE id = 1");
  TEST_OK(db, sqlite3_prepare_v2(
                  db, "SELECT old_balance, new_balance FROM audit WHERE action='update'",
                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 100);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 1), 110);
  sqlite3_finalize(stmt);

  /* AFTER DELETE trigger, and recursive_triggers pragma. */
  test_exec_ok(db, "PRAGMA recursive_triggers = ON");
  test_exec_ok(db, "CREATE TRIGGER trg_after_delete AFTER DELETE ON accounts "
                   "BEGIN "
                   "  INSERT INTO audit(action, account_id, old_balance) "
                   "  VALUES ('delete', OLD.id, OLD.balance); "
                   "END");
  test_exec_ok(db, "DELETE FROM accounts WHERE id = 2");
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM audit WHERE action='delete'",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 1);
  sqlite3_finalize(stmt);

  /* DROP TRIGGER removes it. */
  test_exec_ok(db, "DROP TRIGGER trg_after_delete");
  test_exec_ok(db, "DELETE FROM accounts WHERE id = 3");
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM audit WHERE action='delete'",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 1); /* unchanged */
  sqlite3_finalize(stmt);

  test_close_ok(db);
  TEST_PASS("test_triggers");
  return 0;
}
