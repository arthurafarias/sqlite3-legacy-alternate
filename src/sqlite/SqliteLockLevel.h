#pragma once

/* File Locking Levels (from sqlite3.h) */
enum {
  SQLITE_LOCK_NONE = 0,      /* xUnlock() only */
  SQLITE_LOCK_SHARED = 1,    /* xLock() or xUnlock() */
  SQLITE_LOCK_RESERVED = 2,  /* xLock() only */
  SQLITE_LOCK_PENDING = 3,   /* xLock() only */
  SQLITE_LOCK_EXCLUSIVE = 4, /* xLock() only */
};


