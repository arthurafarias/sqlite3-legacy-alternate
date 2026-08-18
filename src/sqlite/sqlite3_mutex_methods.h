
#pragma once

  typedef struct sqlite3_mutex_methods sqlite3_mutex_methods;
  typedef struct sqlite3_mutex sqlite3_mutex;

  struct sqlite3_mutex_methods {
    int (*xMutexInit)(void);
    int (*xMutexEnd)(void);
    sqlite3_mutex *(*xMutexAlloc)(int);
    void (*xMutexFree)(sqlite3_mutex *);
    void (*xMutexEnter)(sqlite3_mutex *);
    int (*xMutexTry)(sqlite3_mutex *);
    void (*xMutexLeave)(sqlite3_mutex *);
    int (*xMutexHeld)(sqlite3_mutex *);
    int (*xMutexNotheld)(sqlite3_mutex *);
  };


