#include "test_common.h"

static void square_func(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  TEST_ASSERT(argc == 1);
  if (sqlite3_value_type(argv[0]) == SQLITE_NULL) {
    sqlite3_result_null(ctx);
    return;
  }
  double v = sqlite3_value_double(argv[0]);
  sqlite3_result_double(ctx, v * v);
}

static void concat_with_app_data(sqlite3_context *ctx, int argc,
                                  sqlite3_value **argv) {
  const char *sep = (const char *)sqlite3_user_data(ctx);
  TEST_ASSERT(argc == 2);
  const unsigned char *a = sqlite3_value_text(argv[0]);
  const unsigned char *b = sqlite3_value_text(argv[1]);
  char buf[256];
  snprintf(buf, sizeof(buf), "%s%s%s", a ? (const char *)a : "", sep,
           b ? (const char *)b : "");
  sqlite3_result_text(ctx, buf, -1, SQLITE_TRANSIENT);
}

static int destructor_calls = 0;
static void free_sep(void *p) {
  destructor_calls++;
  free(p);
}

int main(void) {
  sqlite3 *db = test_open_memory_db();

  TEST_OK(db, sqlite3_create_function(db, "square", 1,
                                       SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL,
                                       square_func, NULL, NULL));

  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT square(7), square(NULL)", -1,
                                  &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT(sqlite3_column_double(stmt, 0) == 49.0);
  TEST_ASSERT(sqlite3_column_type(stmt, 1) == SQLITE_NULL);
  sqlite3_finalize(stmt);

  /* Application data + destructor lifecycle. */
  char *sep = strdup(" | ");
  TEST_OK(db, sqlite3_create_function_v2(db, "joinsep", 2, SQLITE_UTF8, sep,
                                          concat_with_app_data, NULL, NULL,
                                          free_sep));
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT joinsep('a', 'b')", -1, &stmt,
                                  NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 0), "a | b");
  sqlite3_finalize(stmt);

  test_close_ok(db);
  TEST_ASSERT_MSG(destructor_calls == 1,
                   "user-data destructor should run exactly once, ran %d times",
                   destructor_calls);

  TEST_PASS("test_custom_scalar_function");
  return 0;
}
