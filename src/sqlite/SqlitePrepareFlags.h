#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Prepare Flags (from sqlite3.h) */
enum {
  SQLITE_PREPARE_PERSISTENT = 0x01,
  SQLITE_PREPARE_NORMALIZE  = 0x02,
  SQLITE_PREPARE_NO_VTAB    = 0x04,
  SQLITE_PREPARE_DONT_LOG   = 0x10,
  SQLITE_PREPARE_FROM_DDL   = 0x20,
};

#ifdef __cplusplus
}
#endif
