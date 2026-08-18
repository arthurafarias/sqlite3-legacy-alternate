#include "test_common.h"

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE t(id INTEGER PRIMARY KEY, name TEXT)");
  test_exec_ok(db, "INSERT INTO t VALUES (1, 'alice'), (2, 'bob')");

  /* sqlite3_get_table(): the pre-prepared-statement convenience API that
   * buffers an entire result (header row + data) into one flat array. */
  char **result = NULL;
  int nrow = 0, ncol = 0;
  char *errmsg = NULL;
  int rc = sqlite3_get_table(db, "SELECT id, name FROM t ORDER BY id", &result,
                              &nrow, &ncol, &errmsg);
  TEST_ASSERT_MSG(rc == SQLITE_OK, "get_table failed: %s", errmsg ? errmsg : "?");
  TEST_ASSERT_EQ_INT(nrow, 2);
  TEST_ASSERT_EQ_INT(ncol, 2);

  /* result[0..ncol-1] is the header row; data follows row-major. */
  TEST_ASSERT_EQ_STR(result[0], "id");
  TEST_ASSERT_EQ_STR(result[1], "name");
  TEST_ASSERT_EQ_STR(result[2], "1");
  TEST_ASSERT_EQ_STR(result[3], "alice");
  TEST_ASSERT_EQ_STR(result[4], "2");
  TEST_ASSERT_EQ_STR(result[5], "bob");
  sqlite3_free_table(result);

  /* An empty result set: get_table is built on sqlite3_exec(), whose
   * callback (the only place get_table learns the column count/names)
   * never fires when a query produces zero rows -- so unlike a normal
   * prepared-statement SELECT, an empty get_table() result reports
   * ncol == 0 too, not just nrow == 0. */
  result = NULL;
  rc = sqlite3_get_table(db, "SELECT id, name FROM t WHERE 0", &result, &nrow,
                          &ncol, &errmsg);
  TEST_ASSERT(rc == SQLITE_OK);
  TEST_ASSERT_EQ_INT(nrow, 0);
  TEST_ASSERT_EQ_INT(ncol, 0);
  sqlite3_free_table(result);

  /* A malformed query reports its error the same way sqlite3_exec does. */
  result = NULL;
  errmsg = NULL;
  rc = sqlite3_get_table(db, "SELEC bogus", &result, &nrow, &ncol, &errmsg);
  TEST_ASSERT(rc != SQLITE_OK);
  TEST_ASSERT(errmsg != NULL);
  sqlite3_free(errmsg);
  sqlite3_free_table(result); /* documented safe even when result is NULL */

  test_close_ok(db);
  TEST_PASS("test_get_table");
  return 0;
}
