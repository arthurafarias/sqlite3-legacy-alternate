#pragma once
#include "sqlite/sqlite3_destructor_type.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Constants Defining Special Destructor Behavior (from sqlite3.h) */
static const sqlite3_destructor_type SQLITE_STATIC = ((sqlite3_destructor_type)0);
static const sqlite3_destructor_type SQLITE_TRANSIENT = ((sqlite3_destructor_type)-1);

#ifdef __cplusplus
}
#endif
