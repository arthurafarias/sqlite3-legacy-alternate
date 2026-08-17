
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/Mem.h"
#include "sqlite/RecordCompare.h"
#include "sqlite/i64.h"
#include "sqlite/i8.h"
#include "sqlite/u16.h"
#include "sqlite/u8.h"
  typedef struct KeyInfo KeyInfo;

  typedef struct UnpackedRecord UnpackedRecord;

  struct UnpackedRecord {
    KeyInfo *pKeyInfo;
    Mem *aMem;
    union {
      char *z;
      i64 i;
    } u;
    int n;
    u16 nField;
    i8 default_rc;
    u8 errCode;
    i8 r1;
    i8 r2;
    u8 eqSeen;
  };

  RecordCompare sqlite3VdbeFindCompare(UnpackedRecord *);

#ifdef __cplusplus
}
#endif
