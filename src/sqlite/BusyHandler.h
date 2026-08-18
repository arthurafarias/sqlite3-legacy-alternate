
#pragma once

  struct BusyHandler;

  struct BusyHandler {
    int (*xBusyHandler)(void *, int);
    void *pBusyArg;
    int nBusy;
  };

  int sqlite3InvokeBusyHandler(BusyHandler *);


