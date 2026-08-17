#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/sqlite3_int64.h"
sqlite3_int64 sqlite3_memory_highwater(int resetFlag);

#ifdef __cplusplus
}
#endif
