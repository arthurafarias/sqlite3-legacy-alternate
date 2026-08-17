#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Function Flags (from sqlite3.h) */
enum {
  SQLITE_DETERMINISTIC  = 0x000000800,
  SQLITE_DIRECTONLY     = 0x000080000,
  SQLITE_SUBTYPE        = 0x000100000,
  SQLITE_INNOCUOUS      = 0x000200000,
  SQLITE_RESULT_SUBTYPE = 0x001000000,
  SQLITE_SELFORDER1     = 0x002000000,
};

#ifdef __cplusplus
}
#endif
