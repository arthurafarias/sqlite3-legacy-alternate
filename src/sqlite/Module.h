
#pragma once
#ifdef __cplusplus
extern C {
#endif
  typedef struct Table Table;
  typedef struct sqlite3_module sqlite3_module;
  typedef struct Module Module;

  struct Module {
    const sqlite3_module *pModule;
    const char *zName;
    int nRefModule;
    void *pAux;
    void (*xDestroy)(void *);
    Table *pEpoTab;
  };

#ifdef __cplusplus
}
#endif
