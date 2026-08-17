#define _GNU_SOURCE 1

#include "sqlite/KeyInfo.h"

#include "sqlite/Mem.h"
#include "sqlite/UnpackedRecord.h"
#include "sqlite/sqlite3.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
UnpackedRecord *sqlite3VdbeAllocUnpackedRecord(KeyInfo *pKeyInfo) {
  UnpackedRecord *p;
  u64 nByte;

  nByte = (sizeof(UnpackedRecord)) + sizeof(Mem) * (pKeyInfo->nKeyField + 1);
  p = (UnpackedRecord *)sqlite3DbMallocRaw(pKeyInfo->db, nByte);
  if (!p)
    return 0;
  p->aMem = (Mem *)&((char *)p)[(sizeof(UnpackedRecord))];
  p->pKeyInfo = pKeyInfo;
  p->nField = pKeyInfo->nKeyField + 1;
  return p;
}

void sqlite3KeyInfoUnref(KeyInfo *p) {
  if (p) {

    ((void)(0))

        ;

    ((void)(0))

        ;
    p->nRef--;
    if (p->nRef == 0)
      sqlite3DbNNFreeNN(p->db, p);
  }
}

KeyInfo *sqlite3KeyInfoRef(KeyInfo *p) {
  if (p) {

    ((void)(0))

        ;
    p->nRef++;
  }
  return p;
}
