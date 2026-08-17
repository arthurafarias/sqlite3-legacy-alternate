
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include <pthread.h>
  typedef struct SQLiteThread SQLiteThread;

  struct SQLiteThread {
    pthread_t tid;
    int done;
    void *pOut;
    void *(*xTask)(void *);
    void *pIn;
  };

  int sqlite3ThreadCreate(SQLiteThread **, void *(*)(void *), void *);
  int sqlite3ThreadJoin(SQLiteThread *, void **);

#ifdef __cplusplus
}
#endif
