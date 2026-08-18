#pragma once

/* Conflict resolution modes (from sqlite3.h) */
enum {
  SQLITE_ROLLBACK = 1,
  SQLITE_FAIL = 3,
  SQLITE_REPLACE = 5,
};


