
#pragma once

#include "sqlite/KeyInfo.h"
#include "sqlite/Mem.h"
#include "sqlite/i64.h"
#include "sqlite/u8.h"
  typedef struct Index Index;
  typedef struct Table Table;
  typedef struct UnpackedRecord UnpackedRecord;
  typedef struct Vdbe Vdbe;
  typedef struct VdbeCursor VdbeCursor;
  typedef struct sqlite3_value sqlite3_value;

  typedef struct PreUpdate PreUpdate;

  struct PreUpdate {
    Vdbe *v;
    VdbeCursor *pCsr;
    int op;
    u8 *aRecord;
    KeyInfo *pKeyinfo;
    UnpackedRecord *pUnpacked;
    UnpackedRecord *pNewUnpacked;
    int iNewReg;
    int iBlobWrite;
    i64 iKey1;
    i64 iKey2;
    Mem oldipk;
    Mem *aNew;
    Table *pTab;
    Index *pPk;
    sqlite3_value **apDflt;
    struct {
      u8 keyinfoSpace[sizeof(KeyInfo)];
    } uKey;
  };


