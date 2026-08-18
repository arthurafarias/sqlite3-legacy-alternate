#define _GNU_SOURCE 1
#include "sqlite/Trigger.h"
#include "sqlite/Hash.h"
#include "sqlite/Schema.h"
#include "sqlite/Table.h"
Table *tableOfTrigger(Trigger *pTrigger) {
  return (Table*)(sqlite3HashFind(&pTrigger->pTabSchema->tblHash, pTrigger->table));
}
