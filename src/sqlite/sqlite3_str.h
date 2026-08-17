
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include <stdarg.h>
#include "sqlite/sqlite3_int64.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
  typedef struct sqlite3 sqlite3;
  typedef struct sqlite3_str sqlite3_str;

  struct sqlite3_str {
    sqlite3 *db;
    char *zText;
    u32 nAlloc;
    u32 mxAlloc;
    u32 nChar;
    u8 accError;
    u8 printfFlags;
  };

  char *sqlite3_str_finish(sqlite3_str *);
  void sqlite3_str_free(sqlite3_str *);
  void sqlite3_str_appendf(sqlite3_str *, const char *zFormat, ...);
  void sqlite3_str_vappendf(sqlite3_str *, const char *zFormat, va_list);
  void sqlite3_str_append(sqlite3_str *, const char *zIn, int N);
  void sqlite3_str_appendall(sqlite3_str *, const char *zIn);
  void sqlite3_str_appendchar(sqlite3_str *, int N, char C);
  void sqlite3_str_reset(sqlite3_str *);
  void sqlite3_str_truncate(sqlite3_str *, int N);
  int sqlite3_str_errcode(sqlite3_str *);
  int sqlite3_str_length(sqlite3_str *);
  char *sqlite3_str_value(sqlite3_str *);

  char *printfTempBuf(sqlite3_str * pAccum, sqlite3_int64 n);
  extern sqlite3_str sqlite3OomStr;

#ifdef __cplusplus
}
#endif
