#include "test_common.h"
#include <math.h>

#define TEST_PI 3.14159265358979323846

static double eval_double(sqlite3 *db, const char *sql) {
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, sql, -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  double v = sqlite3_column_double(stmt, 0);
  sqlite3_finalize(stmt);
  return v;
}

static void assert_close(double a, double b, double eps) {
  TEST_ASSERT_MSG(fabs(a - b) < eps, "expected %.10f ~= %.10f", a, b);
}

int main(void) {
  sqlite3 *db = test_open_memory_db();

  assert_close(eval_double(db, "SELECT sin(0)"), 0.0, 1e-9);
  assert_close(eval_double(db, "SELECT cos(0)"), 1.0, 1e-9);
  assert_close(eval_double(db, "SELECT sqrt(4)"), 2.0, 1e-9);
  assert_close(eval_double(db, "SELECT pow(2, 10)"), 1024.0, 1e-9);
  assert_close(eval_double(db, "SELECT power(2, 10)"), 1024.0, 1e-9);
  assert_close(eval_double(db, "SELECT log10(1000)"), 3.0, 1e-9);
  assert_close(eval_double(db, "SELECT ln(exp(1))"), 1.0, 1e-9);
  assert_close(eval_double(db, "SELECT pi()"), TEST_PI, 1e-9);
  assert_close(eval_double(db, "SELECT degrees(pi())"), 180.0, 1e-6);
  assert_close(eval_double(db, "SELECT radians(180)"), TEST_PI, 1e-9);
  assert_close(eval_double(db, "SELECT ceil(4.1)"), 5.0, 1e-9);
  assert_close(eval_double(db, "SELECT floor(4.9)"), 4.0, 1e-9);
  assert_close(eval_double(db, "SELECT trunc(4.9)"), 4.0, 1e-9);
  assert_close(eval_double(db, "SELECT mod(10.0, 3.0)"), 1.0, 1e-9);
  assert_close(eval_double(db, "SELECT atan2(0, 1)"), 0.0, 1e-9);

  /* Combined with aggregate/window usage over generated data. */
  test_exec_ok(db, "CREATE TABLE pts(x REAL, y REAL)");
  test_exec_ok(db, "INSERT INTO pts VALUES (3,4), (0,0), (6,8)");
  assert_close(eval_double(db, "SELECT SUM(sqrt(x*x + y*y)) FROM pts"), 15.0, 1e-9);

  test_close_ok(db);
  TEST_PASS("test_math_functions");
  return 0;
}
