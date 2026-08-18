#include "test_common.h"

static sqlite3 *g_interrupt_target = NULL;
static int interrupt_once_handler(void *ctx) {
  (void)ctx;
  /* sqlite3_interrupt() is a no-op unless called while a statement is
   * actually running -- fire it from inside the progress handler, which
   * only runs mid-VDBE-execution, so the flag genuinely takes effect. */
  sqlite3_interrupt(g_interrupt_target);
  return 0; /* the interrupt flag itself will stop the query, not this */
}

static int auth_denied_deletes = 0;
static int authorizer(void *ctx, int action, const char *arg1, const char *arg2,
                       const char *db_name, const char *trigger_name) {
  (void)ctx;
  (void)arg2;
  (void)db_name;
  (void)trigger_name;
  if (action == SQLITE_DELETE && arg1 && strcmp(arg1, "protected") == 0) {
    auth_denied_deletes++;
    return SQLITE_DENY;
  }
  if (action == SQLITE_READ && arg1 && strcmp(arg1, "secret") == 0 && arg2 &&
      strcmp(arg2, "hidden_col") == 0) {
    return SQLITE_IGNORE; /* column reads as NULL instead of erroring */
  }
  return SQLITE_OK;
}

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE protected(id INTEGER PRIMARY KEY, v TEXT)");
  test_exec_ok(db, "CREATE TABLE secret(id INTEGER PRIMARY KEY, hidden_col TEXT, "
                   "visible_col TEXT)");
  test_exec_ok(db, "INSERT INTO protected VALUES (1, 'x')");
  test_exec_ok(db, "INSERT INTO secret VALUES (1, 'classified', 'public')");

  TEST_OK(db, sqlite3_set_authorizer(db, authorizer, NULL));

  /* SQLITE_DENY on DELETE fails the whole statement with SQLITE_AUTH. */
  int rc = sqlite3_exec(db, "DELETE FROM protected WHERE id=1", NULL, NULL, NULL);
  TEST_ASSERT_MSG(rc == SQLITE_AUTH, "expected SQLITE_AUTH, got %d", rc);
  TEST_ASSERT(auth_denied_deletes == 1);

  /* SQLITE_IGNORE on a column read silently substitutes NULL. */
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT hidden_col, visible_col FROM secret",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT(sqlite3_column_type(stmt, 0) == SQLITE_NULL);
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 1), "public");
  sqlite3_finalize(stmt);

  /* Unregistering (NULL) restores normal behavior. */
  TEST_OK(db, sqlite3_set_authorizer(db, NULL, NULL));
  test_exec_ok(db, "DELETE FROM protected WHERE id=1");

  /* sqlite3_limit: get-or-set compile-time-bounded runtime limits. */
  int prev_limit = sqlite3_limit(db, SQLITE_LIMIT_LENGTH, -1);
  TEST_ASSERT(prev_limit > 0);
  int new_limit = sqlite3_limit(db, SQLITE_LIMIT_LENGTH, 1000);
  TEST_ASSERT_EQ_INT(new_limit, prev_limit); /* returns the *previous* value */
  TEST_ASSERT_EQ_INT(sqlite3_limit(db, SQLITE_LIMIT_LENGTH, -1), 1000);

  rc = sqlite3_exec(db, "SELECT hex(zeroblob(2000))", NULL, NULL, NULL);
  TEST_ASSERT_MSG(rc == SQLITE_TOOBIG, "value exceeding SQLITE_LIMIT_LENGTH should fail, got %d", rc);
  sqlite3_limit(db, SQLITE_LIMIT_LENGTH, prev_limit); /* restore */

  /* sqlite3_interrupt aborts any statements currently running on this
   * connection with SQLITE_INTERRUPT. Calling it with nothing running is a
   * documented no-op, so it must be triggered from inside a callback that
   * only fires mid-execution (here, the progress handler). */
  TEST_ASSERT(!sqlite3_is_interrupted(db));
  g_interrupt_target = db;
  sqlite3_progress_handler(db, 50, interrupt_once_handler, NULL);
  rc = sqlite3_exec(db,
                     "WITH RECURSIVE seq(v) AS "
                     "(SELECT 1 UNION ALL SELECT v+1 FROM seq WHERE v < 1000000) "
                     "SELECT COUNT(*) FROM seq",
                     NULL, NULL, NULL);
  TEST_ASSERT_MSG(rc == SQLITE_INTERRUPT, "expected SQLITE_INTERRUPT, got %d", rc);
  TEST_ASSERT(sqlite3_is_interrupted(db));
  sqlite3_progress_handler(db, 0, NULL, NULL);

  /* The flag stays in effect until the running-statement count reaches
   * zero -- which it now has, so a fresh statement is unaffected. */
  rc = sqlite3_exec(db, "SELECT 1", NULL, NULL, NULL);
  TEST_ASSERT_MSG(rc == SQLITE_OK, "interrupt should not leak into a later statement, got %d", rc);

  test_close_ok(db);
  TEST_PASS("test_authorizer_and_limits");
  return 0;
}
