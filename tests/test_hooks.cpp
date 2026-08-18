#include "test_common.h"

static int trace_calls = 0;
static int trace_callback(unsigned type, void *ctx, void *p, void *x) {
  (void)ctx;
  (void)p;
  (void)x;
  if (type == SQLITE_TRACE_STMT)
    trace_calls++;
  return 0;
}

typedef struct {
  int inserts, updates, deletes;
  char last_table[64];
} update_state;

static void update_callback(void *ctx, int op, char const *db, char const *tbl,
                             sqlite3_int64 rowid) {
  (void)db;
  (void)rowid;
  update_state *st = (update_state *)ctx;
  strncpy(st->last_table, tbl, sizeof(st->last_table) - 1);
  if (op == SQLITE_INSERT)
    st->inserts++;
  else if (op == SQLITE_UPDATE)
    st->updates++;
  else if (op == SQLITE_DELETE)
    st->deletes++;
}

static int commit_calls = 0;
static int commit_hook(void *ctx) {
  (void)ctx;
  commit_calls++;
  return 0; /* 0 = allow the commit */
}

static int rollback_calls = 0;
static void rollback_hook(void *ctx) {
  (void)ctx;
  rollback_calls++;
}

int main(void) {
  sqlite3 *db = test_open_memory_db();

  TEST_ASSERT(sqlite3_trace_v2(db, SQLITE_TRACE_STMT, trace_callback, NULL) ==
              SQLITE_OK);

  test_exec_ok(db, "CREATE TABLE t(x)");
  TEST_ASSERT(trace_calls > 0);

  /* Registered after the schema DDL so commit_calls below counts only the
   * data-changing statements that follow, not the CREATE TABLE's own
   * implicit autocommit transaction. */
  update_state ust = {0};
  sqlite3_update_hook(db, update_callback, &ust);
  sqlite3_commit_hook(db, commit_hook, NULL);
  sqlite3_rollback_hook(db, rollback_hook, NULL);

  test_exec_ok(db, "INSERT INTO t VALUES (1)");
  TEST_ASSERT_EQ_INT(ust.inserts, 1);
  TEST_ASSERT_EQ_STR(ust.last_table, "t");
  TEST_ASSERT_EQ_INT(commit_calls, 1); /* autocommit insert = implicit commit */

  test_exec_ok(db, "UPDATE t SET x = 2 WHERE x = 1");
  TEST_ASSERT_EQ_INT(ust.updates, 1);

  test_exec_ok(db, "DELETE FROM t WHERE x = 2");
  TEST_ASSERT_EQ_INT(ust.deletes, 1);

  int commits_before = commit_calls;
  test_exec_ok(db, "BEGIN");
  test_exec_ok(db, "INSERT INTO t VALUES (99)");
  test_exec_ok(db, "ROLLBACK");
  TEST_ASSERT_EQ_INT(rollback_calls, 1);
  TEST_ASSERT_EQ_INT(commit_calls, commits_before); /* no commit fired */

  /* Unregister by passing NULL. Note ust.inserts is 2 here, not 1: the
   * update_hook fires for the INSERT VALUES(99) above too, even though
   * that transaction was later rolled back -- the hook reports the write
   * as it happens, with no corresponding "undo" callback on rollback. */
  int inserts_before = ust.inserts;
  sqlite3_update_hook(db, NULL, NULL);
  test_exec_ok(db, "INSERT INTO t VALUES (5)");
  TEST_ASSERT_EQ_INT(ust.inserts, inserts_before); /* unchanged: hook was cleared */

  test_close_ok(db);
  TEST_PASS("test_hooks");
  return 0;
}
