#define _GNU_SOURCE 1
#include "sqlite/StatAccum.h"
#include "sqlite/sqlite3.h"
void statAccumDestructor(void *pOld) {
  StatAccum *p = (StatAccum *)pOld;

  sqlite3DbFree(p->db, p);
}
