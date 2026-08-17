#pragma once
#include <stdio.h>

typedef struct test_stats {
  int total;
  int passed;
  int failed;
} test_stats;

static test_stats g_test_stats = {0, 0, 0};

static inline void test_check(int cond, const char *expr, const char *file, int line) {
  g_test_stats.total++;
  if (cond) {
    g_test_stats.passed++;
  } else {
    g_test_stats.failed++;
    fprintf(stderr, "FAIL %s:%d: %s\n", file, line, expr);
  }
}

/* Records pass/fail and keeps running, unlike assert() halting on the
 * first broken invariant (see srs-009.md test-strategy rule 4). */
#define TEST_ASSERT(cond) test_check((cond), #cond, __FILE__, __LINE__)

static inline int test_summary(const char *suite_name) {
  printf("%s: %d/%d passed\n", suite_name, g_test_stats.passed, g_test_stats.total);
  return g_test_stats.failed == 0 ? 0 : 1;
}
