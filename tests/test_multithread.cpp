#include "test_common.h"
#include <pthread.h>

#define DB_PATH "./test_multithread_scratch.db"
#define NUM_THREADS 8
#define ROWS_PER_THREAD 200

static void remove_db_files(void) {
  remove(DB_PATH);
  remove(DB_PATH "-wal");
  remove(DB_PATH "-shm");
  remove(DB_PATH "-journal");
}

typedef struct {
  int thread_id;
  int failed;
} worker_arg;

static void *worker(void *arg_) {
  worker_arg *arg = (worker_arg *)arg_;
  sqlite3 *db = NULL;
  if (sqlite3_open_v2(DB_PATH, &db,
                       SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) !=
      SQLITE_OK) {
    arg->failed = 1;
    return NULL;
  }
  sqlite3_busy_timeout(db, 5000);

  sqlite3_stmt *stmt = NULL;
  if (sqlite3_prepare_v2(db, "INSERT INTO t(thread_id, seq) VALUES (?, ?)", -1,
                          &stmt, NULL) != SQLITE_OK) {
    arg->failed = 1;
    sqlite3_close(db);
    return NULL;
  }

  for (int i = 0; i < ROWS_PER_THREAD; i++) {
    sqlite3_bind_int(stmt, 1, arg->thread_id);
    sqlite3_bind_int(stmt, 2, i);
    int rc;
    do {
      rc = sqlite3_step(stmt);
    } while (rc == SQLITE_BUSY);
    if (rc != SQLITE_DONE) {
      arg->failed = 1;
      break;
    }
    sqlite3_reset(stmt);
  }

  sqlite3_finalize(stmt);
  sqlite3_close(db);
  return NULL;
}

int main(void) {
  TEST_ASSERT_MSG(sqlite3_threadsafe() != 0,
                   "library must be built SQLITE_THREADSAFE for this test");

  remove_db_files();
  sqlite3 *db = NULL;
  int rc = sqlite3_open(DB_PATH, &db);
  TEST_OK(db, rc);
  test_exec_ok(db, "PRAGMA journal_mode=WAL");
  test_exec_ok(db, "CREATE TABLE t(thread_id INTEGER, seq INTEGER)");
  test_close_ok(db);

  pthread_t threads[NUM_THREADS];
  worker_arg args[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; i++) {
    args[i].thread_id = i;
    args[i].failed = 0;
    TEST_ASSERT(pthread_create(&threads[i], NULL, worker, &args[i]) == 0);
  }
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
    TEST_ASSERT_MSG(!args[i].failed, "worker thread %d reported a failure", i);
  }

  rc = sqlite3_open(DB_PATH, &db);
  TEST_OK(db, rc);
  sqlite3_stmt *stmt;
  TEST_OK(db, sqlite3_prepare_v2(db, "SELECT COUNT(*), COUNT(DISTINCT thread_id) FROM t",
                                  -1, &stmt, NULL));
  TEST_ASSERT(sqlite3_step(stmt) == SQLITE_ROW);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 0), NUM_THREADS * ROWS_PER_THREAD);
  TEST_ASSERT_EQ_INT(sqlite3_column_int(stmt, 1), NUM_THREADS);
  sqlite3_finalize(stmt);
  test_close_ok(db);

  remove_db_files();
  TEST_PASS("test_multithread");
  return 0;
}
