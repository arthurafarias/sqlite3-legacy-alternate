#include "test_common.h"

/* A second, independent OOM fault-injection sweep (see
 * test_oom_fault_injection.c for the technique itself) driving a
 * deliberately different workload: ALTER TABLE, views, backup, blob I/O,
 * window functions, ATTACH, and VACUUM. Malloc failure handling is spread
 * throughout the codebase in a call-site-specific way, so a workload that
 * never touches (say) alter.c or backup.c can never exercise the NOMEM
 * branches living there, no matter how many allocation-failure points the
 * first sweep's workload is replayed against. */

static sqlite3_mem_methods g_orig;
static long g_alloc_count;
static long g_fail_at;

static void *fi_malloc(int n) {
  g_alloc_count++;
  if (g_fail_at > 0 && g_alloc_count == g_fail_at)
    return NULL;
  return g_orig.xMalloc(n);
}
static void fi_free(void *p) { g_orig.xFree(p); }
static void *fi_realloc(void *p, int n) {
  g_alloc_count++;
  if (g_fail_at > 0 && g_alloc_count == g_fail_at)
    return NULL;
  return g_orig.xRealloc(p, n);
}
static int fi_size(void *p) { return g_orig.xSize(p); }
static int fi_roundup(int n) { return g_orig.xRoundup(n); }
static int fi_init(void *p) {
  return g_orig.xInit ? g_orig.xInit(g_orig.pAppData) : SQLITE_OK;
}
static void fi_shutdown(void *p) {
  if (g_orig.xShutdown)
    g_orig.xShutdown(g_orig.pAppData);
}
static const sqlite3_mem_methods g_fi_methods = {
    fi_malloc, fi_free, fi_realloc, fi_size, fi_roundup, fi_init, fi_shutdown, NULL,
};

static void run_workload(void) {
  sqlite3 *db = NULL;
  if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
    if (db)
      sqlite3_close(db);
    return;
  }
  sqlite3_exec(db, "CREATE TABLE t(id INTEGER PRIMARY KEY, a INTEGER, b TEXT)",
               NULL, NULL, NULL);
  sqlite3_exec(db, "INSERT INTO t VALUES (1,10,'x'),(2,20,'y'),(3,30,'z')", NULL,
               NULL, NULL);
  sqlite3_exec(db, "ALTER TABLE t ADD COLUMN c REAL DEFAULT 0", NULL, NULL, NULL);
  sqlite3_exec(db, "ALTER TABLE t RENAME COLUMN c TO score", NULL, NULL, NULL);
  sqlite3_exec(db, "CREATE VIEW v AS SELECT a, b FROM t WHERE a > 5", NULL, NULL,
               NULL);
  sqlite3_exec(db,
               "SELECT a, SUM(a) OVER (ORDER BY a) FROM v", NULL, NULL, NULL);
  sqlite3_exec(db, "UPDATE t SET score = a * 1.5", NULL, NULL, NULL);

  sqlite3_stmt *blob_ins = NULL;
  if (sqlite3_prepare_v2(db, "INSERT INTO t(id, b) VALUES (99, zeroblob(32))",
                          -1, &blob_ins, NULL) == SQLITE_OK) {
    sqlite3_step(blob_ins);
  }
  sqlite3_finalize(blob_ins);
  sqlite3_blob *blob = NULL;
  if (sqlite3_blob_open(db, "main", "t", "b", 99, 1, &blob) == SQLITE_OK) {
    unsigned char buf[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    sqlite3_blob_write(blob, buf, sizeof(buf), 0);
    sqlite3_blob_close(blob);
  }

  sqlite3 *backup_dst = NULL;
  if (sqlite3_open(":memory:", &backup_dst) == SQLITE_OK) {
    sqlite3_backup *b = sqlite3_backup_init(backup_dst, "main", db, "main");
    if (b) {
      while (sqlite3_backup_step(b, 1) == SQLITE_OK) {
      }
      sqlite3_backup_finish(b);
    }
    sqlite3_close(backup_dst);
  }

  sqlite3_exec(db, "ATTACH DATABASE ':memory:' AS side", NULL, NULL, NULL);
  sqlite3_exec(db, "CREATE TABLE side.s(x)", NULL, NULL, NULL);
  sqlite3_exec(db, "INSERT INTO side.s SELECT id FROM t", NULL, NULL, NULL);
  sqlite3_exec(db, "DETACH DATABASE side", NULL, NULL, NULL);

  sqlite3_exec(db, "DELETE FROM t WHERE id = 1", NULL, NULL, NULL);
  sqlite3_exec(db, "VACUUM", NULL, NULL, NULL);

  sqlite3_close(db);
}

int main(void) {
  TEST_ASSERT(sqlite3_shutdown() == SQLITE_OK);
  TEST_ASSERT(sqlite3_config(SQLITE_CONFIG_GETMALLOC, &g_orig) == SQLITE_OK);
  TEST_ASSERT(sqlite3_config(SQLITE_CONFIG_MALLOC, &g_fi_methods) == SQLITE_OK);
  TEST_ASSERT(sqlite3_initialize() == SQLITE_OK);

  g_fail_at = -1;
  g_alloc_count = 0;
  run_workload();
  long total_allocs = g_alloc_count;
  TEST_ASSERT_MSG(total_allocs > 50, "expected substantial allocation count, got %ld",
                   total_allocs);

  long sweep_limit = total_allocs < 500 ? total_allocs : 500;
  for (long fail_point = 1; fail_point <= sweep_limit; fail_point++) {
    g_alloc_count = 0;
    g_fail_at = fail_point;
    run_workload(); /* must not crash or hang */
  }
  g_fail_at = -1;

  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE sanity(x)");
  test_close_ok(db);

  TEST_ASSERT(sqlite3_shutdown() == SQLITE_OK);
  TEST_ASSERT(sqlite3_config(SQLITE_CONFIG_MALLOC, &g_orig) == SQLITE_OK);
  TEST_ASSERT(sqlite3_initialize() == SQLITE_OK);

  TEST_PASS("test_oom_fault_injection_extended");
  return 0;
}
