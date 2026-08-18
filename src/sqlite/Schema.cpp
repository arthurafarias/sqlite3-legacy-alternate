#define _GNU_SOURCE 1
#include <string.h>
#include "sqlite/Schema.h"
#include "sqlite/Hash.h"
#include "sqlite/HashElem.h"
#include "sqlite/Table.h"
#include "sqlite/Trigger.h"
#include "sqlite/sqlite3.h"
#include "sqlite/u16.h"
void sqlite3SchemaClear(void *p) {
  Hash temp1;
  Hash temp2;
  HashElem *pElem;
  Schema *pSchema = (Schema *)p;
  sqlite3 xdb;

  memset(&xdb, 0, sizeof(xdb));
  temp1 = pSchema->tblHash;
  temp2 = pSchema->trigHash;
  sqlite3HashInit(&pSchema->trigHash);
  sqlite3HashClear(&pSchema->idxHash);
  for (pElem = ((&temp2)->first); pElem; pElem = ((pElem)->next)) {
    sqlite3DeleteTrigger(&xdb, (Trigger *)((pElem)->data));
  }

  sqlite3HashClear(&temp2);
  sqlite3HashInit(&pSchema->tblHash);
  for (pElem = ((&temp1)->first); pElem; pElem = ((pElem)->next)) {
    Table *pTab = (Table*)(((pElem)->data));
    sqlite3DeleteTable(&xdb, pTab);
  }
  sqlite3HashClear(&temp1);
  sqlite3HashClear(&pSchema->fkeyHash);
  pSchema->pSeqTab = 0;
  if (pSchema->schemaFlags & 0x0001) {
    pSchema->iGeneration++;
  }
  pSchema->schemaFlags &= ~(0x0001 | 0x0008);
}
