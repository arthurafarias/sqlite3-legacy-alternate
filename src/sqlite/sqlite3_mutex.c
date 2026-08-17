#define _GNU_SOURCE 1
#include <pthread.h>
#include <stdlib.h>
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/Sqlite3Config.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_mutex_methods.h"
#include "sqlite/SqliteMutexType.h"
#include "sqlite/SqliteResultCode.h"
sqlite3_mutex *sqlite3_mutex_alloc(int id) {
  if (id <= SQLITE_MUTEX_RECURSIVE && sqlite3_initialize())
    return 0;
  if (id > SQLITE_MUTEX_RECURSIVE && sqlite3MutexInit())
    return 0;

  return sqlite3Config.mutex.xMutexAlloc(id);
}

void sqlite3_mutex_free(sqlite3_mutex *p) {
  if (p) {
    sqlite3Config.mutex.xMutexFree(p);
  }
}

void sqlite3_mutex_enter(sqlite3_mutex *p) {
  if (p) {
    sqlite3Config.mutex.xMutexEnter(p);
  }
}

int sqlite3_mutex_try(sqlite3_mutex *p) {
  int rc = SQLITE_OK;
  if (p) {
    return sqlite3Config.mutex.xMutexTry(p);
  }
  return rc;
}

void sqlite3_mutex_leave(sqlite3_mutex *p) {
  if (p) {
    sqlite3Config.mutex.xMutexLeave(p);
  }
}

void noopMutexFree(sqlite3_mutex *p) {
  (void)(p);
  return;
}

void noopMutexEnter(sqlite3_mutex *p) {
  (void)(p);
  return;
}

int noopMutexTry(sqlite3_mutex *p) {
  (void)(p);
  return SQLITE_OK;
}

void noopMutexLeave(sqlite3_mutex *p) {
  (void)(p);
  return;
}

void pthreadMutexFree(sqlite3_mutex *p) {
  {
    pthread_mutex_destroy(&p->mutex);
    sqlite3_free(p);
  }
}

void pthreadMutexEnter(sqlite3_mutex *p) {
  pthread_mutex_lock(&p->mutex);
}

int pthreadMutexTry(sqlite3_mutex *p) {
  int rc;

  if (pthread_mutex_trylock(&p->mutex) == 0) {
    rc = SQLITE_OK;
  } else {
    rc = SQLITE_BUSY;
  }

  return rc;
}

void pthreadMutexLeave(sqlite3_mutex *p) {
  pthread_mutex_unlock(&p->mutex);
}

sqlite3_mutex *unixBigLock = 0;
