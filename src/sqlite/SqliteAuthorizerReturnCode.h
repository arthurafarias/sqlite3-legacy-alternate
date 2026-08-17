#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Authorizer Return Codes (from sqlite3.h) */
enum {
  SQLITE_DENY   = 1,  /* Abort the SQL statement with an error */
  SQLITE_IGNORE = 2,  /* Don't allow access, but don't generate an error */
};

#ifdef __cplusplus
}
#endif
