#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sqlite/u64.h"

typedef short LogEst;

LogEst sqlite3LogEst(u64);
LogEst sqlite3LogEstAdd(LogEst, LogEst);
LogEst sqlite3LogEstFromDouble(double);
u64 sqlite3LogEstToInt(LogEst);

#ifdef __cplusplus
}
#endif