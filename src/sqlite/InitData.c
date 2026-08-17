#define _GNU_SOURCE 1

#include "sqlite/InitData.h"

#include "sqlite/sqlite3.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
void corruptSchema(InitData *pData, char **azObj, const char *zExtra) {
  sqlite3 *db = pData->db;
  if (db->mallocFailed) {
    pData->rc = 7;
  } else if (pData->pzErrMsg[0] != 0) {

  } else if (pData->mInitFlags & (0x0007)) {
    static const char *azAlterType[] = {"rename", "drop column", "add column", "drop constraint"};
    *pData->pzErrMsg = sqlite3MPrintf(db, "error in %s %s after %s: %s", azObj[0], azObj[1], azAlterType[(pData->mInitFlags & 0x0007) - 1], zExtra);
    pData->rc = 1;
  } else if (db->flags & 0x00000001) {
    pData->rc = sqlite3CorruptError(147952);
  } else {
    char *z;
    const char *zObj = azObj[1] ? azObj[1] : "?";
    z = sqlite3MPrintf(db, "malformed database schema (%s)", zObj);
    if (zExtra && zExtra[0])
      z = sqlite3MPrintf(db, "%z - %s", z, zExtra);
    *pData->pzErrMsg = z;
    pData->rc = sqlite3CorruptError(147959);
  }
}
