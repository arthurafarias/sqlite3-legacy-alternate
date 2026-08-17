
#pragma once
#ifdef __cplusplus
extern C {
#endif

  typedef struct sqlite3_mem_methods sqlite3_mem_methods;

  struct sqlite3_mem_methods {
    void *(*xMalloc)(int);
    void (*xFree)(void *);
    void *(*xRealloc)(void *, int);
    int (*xSize)(void *);
    int (*xRoundup)(int);
    int (*xInit)(void *);
    void (*xShutdown)(void *);
    void *pAppData;
  };

#ifdef __cplusplus
}
#endif
