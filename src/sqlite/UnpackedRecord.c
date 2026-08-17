#define _GNU_SOURCE 1

#include "sqlite/UnpackedRecord.h"

#include "sqlite/CollSeq.h"
#include "sqlite/KeyInfo.h"
#include "sqlite/Mem.h"
#include "sqlite/RecordCompare.h"
#include "sqlite/Vdbe.h"
#include "sqlite/i64.h"
#include "sqlite/i8.h"
#include "sqlite/sqlite3_value.h"
#include "sqlite/u16.h"
#include "sqlite/u8.h"
RecordCompare sqlite3VdbeFindCompare(UnpackedRecord *p) {

  if (p->pKeyInfo->nAllField <= 13) {
    int flags = p->aMem[0].flags;
    if (p->pKeyInfo->aSortFlags[0]) {
      if (p->pKeyInfo->aSortFlags[0] & 0x02) {
        return sqlite3VdbeRecordCompare;
      }
      p->r1 = 1;
      p->r2 = -1;
    } else {
      p->r1 = -1;
      p->r2 = 1;
    }
    if ((flags & 0x0004)) {
      p->u.i = p->aMem[0].u.i;
      return vdbeRecordCompareInt;
    };
    ;
    ;
    if ((flags & (0x0008 | 0x0020 | 0x0001 | 0x0010)) == 0 && p->pKeyInfo->aColl[0] == 0) {


      p->u.z = p->aMem[0].z;
      p->n = p->aMem[0].n;
      return vdbeRecordCompareString;
    }
  }

  return sqlite3VdbeRecordCompare;
}
