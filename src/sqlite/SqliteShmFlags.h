#pragma once
#ifdef __cplusplus
extern "C" {
#endif
/* Flags for the xShmLock VFS method / Maximum xShmLock index (from sqlite3.h) */
enum {
  SQLITE_SHM_UNLOCK = 1,
  SQLITE_SHM_LOCK = 2,
  SQLITE_SHM_SHARED = 4,
  SQLITE_SHM_EXCLUSIVE = 8,
  SQLITE_SHM_NLOCK = 8,
};

#ifdef __cplusplus
}
#endif
