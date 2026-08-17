#pragma once
#ifdef __cplusplus
extern "C" {
#endif
/* Prepared Statement Scan Status Opcodes (from sqlite3.h) */
enum {
  SQLITE_SCANSTAT_NLOOP = 0,
  SQLITE_SCANSTAT_NVISIT = 1,
  SQLITE_SCANSTAT_EST = 2,
  SQLITE_SCANSTAT_NAME = 3,
  SQLITE_SCANSTAT_EXPLAIN = 4,
  SQLITE_SCANSTAT_SELECTID = 5,
  SQLITE_SCANSTAT_PARENTID = 6,
  SQLITE_SCANSTAT_NCYCLE = 7,
};

#ifdef __cplusplus
}
#endif
