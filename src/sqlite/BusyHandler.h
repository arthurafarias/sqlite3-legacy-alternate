
#pragma once

  typedef struct BusyHandler BusyHandler;

  struct BusyHandler {
    int (*xBusyHandler)(void *, int);
    void *pBusyArg;
    int nBusy;
  };

  int sqlite3InvokeBusyHandler(BusyHandler *);


