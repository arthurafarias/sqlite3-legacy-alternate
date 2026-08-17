
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/u8.h"
  typedef struct Module Module;
  typedef struct sqlite3 sqlite3;
  typedef struct sqlite3_vtab sqlite3_vtab;

  typedef struct VTable VTable;

  struct VTable {
    sqlite3 *db;
    Module *pMod;
    sqlite3_vtab *pVtab;
    int nRef;
    u8 bConstraint;
    u8 bAllSchemas;
    u8 eVtabRisk;
    int iSavepoint;
    VTable *pNext;
  };

  void sqlite3VtabLock(VTable *);
  void sqlite3VtabUnlock(VTable *);

#ifdef __cplusplus
}
#endif
