
#pragma once

#include "sqlite/u8.h"
  struct Module;
  struct sqlite3;
  struct sqlite3_vtab;

  struct VTable;

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


