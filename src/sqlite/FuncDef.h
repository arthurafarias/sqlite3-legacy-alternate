
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/i16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
  typedef struct FuncDestructor FuncDestructor;
  typedef struct sqlite3_context sqlite3_context;
  typedef struct FuncDef FuncDef;
  typedef struct sqlite3_value sqlite3_value;

  struct FuncDef {
    i16 nArg;
    u32 funcFlags;
    void *pUserData;
    FuncDef *pNext;
    void (*xSFunc)(sqlite3_context *, int, sqlite3_value **);
    void (*xFinalize)(sqlite3_context *);
    void (*xValue)(sqlite3_context *);
    void (*xInverse)(sqlite3_context *, int, sqlite3_value **);
    const char *zName;
    union {
      FuncDef *pHash;
      FuncDestructor *pDestructor;
    } u;
  };

  void sqlite3InsertBuiltinFuncs(FuncDef *, int);
  extern const FuncDef statInitFuncdef;
  extern const FuncDef statPushFuncdef;
  extern const FuncDef statGetFuncdef;
  int matchQuality(FuncDef * p, int nArg, u8 enc);

#ifdef __cplusplus
}
#endif
