/* Verification executable for SRS-003: a minimal sqlite3 application built
 * against nothing but the public sqlite3 API, linked against the
 * srs-003-organized libsqlite3-legacy-alternate. Proves the façade
 * visibility pass (one type per header, static-vs-public function
 * placement, extern-const scoping) still produces a working library. */
 
#include <stdio.h>
#include <string.h>
#include "sqlite/sqlite3.h"
static int print_row(void *ctx, int argc, char **argv, char **colName) {
  int *count = (int *)ctx;
  (*count)++;
  for (int i = 0; i < argc; i++) {
    printf("  %s = %s\n", colName[i], argv[i] ? argv[i] : "NULL");
  }
  return 0;
}

int main(void) {
  sqlite3 *db = NULL;
  char *errmsg = NULL;
  int rc;

  rc = sqlite3_open(":memory:", &db);
  if (rc != 0 || db == NULL) {
    fprintf(stderr, "sqlite3_open failed: %d\n", rc);
    return 1;
  }

  rc = sqlite3_exec(db, "CREATE TABLE t(id INTEGER PRIMARY KEY, name TEXT);", NULL, NULL, &errmsg);
  if (rc != 0) {
    fprintf(stderr, "CREATE TABLE failed: %s\n", errmsg ? errmsg : sqlite3_errmsg(db));
    sqlite3_free(errmsg);
    sqlite3_close(db);
    return 1;
  }

  rc = sqlite3_exec(db, "INSERT INTO t(name) VALUES ('facade'), ('srs-003');", NULL, NULL, &errmsg);
  if (rc != 0) {
    fprintf(stderr, "INSERT failed: %s\n", errmsg ? errmsg : sqlite3_errmsg(db));
    sqlite3_free(errmsg);
    sqlite3_close(db);
    return 1;
  }

  int rowCount = 0;
  rc = sqlite3_exec(db, "SELECT id, name FROM t ORDER BY id;", print_row, &rowCount, &errmsg);
  if (rc != 0) {
    fprintf(stderr, "SELECT failed: %s\n", errmsg ? errmsg : sqlite3_errmsg(db));
    sqlite3_free(errmsg);
    sqlite3_close(db);
    return 1;
  }

  sqlite3_close(db);

  if (rowCount != 2) {
    fprintf(stderr, "expected 2 rows, got %d\n", rowCount);
    return 1;
  }

  printf("srs-003 real-library verification: PASS\n");
  return 0;
}
