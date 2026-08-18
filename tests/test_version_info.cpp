#include "test_common.h"

int main(void) {
  TEST_ASSERT(sqlite3_libversion() != NULL);
  TEST_ASSERT(strcmp(sqlite3_libversion(), SQLITE_VERSION) == 0);
  TEST_ASSERT(sqlite3_libversion_number() == SQLITE_VERSION_NUMBER);
  TEST_ASSERT(sqlite3_sourceid() != NULL);
  TEST_ASSERT(sqlite3_threadsafe() != 0);

  /* This build has no FTS5 or RTREE (see docs/architecture.md: this
   * project decomposes a specific preprocessed amalgamation, and neither
   * module's core engine made it into src/sqlite/ -- only auxiliary
   * struct declarations did). ENABLE_MATH_FUNCTIONS is the one optional
   * feature actually compiled in that's worth asserting on here. */
  int found_math = 0;
  for (int i = 0;; i++) {
    const char *opt = sqlite3_compileoption_get(i);
    if (!opt)
      break;
    if (strstr(opt, "ENABLE_MATH_FUNCTIONS"))
      found_math = 1;
  }
  TEST_ASSERT_MSG(found_math, "MATH_FUNCTIONS should be compiled in");
  TEST_ASSERT(sqlite3_compileoption_used("ENABLE_MATH_FUNCTIONS"));
  TEST_ASSERT(!sqlite3_compileoption_used("THIS_OPTION_DOES_NOT_EXIST"));

  sqlite3 *db = test_open_memory_db();
  int major = 0, minor = 0, patch = 0;
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT sqlite_version()", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  sscanf((const char *)sqlite3_column_text(stmt, 0), "%d.%d.%d", &major, &minor, &patch);
  TEST_ASSERT(major >= 3);
  sqlite3_finalize(stmt);
  test_close_ok(db);

  TEST_PASS("test_version_info");
  return 0;
}
