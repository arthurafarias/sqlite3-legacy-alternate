
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include <pthread.h>
  typedef struct sqlite3_mutex sqlite3_mutex;

  struct sqlite3_mutex {
    pthread_mutex_t mutex;
  };

  sqlite3_mutex *sqlite3_mutex_alloc(int);
  void sqlite3_mutex_free(sqlite3_mutex *);
  void sqlite3_mutex_enter(sqlite3_mutex *);
  int sqlite3_mutex_try(sqlite3_mutex *);
  void sqlite3_mutex_leave(sqlite3_mutex *);
  int sqlite3_mutex_held(sqlite3_mutex *);

  int sqlite3_mutex_held(sqlite3_mutex *);
  int sqlite3_mutex_notheld(sqlite3_mutex *);

  void noopMutexFree(sqlite3_mutex * p);
  void noopMutexEnter(sqlite3_mutex * p);
  int noopMutexTry(sqlite3_mutex * p);
  void noopMutexLeave(sqlite3_mutex * p);
  void pthreadMutexFree(sqlite3_mutex * p);
  void pthreadMutexEnter(sqlite3_mutex * p);
  int pthreadMutexTry(sqlite3_mutex * p);
  void pthreadMutexLeave(sqlite3_mutex * p);
  extern sqlite3_mutex *unixBigLock;

#ifdef __cplusplus
}
#endif
