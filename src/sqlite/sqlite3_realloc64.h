#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "sqlite/sqlite3_uint64.h"

void *sqlite3_realloc64(void *, sqlite3_uint64);

#ifdef __cplusplus
}
#endif
