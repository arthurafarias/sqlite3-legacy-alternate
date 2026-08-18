#include "test_common.h"

int main(void) {
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE main_t(id INTEGER PRIMARY KEY, v TEXT)");
  test_exec_ok(db, "INSERT INTO main_t VALUES (1, 'from-main')");

  /* Attach a second, independent in-memory database under an alias. */
  test_exec_ok(db, "ATTACH DATABASE ':memory:' AS side");
  test_exec_ok(db, "CREATE TABLE side.side_t(id INTEGER PRIMARY KEY, v TEXT)");
  test_exec_ok(db, "INSERT INTO side.side_t VALUES (1, 'from-side')");

  /* sqlite3_db_names / database_name enumeration via pragma. */
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "PRAGMA database_list", -1, &stmt, NULL));
  int seen_main = 0, seen_side = 0;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const char *name = (const char *)sqlite3_column_text(stmt, 1);
    if (strcmp(name, "main") == 0)
      seen_main = 1;
    if (strcmp(name, "side") == 0)
      seen_side = 1;
  }
  TEST_ASSERT(seen_main && seen_side);
  sqlite3_finalize(stmt);

  /* Cross-database join. */
  TEST_OK(db, sqlite3_prepare_v2(
                  db,
                  "SELECT main_t.v, side.side_t.v FROM main_t, side.side_t "
                  "WHERE main_t.id = side.side_t.id",
                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 0), "from-main");
  TEST_ASSERT_EQ_STR((const char *)sqlite3_column_text(stmt, 1), "from-side");
  sqlite3_finalize(stmt);

  /* Cross-database write inside a single transaction spans both files. */
  test_exec_ok(db, "BEGIN");
  test_exec_ok(db, "INSERT INTO main_t VALUES (2, 'm2')");
  test_exec_ok(db, "INSERT INTO side.side_t VALUES (2, 's2')");
  test_exec_ok(db, "COMMIT");

  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM side.side_t", -1,
                                  &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), 2);
  sqlite3_finalize(stmt);

  /* sqlite3_db_name(db, N) walks attached databases by index. */
  int found_side_by_index = 0;
  for (int i = 0; i < 8; i++) {
    const char *name = sqlite3_db_name(db, i);
    if (name == NULL)
      break;
    if (strcmp(name, "side") == 0)
      found_side_by_index = 1;
  }
  TEST_ASSERT(found_side_by_index);

  /* DETACH removes it; the attached table then becomes unreachable. */
  test_exec_ok(db, "DETACH DATABASE side");
  int rc = sqlite3_exec(db, "SELECT * FROM side.side_t", NULL, NULL, NULL);
  TEST_ASSERT(rc != SQLITE_OK);

  test_close_ok(db);
  TEST_PASS("test_attach_detach");
  return 0;
}
