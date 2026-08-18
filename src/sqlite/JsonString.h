
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
  typedef struct JsonParse JsonParse;
  typedef struct sqlite3_context sqlite3_context;
  typedef struct sqlite3_value sqlite3_value;
  typedef struct JsonString JsonString;

  struct JsonString {
    sqlite3_context *pCtx;
    char *zBuf;
    u64 nAlloc;
    u64 nUsed;
    u8 bStatic;
    u8 eErr;
    char zSpace[100];
  };

  void jsonReturnStringAsBlob(JsonString *);
  void jsonStringZero(JsonString * p);
  void jsonStringInit(JsonString * p, sqlite3_context * pCtx);
  void jsonStringReset(JsonString * p);
  void jsonStringOom(JsonString * p);
  void jsonStringTooDeep(JsonString * p);
  int jsonStringGrow(JsonString * p, u32 N);
  __attribute__((noinline)) void jsonStringExpandAndAppend(JsonString * p, const char *zIn, u32 N);
  void jsonAppendRaw(JsonString * p, const char *zIn, u32 N);
  void jsonAppendRawNZ(JsonString * p, const char *zIn, u32 N);
  __attribute__((noinline)) void jsonAppendCharExpand(JsonString * p, char c);
  void jsonAppendChar(JsonString * p, char c);
  void jsonStringTrimOneChar(JsonString * p);
  int jsonStringTerminate(JsonString * p);
  void jsonAppendSeparator(JsonString * p);
  void jsonAppendControlChar(JsonString * p, u8 c);
  void jsonAppendString(JsonString * p, const char *zIn, u32 N);
  void jsonAppendSqlValue(JsonString * p, sqlite3_value * pValue);
  void jsonReturnString(JsonString * p, JsonParse * pParse, sqlite3_context * ctx);

#ifdef __cplusplus
}
#endif
