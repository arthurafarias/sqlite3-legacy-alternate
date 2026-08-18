#include "test_common.h"

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE t(id INTEGER PRIMARY KEY, name TEXT)");

  /* sqlite3_sql / sqlite3_expanded_sql: get back the statement text, with
   * expanded_sql substituting bound parameter values into the string. */
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT * FROM t WHERE name = ?", -1,
                                  &stmt, NULL));
  sqlite3_bind_text(stmt, 1, "alice", -1, SQLITE_STATIC);

  const char *raw_sql = sqlite3_sql(stmt);
  TEST_ASSERT_EQ_STR(raw_sql, "SELECT * FROM t WHERE name = ?");

  char *expanded = sqlite3_expanded_sql(stmt);
  TEST_ASSERT_MSG(expanded != NULL && strstr(expanded, "'alice'") != NULL,
                   "expanded_sql should substitute the bound value, got: %s",
                   expanded ? expanded : "(null)");
  sqlite3_free(expanded);

  /* sqlite3_normalized_sql (SQLITE_ENABLE_NORMALIZE) isn't part of this
   * build, so normalization coverage stops at expanded_sql above. */
  sqlite3_finalize(stmt);

  /* sqlite3_complete: syntactic (not semantic) check for a full statement. */
  TEST_ASSERT(sqlite3_complete("SELECT 1;"));
  TEST_ASSERT(!sqlite3_complete("SELECT 1"));      /* missing terminator */
  TEST_ASSERT(!sqlite3_complete("SELECT * FROM ")); /* incomplete */
  TEST_ASSERT(sqlite3_complete("CREATE TABLE x(a); INSERT INTO x VALUES(1);"));

  /* sqlite3_keyword_count / keyword_name / keyword_check. */
  int nkw = sqlite3_keyword_count();
  TEST_ASSERT(nkw > 50); /* SQLite has hundreds of reserved keywords */
  const char *first_kw = NULL;
  int first_len = 0;
  TEST_ASSERT(sqlite3_keyword_name(0, &first_kw, &first_len) == SQLITE_OK);
  TEST_ASSERT(first_kw != NULL && first_len > 0);
  TEST_ASSERT(sqlite3_keyword_check("SELECT", 6));
  TEST_ASSERT(sqlite3_keyword_check("select", 6)); /* case-insensitive */
  TEST_ASSERT(!sqlite3_keyword_check("not_a_keyword_xyz", 17));

  /* sqlite3_stricmp / strnicmp: ASCII case-insensitive comparison helpers
   * exposed for extension authors. */
  TEST_ASSERT(sqlite3_stricmp("Hello", "HELLO") == 0);
  TEST_ASSERT(sqlite3_stricmp("Hello", "World") != 0);
  TEST_ASSERT(sqlite3_strnicmp("HelloWorld", "hello!!!", 5) == 0);

  test_close_ok(db);
  TEST_PASS("test_introspection");
  return 0;
}
