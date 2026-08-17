#pragma once
#include "sqlite/Fts5Context.h"
#include "sqlite/Fts5ExtensionApi.h"
typedef struct sqlite3_context sqlite3_context;
typedef struct sqlite3_value sqlite3_value;

#ifdef __cplusplus
extern "C" {
#endif
typedef void (*fts5_extension_function)(const Fts5ExtensionApi *pApi, Fts5Context *pFts, sqlite3_context *pCtx,
                                        int nVal, sqlite3_value **apVal);

#ifdef __cplusplus
}
#endif