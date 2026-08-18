#pragma once

/* Flags for the xAccess VFS method (from sqlite3.h) */
enum {
  SQLITE_ACCESS_EXISTS = 0,
  SQLITE_ACCESS_READWRITE = 1, /* Used by PRAGMA temp_store_directory */
  SQLITE_ACCESS_READ = 2,      /* Unused */
};


