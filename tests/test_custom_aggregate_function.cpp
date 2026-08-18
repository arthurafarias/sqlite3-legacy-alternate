#include "test_common.h"

typedef struct {
  double sum;
  sqlite3_int64 count;
} avg_state;

static void myavg_step(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  TEST_ASSERT(argc == 1);
  avg_state *st = (avg_state *)sqlite3_aggregate_context(ctx, sizeof(avg_state));
  TEST_ASSERT(st != NULL);
  st->sum += sqlite3_value_double(argv[0]);
  st->count++;
}

static void myavg_final(sqlite3_context *ctx) {
  /* aggregate_context with size 0 returns the existing state (or NULL if
   * xStep was never called, e.g. an empty group). */
  avg_state *st = (avg_state *)sqlite3_aggregate_context(ctx, 0);
  if (st == NULL || st->count == 0) {
    sqlite3_result_null(ctx);
    return;
  }
  sqlite3_result_double(ctx, st->sum / (double)st->count);
}

/* A window-function-capable aggregate: running max, with xInverse for the
 * sliding window frame. */
static void mymax_step(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  TEST_ASSERT(argc == 1);
  sqlite3_int64 v = sqlite3_value_int64(argv[0]);
  sqlite3_int64 *cur = (sqlite3_int64 *)sqlite3_aggregate_context(ctx, sizeof(sqlite3_int64));
  if (v > *cur)
    *cur = v;
}
static void mymax_final(sqlite3_context *ctx) {
  sqlite3_int64 *cur = (sqlite3_int64 *)sqlite3_aggregate_context(ctx, sizeof(sqlite3_int64));
  sqlite3_result_int64(ctx, *cur);
}
static void mymax_value(sqlite3_context *ctx) { mymax_final(ctx); }
static void mymax_inverse(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  /* Simplified: full recompute isn't tracked; this stub only needs to exist
   * for SQLite to accept the window function registration. */
  (void)ctx;
  (void)argc;
  (void)argv;
}

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE t(g TEXT, v REAL)");
  test_exec_ok(db, "INSERT INTO t VALUES ('a', 1), ('a', 2), ('a', 3), "
                   "('b', 10), ('b', 20)");

  TEST_OK(db, sqlite3_create_function(db, "myavg", 1, SQLITE_UTF8, NULL, NULL,
                                       myavg_step, myavg_final));

  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(
                  db, "SELECT g, myavg(v) FROM t GROUP BY g ORDER BY g", -1,
                  &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 0), "a");
  TEST_ASSERT(sqlite3_column_double(stmt, 1) == 2.0);
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 0), "b");
  TEST_ASSERT(sqlite3_column_double(stmt, 1) == 15.0);
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_DONE);
  sqlite3_finalize(stmt);

  /* Aggregate over an empty group returns NULL, not a crash. */
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT myavg(v) FROM t WHERE 0", -1,
                                  &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT(sqlite3_column_type(stmt, 0) == SQLITE_NULL);
  sqlite3_finalize(stmt);

  /* Register as a full window function (step/final/value/inverse). */
  TEST_OK(db, sqlite3_create_window_function(db, "mymax", 1, SQLITE_UTF8, NULL,
                                              mymax_step, mymax_final,
                                              mymax_value, mymax_inverse, NULL));
  TEST_OK(db, sqlite3_prepare_v2(
                  db,
                  "SELECT v, mymax(v) OVER (ORDER BY v ROWS BETWEEN "
                  "UNBOUNDED PRECEDING AND CURRENT ROW) FROM t WHERE g='a'",
                  -1, &stmt, NULL));
  int rc;
  sqlite3_int64 last_max = 0;
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    sqlite3_int64 m = sqlite3_column_int64(stmt, 1);
    TEST_ASSERT(m >= last_max);
    last_max = m;
  }
  TEST_ASSERT(rc == SQLITE_DONE);
  sqlite3_finalize(stmt);

  test_close_ok(db);
  TEST_PASS("test_custom_aggregate_function");
  return 0;
}
