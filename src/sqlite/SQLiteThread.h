
#pragma once

#include <pthread.h>
  struct SQLiteThread;

  struct SQLiteThread {
    pthread_t tid;
    int done;
    void *pOut;
    void *(*xTask)(void *);
    void *pIn;
  };

  int sqlite3ThreadCreate(SQLiteThread **, void *(*)(void *), void *);
  int sqlite3ThreadJoin(SQLiteThread *, void **);


