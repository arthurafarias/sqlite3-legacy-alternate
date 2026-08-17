#define _GNU_SOURCE 1
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite/SQLiteThread.h"
#include "sqlite/sqlite3.h"
#include "sqlite/u64.h"
#include "sqlite/SqliteResultCode.h"
int sqlite3ThreadCreate(SQLiteThread **ppThread, void *(*xTask)(void *), void *pIn) {
  SQLiteThread *p;
  int rc;

  *ppThread = 0;
  p = sqlite3Malloc(sizeof(*p));
  if (p == 0)
    return 7;
  memset(p, 0, sizeof(*p));
  p->xTask = xTask;
  p->pIn = pIn;

  if (sqlite3FaultSim(200)) {
    rc = 1;
  } else {
    rc = pthread_create(&p->tid, 0, xTask, pIn);
  }
  if (rc) {
    p->done = 1;
    p->pOut = xTask(pIn);
  }
  *ppThread = p;
  return SQLITE_OK;
}

int sqlite3ThreadJoin(SQLiteThread *p, void **ppOut) {
  int rc;

  if (p == 0)
    return 7;
  if (p->done) {
    *ppOut = p->pOut;
    rc = SQLITE_OK;
  } else {
    rc = pthread_join(p->tid, ppOut) ? SQLITE_ERROR : SQLITE_OK;
  }
  sqlite3_free(p);
  return rc;
}
