#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Conflict resolution modes (from sqlite3.h) */
enum {
  SQLITE_ROLLBACK = 1,
  SQLITE_FAIL     = 3,
  SQLITE_REPLACE  = 5,
};

#ifdef __cplusplus
}
#endif
