#include "test_common.h"
#include <ctype.h>

/* sqlite3_result_error* : the ways a UDF can fail a query. */
static void fail_generic(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  (void)argc;
  (void)argv;
  sqlite3_result_error(ctx, "custom failure message", -1);
}
static void fail_with_code(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  (void)argc;
  (void)argv;
  sqlite3_result_error_code(ctx, SQLITE_CONSTRAINT);
}
static void fail_toobig(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  (void)argc;
  (void)argv;
  sqlite3_result_error_toobig(ctx);
}
static void fail_nomem(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  (void)argc;
  (void)argv;
  sqlite3_result_error_nomem(ctx);
}

/* sqlite3_get_auxdata/set_auxdata: per-invocation-site caching, keyed by
 * argument index -- the mechanism SQLite's own LIKE/GLOB use internally to
 * avoid recompiling the same pattern on every row of a scan. */
static int g_recompute_count = 0;
static void free_marker(void *p) { sqlite3_free(p); }
static void cached_upper(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  TEST_ASSERT(argc == 1);
  char *cached = (char *)sqlite3_get_auxdata(ctx, 0);
  if (cached == NULL) {
    g_recompute_count++;
    const unsigned char *in = sqlite3_value_text(argv[0]);
    size_t n = in ? strlen((const char *)in) : 0;
    cached = sqlite3_malloc((int)n + 1);
    for (size_t i = 0; i < n; i++)
      cached[i] = (char)toupper(in[i]);
    cached[n] = '\0';
    sqlite3_set_auxdata(ctx, 0, cached, free_marker);
  }
  sqlite3_result_text(ctx, cached, -1, SQLITE_TRANSIENT);
}

/* sqlite3_context_db_handle: a UDF can reach back to its own connection. */
static void whoami(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  (void)argc;
  (void)argv;
  sqlite3 *db = sqlite3_context_db_handle(ctx);
  sqlite3_result_text(ctx, sqlite3_db_filename(db, "main"), -1, SQLITE_TRANSIENT);
}

int main(void) {
  sqlite3 *db = test_open_memory_db();
  TEST_OK(db, sqlite3_create_function(db, "fail_generic", 1, SQLITE_UTF8, NULL,
                                       fail_generic, NULL, NULL));
  TEST_OK(db, sqlite3_create_function(db, "fail_with_code", 1, SQLITE_UTF8, NULL,
                                       fail_with_code, NULL, NULL));
  TEST_OK(db, sqlite3_create_function(db, "fail_toobig", 1, SQLITE_UTF8, NULL,
                                       fail_toobig, NULL, NULL));
  TEST_OK(db, sqlite3_create_function(db, "fail_nomem", 1, SQLITE_UTF8, NULL,
                                       fail_nomem, NULL, NULL));
  TEST_OK(db, sqlite3_create_function(db, "cached_upper", 1, SQLITE_UTF8, NULL,
                                       cached_upper, NULL, NULL));
  TEST_OK(db, sqlite3_create_function(db, "whoami", 0, SQLITE_UTF8, NULL, whoami,
                                       NULL, NULL));

  int rc = sqlite3_exec(db, "SELECT fail_generic(1)", NULL, NULL, NULL);
  TEST_ASSERT(rc == SQLITE_ERROR);
  TEST_ASSERT(strstr(sqlite3_errmsg(db), "custom failure message") != NULL);

  rc = sqlite3_exec(db, "SELECT fail_with_code(1)", NULL, NULL, NULL);
  TEST_ASSERT(rc == SQLITE_CONSTRAINT);

  rc = sqlite3_exec(db, "SELECT fail_toobig(1)", NULL, NULL, NULL);
  TEST_ASSERT(rc == SQLITE_TOOBIG);

  rc = sqlite3_exec(db, "SELECT fail_nomem(1)", NULL, NULL, NULL);
  TEST_ASSERT(rc == SQLITE_NOMEM);

  /* cached_upper() is invoked once per row but should only *recompute* its
   * cached value once per distinct argument value, thanks to auxdata. */
  test_exec_ok(db, "CREATE TABLE t(v TEXT)");
  test_exec_ok(db, "INSERT INTO t VALUES ('same'), ('same'), ('same')");
  g_recompute_count = 0;
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT cached_upper('literal') FROM t", -1,
                                  &stmt, NULL));
  int rows = 0;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    rows++;
    TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 0), "LITERAL");
  }
  TEST_ASSERT_EQ_INT(rows, 3);
  TEST_ASSERT_MSG(g_recompute_count == 1,
                   "auxdata should cache across rows for a constant argument, recomputed %d times",
                   g_recompute_count);
  sqlite3_finalize(stmt);

  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT whoami()", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 0), ""); /* :memory: */
  sqlite3_finalize(stmt);

  test_close_ok(db);
  TEST_PASS("test_udf_error_reporting_and_auxdata");
  return 0;
}
