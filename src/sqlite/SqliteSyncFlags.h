#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Synchronization Type Flags (from sqlite3.h) */
enum {
  SQLITE_SYNC_NORMAL   = 0x00002,
  SQLITE_SYNC_FULL     = 0x00003,
  SQLITE_SYNC_DATAONLY = 0x00010,
};

#ifdef __cplusplus
}
#endif
