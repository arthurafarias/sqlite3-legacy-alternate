
#pragma once

#include "sqlite/FuncDef.h"
#include "sqlite/MemValue.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
struct JsonParse;

struct sqlite3;
struct sqlite3_value {
  MemValue u;
  char *z;
  int n;
  u16 flags;
  u8 enc;
  u8 eSubtype;

  sqlite3 *db;
  int szMalloc;
  u32 uTemp;
  char *zMalloc;
  void (*xDel)(void *);
};

const void *sqlite3_value_blob(sqlite3_value *);
double sqlite3_value_double(sqlite3_value *);
int sqlite3_value_int(sqlite3_value *);
sqlite3_int64 sqlite3_value_int64(sqlite3_value *);
void *sqlite3_value_pointer(sqlite3_value *, const char *);
const unsigned char *sqlite3_value_text(sqlite3_value *);
const void *sqlite3_value_text16(sqlite3_value *);
const void *sqlite3_value_text16le(sqlite3_value *);
const void *sqlite3_value_text16be(sqlite3_value *);
int sqlite3_value_bytes(sqlite3_value *);
int sqlite3_value_bytes16(sqlite3_value *);
int sqlite3_value_type(sqlite3_value *);
int sqlite3_value_numeric_type(sqlite3_value *);
int sqlite3_value_nochange(sqlite3_value *);
int sqlite3_value_frombind(sqlite3_value *);
int sqlite3_value_encoding(sqlite3_value *);
unsigned int sqlite3_value_subtype(sqlite3_value *);
sqlite3_value *sqlite3_value_dup(const sqlite3_value *);
void sqlite3_value_free(sqlite3_value *);
int sqlite3_vtab_in_first(sqlite3_value *pVal, sqlite3_value **ppOut);
int sqlite3_vtab_in_next(sqlite3_value *pVal, sqlite3_value **ppOut);
void sqlite3MemSetArrayInt64(sqlite3_value *aMem, int iIdx, i64 val);
const void *sqlite3ValueText(sqlite3_value *, u8);
int sqlite3ValueBytes(sqlite3_value *, u8);
void sqlite3ValueSetStr(sqlite3_value *, int, const void *, u8, void (*)(void *));
void sqlite3ValueSetNull(sqlite3_value *);
void sqlite3ValueFree(sqlite3_value *);
void sqlite3ValueApplyAffinity(sqlite3_value *, u8, u8);

int sqlite3ValueIsOfClass(const sqlite3_value *, void (*)(void *));
__attribute__((noinline)) const void *valueToText(sqlite3_value *pVal, u8 enc);
__attribute__((noinline)) int valueBytes(sqlite3_value *pVal, u8 enc);
int valueFromValueList(sqlite3_value *pVal, sqlite3_value **ppOut, int bNext);
int jsonArgIsJsonb(sqlite3_value *pJson, JsonParse *p);


