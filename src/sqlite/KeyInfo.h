
#pragma once

#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
  typedef struct CollSeq CollSeq;
  typedef struct UnpackedRecord UnpackedRecord;
  typedef struct sqlite3 sqlite3;
  typedef struct KeyInfo KeyInfo;

  struct KeyInfo {
    u32 nRef;
    u8 enc;
    u16 nKeyField;
    u16 nAllField;
    sqlite3 *db;
    u8 *aSortFlags;
    CollSeq *aColl[1];
  };

  UnpackedRecord *sqlite3VdbeAllocUnpackedRecord(KeyInfo *);
  void sqlite3KeyInfoUnref(KeyInfo *);
  KeyInfo *sqlite3KeyInfoRef(KeyInfo *);


