#pragma once

struct sqlite3;
struct sqlite3_api_routines;

typedef int (*sqlite3_loadext_entry)(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pThunk);


