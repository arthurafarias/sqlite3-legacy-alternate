#pragma once

/* Configuration Options (from sqlite3.h) */
enum {
  SQLITE_CONFIG_SINGLETHREAD = 1,         /* nil */
  SQLITE_CONFIG_MULTITHREAD = 2,          /* nil */
  SQLITE_CONFIG_SERIALIZED = 3,           /* nil */
  SQLITE_CONFIG_MALLOC = 4,               /* sqlite3_mem_methods* */
  SQLITE_CONFIG_GETMALLOC = 5,            /* sqlite3_mem_methods* */
  SQLITE_CONFIG_SCRATCH = 6,              /* No longer used */
  SQLITE_CONFIG_PAGECACHE = 7,            /* void*, int sz, int N */
  SQLITE_CONFIG_HEAP = 8,                 /* void*, int nByte, int min */
  SQLITE_CONFIG_MEMSTATUS = 9,            /* boolean */
  SQLITE_CONFIG_MUTEX = 10,               /* sqlite3_mutex_methods* */
  SQLITE_CONFIG_GETMUTEX = 11,            /* sqlite3_mutex_methods* */
  SQLITE_CONFIG_LOOKASIDE = 13,           /* int int */
  SQLITE_CONFIG_PCACHE = 14,              /* no-op */
  SQLITE_CONFIG_GETPCACHE = 15,           /* no-op */
  SQLITE_CONFIG_LOG = 16,                 /* xFunc, void* */
  SQLITE_CONFIG_URI = 17,                 /* int */
  SQLITE_CONFIG_PCACHE2 = 18,             /* sqlite3_pcache_methods2* */
  SQLITE_CONFIG_GETPCACHE2 = 19,          /* sqlite3_pcache_methods2* */
  SQLITE_CONFIG_COVERING_INDEX_SCAN = 20, /* int */
  SQLITE_CONFIG_SQLLOG = 21,              /* xSqllog, void* */
  SQLITE_CONFIG_MMAP_SIZE = 22,           /* sqlite3_int64, sqlite3_int64 */
  SQLITE_CONFIG_WIN32_HEAPSIZE = 23,      /* int nByte */
  SQLITE_CONFIG_PCACHE_HDRSZ = 24,        /* int *psz */
  SQLITE_CONFIG_PMASZ = 25,               /* unsigned int szPma */
  SQLITE_CONFIG_STMTJRNL_SPILL = 26,      /* int nByte */
  SQLITE_CONFIG_SMALL_MALLOC = 27,        /* boolean */
  SQLITE_CONFIG_SORTERREF_SIZE = 28,      /* int nByte */
  SQLITE_CONFIG_MEMDB_MAXSIZE = 29,       /* sqlite3_int64 */
  SQLITE_CONFIG_ROWID_IN_VIEW = 30,       /* int* */
};


