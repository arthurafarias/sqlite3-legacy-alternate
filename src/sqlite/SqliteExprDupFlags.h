#pragma once
#ifdef __cplusplus
extern "C" {
#endif
/* sqlite3ExprDup() Flags (from the sqlite3 amalgamation's sqliteInt.h,
   internal/private - not part of the public sqlite3.h API and not
   guaranteed stable across versions). */
enum {
  EXPRDUP_REDUCE = 0x0001, /* Used reduced-size Expr nodes */
};

#ifdef __cplusplus
}
#endif
