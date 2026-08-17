
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "sqlite/PGroup.h"
#include "sqlite/PgFreeslot.h"
#include "sqlite/sqlite3_mutex.h"

typedef struct PCacheGlobal PCacheGlobal;

struct PCacheGlobal {
  PGroup grp;

  int isInit;
  int separateCache;
  int nInitPage;
  int szSlot;
  int nSlot;
  int nReserve;
  void *pStart, *pEnd;

  sqlite3_mutex *mutex;
  PgFreeslot *pFree;
  int nFreeSlot;
  int bUnderPressure;
};

extern struct PCacheGlobal pcache1_g;

#ifdef __cplusplus
}
#endif
