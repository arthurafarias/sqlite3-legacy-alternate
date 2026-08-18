/* A minimal runtime-loadable SQLite extension, built as a separate shared
 * object so test_load_extension.c can exercise sqlite3_load_extension()
 * and the sqlite3_enable_load_extension()/DBCONFIG gate around it -- code
 * paths a statically-linked test program can otherwise never reach.
 *
 * Unlike a standard SQLite extension, this doesn't go through the
 * sqlite3ext.h/sqlite3_api_routines thunk (this library doesn't provide
 * that header): it links directly against the real library, since both
 * the test executable and this module resolve against the same shared
 * object at runtime. */
#include <stddef.h>
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_api_routines.h"
#include "sqlite/sqlite3_context.h"

static void ext_double_func(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
  (void)argc;
  sqlite3_result_double(ctx, sqlite3_value_double(argv[0]) * 2.0);
}

#ifdef _WIN32
__declspec(dllexport)
#endif
int sqlite3_extension_init(sqlite3 *db, char **pzErrMsg,
                            const sqlite3_api_routines *pApi) {
  (void)pApi;
  (void)pzErrMsg;
  return sqlite3_create_function(db, "ext_double", 1,
                                  SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL,
                                  ext_double_func, NULL, NULL);
}
