#pragma once
#ifdef __cplusplus
extern "C" {
#endif
/* Special Index.aiColumn[] Values (from the sqlite3 amalgamation's
   sqliteInt.h, internal/private - not part of the public sqlite3.h API
   and not guaranteed stable across versions). */
enum {
  XN_ROWID = -1, /* Indexed column is the rowid */
  XN_EXPR = -2,  /* Indexed column is an expression */
};

#ifdef __cplusplus
}
#endif
