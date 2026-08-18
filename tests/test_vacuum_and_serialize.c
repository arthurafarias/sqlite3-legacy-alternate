#include "test_common.h"
#include <unistd.h>

#define VACUUM_DB_PATH "./test_vacuum_scratch.db"

int main(void) {
  remove(VACUUM_DB_PATH);

  /* VACUUM on an in-memory db is legal but a no-op-ish rebuild; exercise it
   * for the basic path, plus VACUUM INTO to snapshot to a new file. */
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE t(id INTEGER PRIMARY KEY, payload TEXT)");
  test_exec_ok(db, "WITH RECURSIVE seq(v) AS "
                   "(SELECT 1 UNION ALL SELECT v+1 FROM seq WHERE v < 1000) "
                   "INSERT INTO t SELECT v, hex(randomblob(100)) FROM seq");
  test_exec_ok(db, "DELETE FROM t WHERE id % 2 = 0"); /* leave free pages behind */
  test_exec_ok(db, "VACUUM");

  char vacuum_into_sql[128];
  snprintf(vacuum_into_sql, sizeof(vacuum_into_sql), "VACUUM INTO '%s'",
           VACUUM_DB_PATH);
  test_exec_ok(db, vacuum_into_sql);
  TEST_ASSERT_MSG(access(VACUUM_DB_PATH, F_OK) == 0,
                   "VACUUM INTO should have created the target file");

  sqlite3 *snap = NULL;
  TEST_ASSERT(sqlite3_open_v2(VACUUM_DB_PATH, &snap, SQLITE_OPEN_READONLY,
                               NULL) == SQLITE_OK);
  sqlite3_stmt *stmt;
  TEST_OK(snap, sqlite3_prepare_v2(snap, "SELECT COUNT(*) FROM t", -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 500);
  sqlite3_finalize(stmt);
  test_close_ok(snap);
  remove(VACUUM_DB_PATH);

  /* Incremental auto-vacuum mode requires being set before any tables are
   * created (or via VACUUM to rebuild), then PRAGMA incremental_vacuum
   * reclaims freed pages a chunk at a time. */
  sqlite3 *db2 = test_open_memory_db();
  test_exec_ok(db2, "PRAGMA auto_vacuum = INCREMENTAL");
  test_exec_ok(db2, "CREATE TABLE t(id INTEGER PRIMARY KEY, payload TEXT)");
  test_exec_ok(db2, "VACUUM"); /* apply the auto_vacuum mode change */
  test_exec_ok(db2, "WITH RECURSIVE seq(v) AS "
                    "(SELECT 1 UNION ALL SELECT v+1 FROM seq WHERE v < 1000) "
                    "INSERT INTO t SELECT v, hex(randomblob(100)) FROM seq");
  test_exec_ok(db2, "DELETE FROM t");
  test_exec_ok(db2, "PRAGMA incremental_vacuum(10)");
  test_close_ok(db2);

  /* sqlite3_serialize copies the whole database into memory as a single
   * buffer, and sqlite3_deserialize loads such a buffer as a live db. */
  sqlite3 *src = test_open_memory_db();
  test_exec_ok(src, "CREATE TABLE t(x INTEGER)");
  test_exec_ok(src, "INSERT INTO t VALUES (1), (2), (3)");

  sqlite3_int64 size = 0;
  unsigned char *blob = sqlite3_serialize(src, "main", &size, 0);
  TEST_ASSERT_MSG(blob != NULL && size > 0, "serialize should return a non-empty buffer");

  /* Copy the buffer since deserialize (without FREEONCLOSE) doesn't take
   * ownership, and src is closed independently below. */
  unsigned char *owned = sqlite3_malloc64(size);
  TEST_ASSERT(owned != NULL);
  memcpy(owned, blob, size);
  sqlite3_free(blob);

  sqlite3 *restored = test_open_memory_db();
  int rc = sqlite3_deserialize(restored, "main", owned, size, size,
                                SQLITE_DESERIALIZE_FREEONCLOSE);
  TEST_ASSERT_MSG(rc == SQLITE_OK, "deserialize failed: %d", rc);

  TEST_OK(restored, sqlite3_prepare_v2(restored, "SELECT SUM(x) FROM t", -1,
                                        &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 6);
  sqlite3_finalize(stmt);

  test_close_ok(src);
  test_close_ok(restored); /* frees `owned` via FREEONCLOSE */

  TEST_PASS("test_vacuum_and_serialize");
  return 0;
}
