#pragma once

#include "sqlite/u64.h"
typedef short LogEst;

LogEst sqlite3LogEst(u64);
LogEst sqlite3LogEstAdd(LogEst, LogEst);
LogEst sqlite3LogEstFromDouble(double);
u64 sqlite3LogEstToInt(LogEst);

