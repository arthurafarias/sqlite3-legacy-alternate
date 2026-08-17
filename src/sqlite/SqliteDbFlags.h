#pragma once
#ifdef __cplusplus
extern "C" {
#endif
/* sqlite3.flags Bit Values (from the sqlite3 amalgamation's sqliteInt.h,
   internal/private - not part of the public sqlite3.h API and not
   guaranteed stable across versions). Only the subset referenced by this
   library's sources is listed here; extend as more of them are needed. */
enum {
  SQLITE_EnableQPSG = 0x00800000, /* Query Planner Stability Guarantee */
};

#ifdef __cplusplus
}
#endif
