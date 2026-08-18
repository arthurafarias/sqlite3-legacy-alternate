#define _GNU_SOURCE 1
#include "sqlite/VTable.h"
#include "sqlite/Module.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_module.h"
#include "sqlite/sqlite3_vtab.h"
void sqlite3VtabLock(VTable *pVTab) {
  pVTab->nRef++;
}

void sqlite3VtabUnlock(VTable *pVTab) {
  sqlite3 *db = pVTab->db;

  pVTab->nRef--;
  if (pVTab->nRef == 0) {
    sqlite3_vtab *p = pVTab->pVtab;
    if (p) {
      p->pModule->xDisconnect(p);
    }
    sqlite3VtabModuleUnref(pVTab->db, pVTab->pMod);
    sqlite3DbFree(db, pVTab);
  }
}
