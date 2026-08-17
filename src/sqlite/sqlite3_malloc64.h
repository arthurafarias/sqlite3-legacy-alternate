#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "sqlite/sqlite3_uint64.h"

void *sqlite3_malloc64(sqlite3_uint64);

#ifdef __cplusplus
}
#endif
