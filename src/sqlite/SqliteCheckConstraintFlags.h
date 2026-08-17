#pragma once
#ifdef __cplusplus
extern "C" {
#endif
/* CHECK Constraint Walker eCode Bit Values (from the sqlite3 amalgamation's
   Expr.c, internal/private - not part of the public sqlite3.h API and not
   guaranteed stable across versions). */
enum {
  CKCNSTRNT_COLUMN = 0x01, /* CHECK constraint uses a changing column */
  CKCNSTRNT_ROWID = 0x02,  /* CHECK constraint references the ROWID */
};

#ifdef __cplusplus
}
#endif
