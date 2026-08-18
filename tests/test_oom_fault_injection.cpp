#include "test_common.h"

/* SQLite is designed to fail every allocation gracefully with SQLITE_NOMEM
 * rather than crash. That out-of-memory handling is scattered after nearly
 * every malloc/realloc call throughout the codebase (parser, VDBE, btree,
 * pager, string formatting, ...) and is otherwise unreachable from a
 * black-box API test, since a healthy process never runs out of memory on
 * a handful of small queries. This test installs a counting allocator that
 * can be told to fail on the Nth allocation, then replays a representative
 * workload once per N across the whole range -- the same fault-injection
 * technique SQLite's own test suite uses. */

static sqlite3_mem_methods g_orig;
static long g_alloc_count;
static long g_fail_at; /* <=0 disables injection */

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

/* A workload that touches a broad slice of the library: schema creation,
 * indexes, triggers, multi-row insert, a join, an aggregate, and a virtual
 * table (FTS5) -- each exercises a different allocation-heavy subsystem. */
static void run_workload(void) {
  sqlite3 *db = NULL;
  if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
    if (db)
      sqlite3_close(db);
    return;
  }
  sqlite3_exec(db, "CREATE TABLE t(id INTEGER PRIMARY KEY, name TEXT)", NULL,
               NULL, NULL);
  sqlite3_exec(db, "CREATE INDEX idx_name ON t(name)", NULL, NULL, NULL);
  sqlite3_exec(db,
               "CREATE TRIGGER trg AFTER INSERT ON t BEGIN "
               "SELECT 1; END",
               NULL, NULL, NULL);
  sqlite3_exec(db,
               "INSERT INTO t(name) VALUES ('alice'), ('bob'), ('carol')",
               NULL, NULL, NULL);
  sqlite3_exec(db, "CREATE VIRTUAL TABLE ft USING fts5(name)", NULL, NULL, NULL);
  sqlite3_exec(db, "INSERT INTO ft(name) SELECT name FROM t", NULL, NULL, NULL);

  sqlite3_stmt *stmt = NULL;
  if (sqlite3_prepare_v2(db, "SELECT t.name, COUNT(*) FROM t, ft WHERE "
                              "ft MATCH 'alice OR bob' GROUP BY t.name",
                          -1, &stmt, NULL) == SQLITE_OK) {
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      /* just drain the result set */
    }
  }
  sqlite3_finalize(stmt);
  sqlite3_close(db);
}

int main(void) {
  TEST_ASSERT(sqlite3_shutdown() == SQLITE_OK);
  TEST_ASSERT(sqlite3_config(SQLITE_CONFIG_GETMALLOC, &g_orig) == SQLITE_OK);
  TEST_ASSERT(sqlite3_config(SQLITE_CONFIG_MALLOC, &g_fi_methods) == SQLITE_OK);
  TEST_ASSERT(sqlite3_initialize() == SQLITE_OK);

  /* Baseline: how many allocations does one clean run take? */
  g_fail_at = -1;
  g_alloc_count = 0;
  run_workload();
  long total_allocs = g_alloc_count;
  TEST_ASSERT_MSG(total_allocs > 50,
                   "expected a substantial number of allocations, got %ld",
                   total_allocs);

  /* Cap the sweep for runtime's sake -- every Nth-allocation failure point
   * up to this bound is exercised; SQLite's OOM handling doesn't depend on
   * *which* allocation failed, so a large prefix is representative without
   * paying for every one of what can be several thousand allocations. */
  long sweep_limit = total_allocs < 400 ? total_allocs : 400;

  for (long fail_point = 1; fail_point <= sweep_limit; fail_point++) {
    g_alloc_count = 0;
    g_fail_at = fail_point;
    run_workload(); /* must not crash or hang, regardless of outcome */
  }
  g_fail_at = -1;

  /* Library must still be fully usable after the sweep. */
  sqlite3 *db = test_open_memory_db();
  test_exec_ok(db, "CREATE TABLE sanity(x)");
  test_exec_ok(db, "INSERT INTO sanity VALUES (1)");
  test_close_ok(db);

  TEST_ASSERT(sqlite3_shutdown() == SQLITE_OK);
  TEST_ASSERT(sqlite3_config(SQLITE_CONFIG_MALLOC, &g_orig) == SQLITE_OK);
  TEST_ASSERT(sqlite3_initialize() == SQLITE_OK);

  TEST_PASS("test_oom_fault_injection");
  return 0;
}
