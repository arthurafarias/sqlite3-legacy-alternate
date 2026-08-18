#pragma once

typedef struct sqlite3 sqlite3;
typedef struct sqlite3_api_routines sqlite3_api_routines;

typedef int (*sqlite3_loadext_entry)(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pThunk);


