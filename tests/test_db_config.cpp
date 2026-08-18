#include "test_common.h"

int main(void) {
  sqlite3 *db = test_open_memory_db();

  /* SQLITE_DBCONFIG_DEFENSIVE locks down ways to corrupt/bypass a schema
   * (e.g. writing directly to sqlite_schema). */
  int prev = 0;
  TEST_OK(db, sqlite3_db_config(db, SQLITE_DBCONFIG_DEFENSIVE, 1, &prev));
  int rc = sqlite3_exec(db, "CREATE TABLE t(x)", NULL, NULL, NULL);
  TEST_OK(db, rc);
  rc = sqlite3_exec(db, "DELETE FROM sqlite_schema WHERE name='t'", NULL, NULL, NULL);
  TEST_ASSERT_MSG(rc != SQLITE_OK, "DEFENSIVE mode should block writes to sqlite_schema");
  TEST_OK(db, sqlite3_db_config(db, SQLITE_DBCONFIG_DEFENSIVE, 0, &prev));

  /* SQLITE_DBCONFIG_ENABLE_FKEY: the config-based twin of PRAGMA foreign_keys. */
  int fk_enabled = -1;
  TEST_OK(db, sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_FKEY, 1, &fk_enabled));
  TEST_ASSERT(fk_enabled == 1);
  TEST_OK(db, sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_FKEY, 0, &fk_enabled));
  TEST_ASSERT(fk_enabled == 0);

  /* SQLITE_DBCONFIG_ENABLE_TRIGGER: despite the name, this does not block
   * CREATE TRIGGER itself -- it suppresses *firing* of triggers already
   * defined in the main/attached schemas (TEMP triggers still fire
   * regardless, since SQLite 3.35.0). */
  test_exec_ok(db, "CREATE TABLE trig_log(msg TEXT)");
  test_exec_ok(db, "CREATE TRIGGER trg AFTER INSERT ON t "
                   "BEGIN INSERT INTO trig_log VALUES ('fired'); END");

  int trig_enabled = -1;
  TEST_OK(db, sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_TRIGGER, 0, &trig_enabled));
  TEST_ASSERT(trig_enabled == 0);
  test_exec_ok(db, "INSERT INTO t VALUES (1)");
  sqlite3_stmt *count_stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM trig_log", -1,
                                  &count_stmt, NULL));
  TEST_ASSERT(sqlite3_step(count_stmt) == SQLITE_ROW);
  TEST_ASSERT_MSG(sqlite3_column_int(count_stmt, 0) == 0,
                   "trigger should not have fired while ENABLE_TRIGGER is off");
  sqlite3_finalize(count_stmt);

  TEST_OK(db, sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_TRIGGER, 1, &trig_enabled));
  TEST_ASSERT(trig_enabled == 1);
  test_exec_ok(db, "INSERT INTO t VALUES (2)");
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM trig_log", -1,
                                  &count_stmt, NULL));
  TEST_ASSERT(sqlite3_step(count_stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(count_stmt, 0), 1);
  sqlite3_finalize(count_stmt);

  /* SQLITE_DBCONFIG_DQS_DDL / DQS_DML: toggle the legacy "double-quoted
   * string" misfeature (double quotes silently accepted as a string
   * literal instead of an identifier when no matching column exists). */
  int dqs = -1;
  TEST_OK(db, sqlite3_db_config(db, SQLITE_DBCONFIG_DQS_DML, 0, &dqs));
  rc = sqlite3_exec(db, "SELECT \"not_a_column_or_string\"", NULL, NULL, NULL);
  TEST_ASSERT_MSG(rc != SQLITE_OK,
                   "with DQS_DML off, an unresolvable double-quoted token should be an error");

  /* SQLITE_DBCONFIG_TRUSTED_SCHEMA: default ON; turning it off restricts
   * which SQL functions a stored view/trigger/index-expression may call. */
  int trusted = -1;
  TEST_OK(db, sqlite3_db_config(db, SQLITE_DBCONFIG_TRUSTED_SCHEMA, -1, &trusted));
  TEST_ASSERT(trusted == 0 || trusted == 1);

  /* SQLITE_DBCONFIG_RESET_DATABASE + VACUUM wipes the schema entirely. */
  TEST_OK(db, sqlite3_db_config(db, SQLITE_DBCONFIG_RESET_DATABASE, 1, &prev));
  test_exec_ok(db, "VACUUM");
  TEST_OK(db, sqlite3_db_config(db, SQLITE_DBCONFIG_RESET_DATABASE, 0, &prev));
  rc = sqlite3_exec(db, "SELECT * FROM t", NULL, NULL, NULL);
  TEST_ASSERT_MSG(rc != SQLITE_OK, "RESET_DATABASE + VACUUM should have dropped table t");

  test_close_ok(db);
  TEST_PASS("test_db_config");
  return 0;
}
