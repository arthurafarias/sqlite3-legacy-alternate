#include "test_common.h"
#include <stdint.h>

/* SQLite's *_16 API family exchanges text as native-byte-order UTF-16.
 * These helpers convert plain ASCII (a subset of UTF-16 with each code
 * unit equal to the ASCII byte) to and from that form -- avoiding a
 * dependency on platform wchar_t, which is 4 bytes wide on Linux. */
static uint16_t *ascii_to_utf16(const char *ascii) {
  size_t n = strlen(ascii);
  uint16_t *buf = (uint16_t*) malloc((n + 1) * sizeof(uint16_t));
  for (size_t i = 0; i <= n; i++)
    buf[i] = (uint16_t)(unsigned char)ascii[i];
  return buf;
}

static void utf16_to_ascii(const void *utf16, int nbytes, char *out, size_t outsize) {
  const uint16_t *p = (const uint16_t *)utf16;
  size_t n = (size_t)nbytes / 2;
  size_t i = 0;
  for (; i < n && i + 1 < outsize; i++)
    out[i] = (char)p[i];
  out[i] = '\0';
}

int main(void) {
  /* sqlite3_open16: filename is UTF-16, and the database's default text
   * encoding becomes UTF-16 (native byte order) for a fresh database. */
  uint16_t *mem_name = ascii_to_utf16(":memory:");
  sqlite3 *db = NULL;
  int rc = sqlite3_open16(mem_name, &db);
  TEST_ASSERT_MSG(rc == SQLITE_OK, "sqlite3_open16 failed: %d", rc);
  free(mem_name);

  test_exec_ok(db, "CREATE TABLE t(id INTEGER PRIMARY KEY, name TEXT)");

  /* sqlite3_prepare16_v2 + sqlite3_bind_text16. */
  uint16_t *insert_sql = ascii_to_utf16("INSERT INTO t(name) VALUES (?)");
  sqlite3_stmt *stmt = NULL;
  rc = sqlite3_prepare16_v2(db, insert_sql, -1, &stmt, NULL);
  TEST_ASSERT_MSG(rc == SQLITE_OK, "prepare16_v2 failed: %d", rc);
  free(insert_sql);

  uint16_t *name_utf16 = ascii_to_utf16("hello-utf16");
  TEST_OK(db, sqlite3_bind_text16(stmt, 1, name_utf16, -1, SQLITE_TRANSIENT));
  free(name_utf16);
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_DONE);
  sqlite3_finalize(stmt);

  /* sqlite3_column_text16 / column_bytes16 read it back. */
  uint16_t *select_sql = ascii_to_utf16("SELECT name FROM t WHERE id = 1");
  rc = sqlite3_prepare16_v2(db, select_sql, -1, &stmt, NULL);
  TEST_ASSERT(rc == SQLITE_OK);
  free(select_sql);

  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  const void *col_text = sqlite3_column_text16(stmt, 0);
  int col_bytes = sqlite3_column_bytes16(stmt, 0);
  char roundtrip[64];
  utf16_to_ascii(col_text, col_bytes, roundtrip, sizeof(roundtrip));
  TEST_ASSERT_EQ_STR(roundtrip, "hello-utf16");
  sqlite3_finalize(stmt);

  /* UTF-8 and UTF-16 API calls interoperate freely regardless of which
   * one wrote or reads the value -- SQLite transcodes as needed. */
  test_exec_ok(db, "INSERT INTO t(name) VALUES ('written-as-utf8')");
  uint16_t *select_all = ascii_to_utf16("SELECT name FROM t WHERE id = 2");
  rc = sqlite3_prepare16_v2(db, select_all, -1, &stmt, NULL);
  TEST_ASSERT(rc == SQLITE_OK);
  free(select_all);
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  col_text = sqlite3_column_text16(stmt, 0);
  col_bytes = sqlite3_column_bytes16(stmt, 0);
  utf16_to_ascii(col_text, col_bytes, roundtrip, sizeof(roundtrip));
  TEST_ASSERT_EQ_STR(roundtrip, "written-as-utf8");
  sqlite3_finalize(stmt);

  /* sqlite3_errmsg16 on a deliberate failure. */
  rc = sqlite3_exec(db, "SELECT * FROM no_such_table", NULL, NULL, NULL);
  TEST_ASSERT(rc != SQLITE_OK);
  TEST_ASSERT(sqlite3_errmsg16(db) != NULL);

  sqlite3_close(db);
  TEST_PASS("test_utf16");
  return 0;
}
