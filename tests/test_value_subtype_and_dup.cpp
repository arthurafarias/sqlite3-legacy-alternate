#include "test_common.h"

#define MY_SUBTYPE 77

/* A function that tags its result with a custom subtype -- the mechanism
 * JSON1 itself uses internally so that json_extract() output can be told
 * apart from plain text that merely looks like JSON. */
static void tag_func(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  TEST_ASSERT(argc == 1);
  sqlite3_result_value(ctx, argv[0]);
  sqlite3_result_subtype(ctx, MY_SUBTYPE);
}

/* A function that reports whether its argument carries that subtype. */
static void has_tag_func(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  TEST_ASSERT(argc == 1);
  sqlite3_result_int(ctx, sqlite3_value_subtype(argv[0]) == MY_SUBTYPE);
}

int main(void) {
  sqlite3 *db = test_open_memory_db();
  TEST_OK(db, sqlite3_create_function(db, "tag", 1, SQLITE_UTF8, NULL, tag_func,
                                       NULL, NULL));
  TEST_OK(db, sqlite3_create_function(db, "has_tag", 1, SQLITE_UTF8, NULL,
                                       has_tag_func, NULL, NULL));

  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT has_tag(tag('x')), has_tag('x')",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 1); /* tagged */
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 1), 0); /* plain value, no subtype */
  sqlite3_finalize(stmt);

  /* Subtypes are a scalar-function-boundary concept only -- storing a
   * tagged value in a table and reading it back loses the tag. */
  test_exec_ok(db, "CREATE TABLE t(v)");
  TEST_OK(db, sqlite3_prepare_v2(db, "INSERT INTO t VALUES (tag('x'))", -1,
                                  &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_DONE);
  sqlite3_finalize(stmt);
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT has_tag(v) FROM t", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 0);
  sqlite3_finalize(stmt);

  /* JSON1's own use of subtypes: json_extract()'s result is tagged so that
   * json_array()/json_object() nest it as JSON rather than re-quoting it
   * as a string when it's used as an argument to another JSON function. */
  TEST_OK(db, sqlite3_prepare_v2(
                  db,
                  "SELECT json_array(json_extract('{\"a\":[1,2]}','$.a')), "
                  "       json_array('[1,2]')",
                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 0), "[[1,2]]");
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 1), "[\"[1,2]\"]");
  sqlite3_finalize(stmt);

  /* sqlite3_value_dup / sqlite3_value_free: take an independent, owned copy
   * of a protected sqlite3_value (e.g. to outlive the current callback). */
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT 42", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  sqlite3_value *original = sqlite3_column_value(stmt, 0);
  sqlite3_value *copy = sqlite3_value_dup(original);
  TEST_ASSERT(copy != NULL);
  TEST_ASSERT_EQ_INT(sqlite3_value_int(copy), 42);
  sqlite3_finalize(stmt); /* `original` is now invalid; `copy` still stands alone */
  TEST_ASSERT_EQ_INT(sqlite3_value_int(copy), 42);
  sqlite3_value_free(copy);

  test_close_ok(db);
  TEST_PASS("test_value_subtype_and_dup");
  return 0;
}
