#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/sqlite3_int64.h"
sqlite3_int64 sqlite3_soft_heap_limit64(sqlite3_int64 N);
sqlite3_int64 sqlite3_hard_heap_limit64(sqlite3_int64 N);
void sqlite3_soft_heap_limit(int N);

#ifdef __cplusplus
}
#endif