#include <string.h>
#include "record_store.h"
#include "test_harness.h"

/* Positive path: record_store_open, record_store_insert, record_store_get,
 * record_store_close all round-trip correctly. */
static void test_insert_then_get_roundtrips(void) {
  record_store *s = record_store_open(16);
  TEST_ASSERT(s != NULL);
  TEST_ASSERT(record_store_insert(s, 1, "alice") == 0);
  TEST_ASSERT(record_store_insert(s, 2, "bob") == 0);
  const char *alice = record_store_get(s, 1);
  TEST_ASSERT(alice != NULL && strcmp(alice, "alice") == 0);
  const char *bob = record_store_get(s, 2);
  TEST_ASSERT(bob != NULL && strcmp(bob, "bob") == 0);
  record_store_close(s);
}

/* Negative path: record_store_get on a key that was never inserted. */
static void test_get_missing_key_returns_null(void) {
  record_store *s = record_store_open(4);
  TEST_ASSERT(record_store_get(s, 999) == NULL);
  record_store_close(s);
}

/* Positive path: record_store_insert on an existing key updates the value
 * in place instead of failing or growing the store. */
static void test_insert_overwrites_existing_key(void) {
  record_store *s = record_store_open(4);
  TEST_ASSERT(record_store_insert(s, 5, "first") == 0);
  TEST_ASSERT(record_store_insert(s, 5, "second") == 0);
  const char *v = record_store_get(s, 5);
  TEST_ASSERT(v != NULL && strcmp(v, "second") == 0);
  record_store_close(s);
}

/* Negative path: record_store_insert once the store is at capacity. */
static void test_insert_fails_when_full(void) {
  record_store *s = record_store_open(2);
  TEST_ASSERT(record_store_insert(s, 1, "a") == 0);
  TEST_ASSERT(record_store_insert(s, 2, "b") == 0);
  TEST_ASSERT(record_store_insert(s, 3, "c") == -1);
  record_store_close(s);
}

/* Negative path: record_store_open with a non-positive capacity. */
static void test_open_rejects_nonpositive_capacity(void) {
  TEST_ASSERT(record_store_open(0) == NULL);
  TEST_ASSERT(record_store_open(-1) == NULL);
}

int main(void) {
  test_insert_then_get_roundtrips();
  test_get_missing_key_returns_null();
  test_insert_overwrites_existing_key();
  test_insert_fails_when_full();
  test_open_rejects_nonpositive_capacity();
  return test_summary("srs-009 coverage-harness-example");
}
