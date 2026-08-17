#pragma once
#ifdef __cplusplus
extern "C" {
#endif
/* Virtual Table Configuration Options (from sqlite3.h) */
enum {
  SQLITE_VTAB_CONSTRAINT_SUPPORT = 1,
  SQLITE_VTAB_INNOCUOUS = 2,
  SQLITE_VTAB_DIRECTONLY = 3,
  SQLITE_VTAB_USES_ALL_SCHEMAS = 4,
};

#ifdef __cplusplus
}
#endif
