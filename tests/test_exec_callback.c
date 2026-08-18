#include "test_common.h"

typedef struct {
  int rows;
  int cols_seen;
  long long sum;
} exec_state;

static int sum_callback(void *ctx, int argc, char **argv, char **colnames) {
  exec_state *st = (exec_state *)ctx;
  st->rows++;
  st->cols_seen = argc;
  for (int i = 0; i < argc; i++) {
    if (argv[i])
      st->sum += atoll(argv[i]);
  }
  (void)colnames;
  return 0;
}

static int abort_callback(void *ctx, int argc, char **argv, char **colnames) {
  (void)ctx;
  (void)argc;
  (void)argv;
  (void)colnames;
  return 1; /* non-zero aborts the query with SQLITE_ABORT */
}

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE nums(v INTEGER)");
  test_exec_ok(db, "INSERT INTO nums VALUES (1),(2),(3),(4),(5)");

  exec_state st = {0};
  int rc = sqlite3_exec(db, "SELECT v FROM nums ORDER BY v", sum_callback, &st,
                         NULL);
  TEST_OK(db, rc);
  TEST_ASSERT_EQ_INT(st.rows, 5);
  TEST_ASSERT_EQ_INT(st.cols_seen, 1);
  TEST_ASSERT_EQ_INT(st.sum, 15);

  /* A NULL callback simply discards result rows. */
  rc = sqlite3_exec(db, "SELECT v FROM nums", NULL, NULL, NULL);
  TEST_OK(db, rc);

  /* A callback returning non-zero aborts the statement. */
  rc = sqlite3_exec(db, "SELECT v FROM nums", abort_callback, NULL, NULL);
  TEST_ASSERT_MSG(rc == SQLITE_ABORT,
                   "aborting callback should yield SQLITE_ABORT, got %d", rc);

  /* Malformed SQL surfaces a message through errmsg. */
  char *errmsg = NULL;
  rc = sqlite3_exec(db, "SELEC 1", NULL, NULL, &errmsg);
  TEST_ASSERT(rc != SQLITE_OK);
  TEST_ASSERT(errmsg != NULL);
  sqlite3_free(errmsg);

  /* Multi-statement batches execute in order. */
  rc = sqlite3_exec(db,
                     "INSERT INTO nums VALUES (6); INSERT INTO nums VALUES (7);",
                     NULL, NULL, NULL);
  TEST_OK(db, rc);
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM nums", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 7);
  sqlite3_finalize(stmt);

  test_close_ok(db);
  TEST_PASS("test_exec_callback");
  return 0;
}
