#pragma once
#include "sqlite/Fts5Context.h"
#include "sqlite/Fts5ExtensionApi.h"
struct sqlite3_context;
struct sqlite3_value;


typedef void (*fts5_extension_function)(const Fts5ExtensionApi *pApi, Fts5Context *pFts, sqlite3_context *pCtx,
                                        int nVal, sqlite3_value **apVal);

