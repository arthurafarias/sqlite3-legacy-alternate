#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Mutex Types (from sqlite3.h) */
enum {
  SQLITE_MUTEX_FAST          = 0,
  SQLITE_MUTEX_RECURSIVE     = 1,
  SQLITE_MUTEX_STATIC_MAIN   = 2,
  SQLITE_MUTEX_STATIC_MEM    = 3,  /* sqlite3_malloc() */
  SQLITE_MUTEX_STATIC_MEM2   = 4,  /* NOT USED */
  SQLITE_MUTEX_STATIC_OPEN   = 4,  /* sqlite3BtreeOpen() */
  SQLITE_MUTEX_STATIC_PRNG   = 5,  /* sqlite3_randomness() */
  SQLITE_MUTEX_STATIC_LRU    = 6,  /* lru page list */
  SQLITE_MUTEX_STATIC_LRU2   = 7,  /* NOT USED */
  SQLITE_MUTEX_STATIC_PMEM   = 7,  /* sqlite3PageMalloc() */
  SQLITE_MUTEX_STATIC_APP1   = 8,  /* For use by application */
  SQLITE_MUTEX_STATIC_APP2   = 9,  /* For use by application */
  SQLITE_MUTEX_STATIC_APP3   = 10,  /* For use by application */
  SQLITE_MUTEX_STATIC_VFS1   = 11,  /* For use by built-in VFS */
  SQLITE_MUTEX_STATIC_VFS2   = 12,  /* For use by extension VFS */
  SQLITE_MUTEX_STATIC_VFS3   = 13,  /* For use by application VFS */
  SQLITE_MUTEX_STATIC_MASTER = 2,
  /* Not part of the public sqlite3.h; sqliteInt.h aliases this to VFS1. */
  SQLITE_MUTEX_STATIC_TEMPDIR = SQLITE_MUTEX_STATIC_VFS1,
};

#ifdef __cplusplus
}
#endif
