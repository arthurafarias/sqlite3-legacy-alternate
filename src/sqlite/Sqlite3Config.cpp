#define _GNU_SOURCE 1
#include "sqlite/Sqlite3Config.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_mem_methods.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/sqlite3_mutex_methods.h"
#include "sqlite/sqlite3_pcache_methods2.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
struct Sqlite3Config sqlite3Config = {
    1,
    1,
    1 == 1,
    0,
    1,
    0,
    1,
    0x7ffffffe,
    0,
    1200,
    40,
    (64 * 1024),
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    (void *)0,
    0,
    0,
    0,
    0,
    0x7fff0000,
    (void *)0,
    0,
    20,
    0,
    0,
    250,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1073741824,
    0,
    0,
    0,
    0x7ffffffe,
    0x7fffffff,
    0,
};
