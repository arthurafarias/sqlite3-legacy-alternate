#ifndef SQLITE_LEGACY_TEST_COMMON_H
#define SQLITE_LEGACY_TEST_COMMON_H

#include "sqlite/sqlite3.h"
/* sqlite3.h only re-exports the subset of the public API the façade pass
 * (srs-003, see docs/architecture.md) routed through it; these headers
 * declare the rest of what a normal application build gets for free from
 * upstream SQLite's single-file sqlite3.h (backup, blob I/O, custom
 * function results, VFS registration, snapshots, sqlite3_str, ...), which
 * this test suite exercises directly. */
#include "sqlite/sqlite3_backup.h"
#include "sqlite/sqlite3_blob.h"
#include "sqlite/sqlite3_context.h"
#include "sqlite/sqlite3_hard_heap.h"
#include "sqlite/sqlite3_index_info.h"
#include "sqlite/sqlite3_libversion.h"
#include "sqlite/sqlite3_libversion_number.h"
#include "sqlite/sqlite3_mem_methods.h"
#include "sqlite/sqlite3_module.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/sqlite3_snapshot.h"
#include "sqlite/sqlite3_soft_heap.h"
#include "sqlite/sqlite3_sourceid.h"
#include "sqlite/sqlite3_stmt.h"
#include "sqlite/sqlite3_str.h"
#include "sqlite/sqlite3_vfs.h"
#include "sqlite/sqlite3_vtab.h"
#include "sqlite/sqlite3_vtab_cursor.h"

/* SQLITE_TEXT (the sqlite3_column_type()/sqlite3_value_type() constant) has
 * no #define in sqlite3.h here -- only its historical alias SQLITE3_TEXT
 * survived the container-convention split (see SqliteFundamentalDatatype.h
 * in src/sqlite/). Upstream SQLite always defines both to the same value. */
#ifndef SQLITE_TEXT
#define SQLITE_TEXT SQLITE3_TEXT
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Every test is a standalone executable run under CTest: a non-zero exit
 * code (via TEST_FAIL/TEST_ASSERT) is a failing test, a clean return from
 * main() is a pass. Keeping assertions fatal avoids partial/garbled state
 * being probed further after an invariant has already broken. */

#define TEST_FAIL(fmt, ...)                                                  \
  do {                                                                       \
    fprintf(stderr, "FAIL %s:%d: " fmt "\n", __FILE__, __LINE__,             \
            ##__VA_ARGS__);                                                  \
    exit(1);                                                                 \
  } while (0)

#define TEST_ASSERT(cond, ...)                                               \
  do {                                                                       \
    if (!(cond)) {                                                           \
      TEST_FAIL("assertion failed (%s)", #cond);                             \
    }                                                                        \
  } while (0)

#define TEST_ASSERT_MSG(cond, fmt, ...)                                      \
  do {                                                                       \
    if (!(cond)) {                                                           \
      TEST_FAIL("assertion failed (%s): " fmt, #cond, ##__VA_ARGS__);        \
    }                                                                        \
  } while (0)

#define TEST_ASSERT_EQ_INT(actual, expected)                                 \
  do {                                                                       \
    long long _a = (long long)(actual);                                     \
    long long _e = (long long)(expected);                                   \
    if (_a != _e) {                                                         \
      TEST_FAIL("%s == %lld, expected %s == %lld", #actual, _a, #expected,   \
                _e);                                                        \
    }                                                                        \
  } while (0)

#define TEST_ASSERT_EQ_STR(actual, expected)                                 \
  do {                                                                       \
    const char *_a = (const char *)(actual);                                \
    const char *_e = (const char *)(expected);                              \
    if (_a == NULL || _e == NULL || strcmp(_a, _e) != 0) {                   \
      TEST_FAIL("%s == \"%s\", expected %s == \"%s\"", #actual,              \
                _a ? _a : "(null)", #expected, _e ? _e : "(null)");          \
    }                                                                        \
  } while (0)

#define TEST_OK(db, rc)                                                      \
  do {                                                                       \
    int _rc = (rc);                                                         \
    if (_rc != SQLITE_OK) {                                                 \
      TEST_FAIL("expected SQLITE_OK, got %d (%s): %s", _rc,                  \
                sqlite3_errstr(_rc), sqlite3_errmsg(db));                    \
    }                                                                        \
  } while (0)

#define TEST_PASS(name)                                                      \
  do {                                                                       \
    printf("PASS %s\n", (name));                                            \
  } while (0)

static inline sqlite3 *test_open_memory_db(void) {
  sqlite3 *db = NULL;
  int rc = sqlite3_open_v2(
      ":memory:", &db,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, NULL);
  TEST_ASSERT_MSG(rc == SQLITE_OK, "sqlite3_open_v2 failed: %d", rc);
  return db;
}

static inline void test_exec_ok(sqlite3 *db, const char *sql) {
  char *errmsg = NULL;
  int rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
  if (rc != SQLITE_OK) {
    TEST_FAIL("exec failed: %s (sql: %s)", errmsg ? errmsg : "(no message)",
              sql);
  }
  sqlite3_free(errmsg);
}

static inline void test_close_ok(sqlite3 *db) {
  int rc = sqlite3_close(db);
  TEST_ASSERT_MSG(rc == SQLITE_OK,
                   "sqlite3_close failed: %d (statements left open?)", rc);
}

#endif /* SQLITE_LEGACY_TEST_COMMON_H */
