#include "test_common.h"
#include <ctype.h>

/* Case-insensitive, then length-descending tie-break -- deliberately not
 * something a built-in collation does, so we can prove the custom one ran. */
static int collate_ci_len_desc(void *unused, int lenA, const void *a, int lenB,
                                const void *b) {
  (void)unused;
  const unsigned char *sa = (const unsigned char *)a;
  const unsigned char *sb = (const unsigned char *)b;
  int n = lenA < lenB ? lenA : lenB;
  for (int i = 0; i < n; i++) {
    int ca = tolower(sa[i]);
    int cb = tolower(sb[i]);
    if (ca != cb)
      return ca - cb;
  }
  if (lenA != lenB)
    return lenB - lenA; /* longer sorts first on ties */
  return 0;
}

static int needed_calls = 0;
static void collation_needed(void *unused, sqlite3 *db, int text_rep,
                              const char *name) {
  (void)unused;
  needed_calls++;
  sqlite3_create_collation(db, name, text_rep, NULL, collate_ci_len_desc);
}

int main(void) {
  sqlite3 *db = test_open_memory_db();
  TEST_OK(db, sqlite3_create_collation(db, "CI_LEN", SQLITE_UTF8, NULL,
                                        collate_ci_len_desc));

  test_exec_ok(db, "CREATE TABLE t(name TEXT COLLATE CI_LEN)");
  test_exec_ok(db, "INSERT INTO t VALUES ('bob'), ('Alice'), ('ALICE!'), ('bo')");

  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT name FROM t ORDER BY name", -1,
                                  &stmt, NULL));
  const char *expected[] = {"ALICE!", "Alice", "bob", "bo"};
  for (int i = 0; i < 4; i++) {
    TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
    TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 0), expected[i]);
  }
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_DONE);
  sqlite3_finalize(stmt);

  /* sqlite3_collation_needed: lazily register a collation the first time a
   * query references it by name. */
  TEST_OK(db, sqlite3_collation_needed(db, NULL, collation_needed));
  test_exec_ok(db, "CREATE TABLE u(name TEXT COLLATE LAZY_COLLATION)");
  test_exec_ok(db, "INSERT INTO u VALUES ('z'), ('a')");
  int rc = sqlite3_exec(db, "SELECT name FROM u ORDER BY name", NULL, NULL, NULL);
  TEST_OK(db, rc);
  TEST_ASSERT_MSG(needed_calls >= 1, "collation_needed callback should fire");

  test_close_ok(db);
  TEST_PASS("test_custom_collation");
  return 0;
}
