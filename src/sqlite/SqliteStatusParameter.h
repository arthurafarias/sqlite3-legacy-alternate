#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Status Parameters (from sqlite3.h) */
enum {
  SQLITE_STATUS_MEMORY_USED        = 0,
  SQLITE_STATUS_PAGECACHE_USED     = 1,
  SQLITE_STATUS_PAGECACHE_OVERFLOW = 2,
  SQLITE_STATUS_SCRATCH_USED       = 3,  /* NOT USED */
  SQLITE_STATUS_SCRATCH_OVERFLOW   = 4,  /* NOT USED */
  SQLITE_STATUS_MALLOC_SIZE        = 5,
  SQLITE_STATUS_PARSER_STACK       = 6,
  SQLITE_STATUS_PAGECACHE_SIZE     = 7,
  SQLITE_STATUS_SCRATCH_SIZE       = 8,  /* NOT USED */
  SQLITE_STATUS_MALLOC_COUNT       = 9,
};

#ifdef __cplusplus
}
#endif
