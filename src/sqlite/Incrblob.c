#define _GNU_SOURCE 1
#include "sqlite/Incrblob.h"
#include "sqlite/BtCursor.h"
#include "sqlite/Mem.h"
#include "sqlite/Vdbe.h"
#include "sqlite/VdbeCursor.h"
#include "sqlite/i16.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_stmt.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/SqliteResultCode.h"
int blobSeekToRow(Incrblob *p, sqlite3_int64 iRow, char **pzErr) {
  int rc;
  char *zErr = 0;
  Vdbe *v = (Vdbe *)p->pStmt;

  sqlite3VdbeMemSetInt64(&v->aMem[1], iRow);

  if (v->pc > 4) {
    v->pc = 4;

    rc = sqlite3VdbeExec(v);
  } else {
    rc = sqlite3_step(p->pStmt);
  }
  if (rc == SQLITE_ROW) {
    VdbeCursor *pC = v->apCsr[0];
    u32 type;

    type = pC->nHdrParsed > p->iCol ? pC->aType[p->iCol] : 0;
    if (type < 12) {
      zErr = sqlite3MPrintf(p->db, "cannot open value of type %s", type == 0 ? "null" : type == 7 ? "real" : "integer");
      rc = SQLITE_ERROR;
      sqlite3_finalize(p->pStmt);
      p->pStmt = 0;
    } else {
      p->iOffset = pC->aType[p->iCol + pC->nField];
      p->nByte = sqlite3VdbeSerialTypeLen(type);
      p->pCsr = pC->uc.pCursor;
      sqlite3BtreeIncrblobCursor(p->pCsr);
    }
  }

  if (rc == SQLITE_ROW) {
    rc = SQLITE_OK;
  } else if (p->pStmt) {
    rc = sqlite3_finalize(p->pStmt);
    p->pStmt = 0;
    if (rc == SQLITE_OK) {
      zErr = sqlite3MPrintf(p->db, "no such rowid: %lld", iRow);
      rc = SQLITE_ERROR;
    } else {
      zErr = sqlite3MPrintf(p->db, "%s", sqlite3_errmsg(p->db));
    }
  }

  *pzErr = zErr;
  return rc;
}
