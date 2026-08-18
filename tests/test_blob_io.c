#include "test_common.h"

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE files(id INTEGER PRIMARY KEY, data BLOB)");
  test_exec_ok(db, "INSERT INTO files(data) VALUES (zeroblob(64))");

  sqlite3_int64 rowid = sqlite3_last_insert_rowid(db);
  TEST_ASSERT(rowid > 0);

  sqlite3_blob *blob = NULL;
  TEST_OK(db, sqlite3_blob_open(db, "main", "files", "data", rowid,
                                 /*flags=*/1, &blob));
  TEST_ASSERT_EQ_INT(sqlite3_blob_bytes(blob), 64);

  unsigned char chunk[16];
  memset(chunk, 0xAB, sizeof(chunk));
  TEST_OK(db, sqlite3_blob_write(blob, chunk, sizeof(chunk), 0));
  memset(chunk, 0xCD, sizeof(chunk));
  TEST_OK(db, sqlite3_blob_write(blob, chunk, sizeof(chunk), 16));

  unsigned char readback[32];
  TEST_OK(db, sqlite3_blob_read(blob, readback, sizeof(readback), 0));
  for (int i = 0; i < 16; i++)
    TEST_ASSERT(readback[i] == 0xAB);
  for (int i = 16; i < 32; i++)
    TEST_ASSERT(readback[i] == 0xCD);

  /* Writing past the end of a fixed-size blob must fail without resizing. */
  int rc = sqlite3_blob_write(blob, chunk, sizeof(chunk), 60);
  TEST_ASSERT_MSG(rc != SQLITE_OK, "write past blob end should fail, got %d", rc);

  TEST_OK(db, sqlite3_blob_close(blob));

  /* A second row + sqlite3_blob_reopen to point the same handle elsewhere. */
  test_exec_ok(db, "INSERT INTO files(data) VALUES (zeroblob(8))");
  sqlite3_int64 rowid2 = sqlite3_last_insert_rowid(db);
  TEST_OK(db, sqlite3_blob_open(db, "main", "files", "data", rowid, 1, &blob));
  TEST_OK(db, sqlite3_blob_reopen(blob, rowid2));
  TEST_ASSERT_EQ_INT(sqlite3_blob_bytes(blob), 8);
  TEST_OK(db, sqlite3_blob_close(blob));

  test_close_ok(db);
  TEST_PASS("test_blob_io");
  return 0;
}
