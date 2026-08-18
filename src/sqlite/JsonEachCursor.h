
#pragma once

#include "sqlite/JsonParse.h"
#include "sqlite/JsonString.h"
#include "sqlite/sqlite3_vtab_cursor.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
  struct JsonParent;

  struct JsonEachCursor;
  struct JsonEachCursor {
    sqlite3_vtab_cursor base;
    u32 iRowid;
    u32 i;
    u32 iEnd;
    u32 nRoot;
    u8 eType;
    u8 bRecursive;
    u8 eMode;
    u32 nParent;
    u32 nParentAlloc;
    JsonParent *aParent;
    sqlite3 *db;
    JsonString path;
    JsonParse sParse;
  };

  void jsonEachCursorReset(JsonEachCursor * p);
  int jsonSkipLabel(JsonEachCursor * p);
  void jsonAppendPathName(JsonEachCursor * p);
  int jsonEachPathLength(JsonEachCursor * p);


