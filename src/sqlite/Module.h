
#pragma once

  struct Table;
  struct sqlite3_module;
  struct Module;

  struct Module {
    const sqlite3_module *pModule;
    const char *zName;
    int nRefModule;
    void *pAux;
    void (*xDestroy)(void *);
    Table *pEpoTab;
  };


