
#pragma once
#ifdef __cplusplus
extern C {
#endif

  typedef struct BusyHandler BusyHandler;

  struct BusyHandler {
    int (*xBusyHandler)(void *, int);
    void *pBusyArg;
    int nBusy;
  };

  int sqlite3InvokeBusyHandler(BusyHandler *);

#ifdef __cplusplus
}
#endif
