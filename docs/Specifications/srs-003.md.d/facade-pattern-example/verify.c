/* Verification executable: a minimal sqlite3-shaped application
 * (open / exec / prepare / step / column / finalize / close) built
 * against nothing but the public facade headers below, proving the
 * srs-003 layout actually compiles, links, and runs. */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "db.h"
#include "db_limits.h"
#include "stmt.h"

int main(void) {
  assert(strlen("hello") <= (size_t)db_limits_default.max_key_len);

  db *conn = db_open("facade-demo");
  assert(conn != NULL);

  assert(db_exec(conn, "SET hello world") == 0);
  assert(db_exec(conn, "SET answer 42") == 0);

  stmt *s = db_prepare(conn, "GET hello");
  assert(s != NULL);
  assert(stmt_step(s) == 1);
  assert(strcmp(stmt_column_text(s, 0), "world") == 0);
  assert(stmt_step(s) == 0);
  stmt_finalize(s);

  stmt *missing = db_prepare(conn, "GET nope");
  assert(missing != NULL);
  assert(stmt_step(missing) == 0);
  stmt_finalize(missing);

  db_close(conn);

  printf("srs-003 facade example: PASS\n");
  return 0;
}
