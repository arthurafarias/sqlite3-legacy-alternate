#define _GNU_SOURCE 1
#include "sqlite/sqlite3_blob.h"
#include "sqlite/BtCursor.h"
#include "sqlite/Incrblob.h"
#include "sqlite/Vdbe.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/sqlite3_stmt.h"
#include "sqlite/u32.h"
#include "sqlite/SqliteResultCode.h"
int sqlite3_blob_close(sqlite3_blob *pBlob) {
  Incrblob *p = (Incrblob *)pBlob;
  int rc;
  sqlite3 *db;

  if (p) {
    sqlite3_stmt *pStmt = p->pStmt;
    db = p->db;
    sqlite3_mutex_enter(db->mutex);
    sqlite3DbFree(db, p);
    sqlite3_mutex_leave(db->mutex);
    rc = sqlite3_finalize(pStmt);
  } else {
    rc = SQLITE_OK;
  }
  return rc;
}

int blobReadWrite(sqlite3_blob *pBlob, void *z, int n, int iOffset, int (*xCall)(BtCursor *, u32, u32, void *)) {
  int rc = SQLITE_OK;
  Incrblob *p = (Incrblob *)pBlob;
  Vdbe *v;
  sqlite3 *db;

  if (p == 0)
    return sqlite3MisuseError(106384);
  db = p->db;
  sqlite3_mutex_enter(db->mutex);
  v = (Vdbe *)p->pStmt;

  if (n < 0 || iOffset < 0 || ((sqlite3_int64)iOffset + n) > p->nByte) {
    rc = SQLITE_ERROR;
  } else if (v == 0) {
    rc = SQLITE_ABORT;
  } else {
    sqlite3BtreeEnterCursor(p->pCsr);

    rc = xCall(p->pCsr, iOffset + p->iOffset, n, z);

    sqlite3BtreeLeaveCursor(p->pCsr);
    if (rc == SQLITE_ABORT) {
      sqlite3VdbeFinalize(v);
      p->pStmt = 0;
    } else {
      v->rc = rc;
    }
  }
  sqlite3Error(db, rc);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

int sqlite3_blob_read(sqlite3_blob *pBlob, void *z, int n, int iOffset) {
  return blobReadWrite(pBlob, z, n, iOffset, sqlite3BtreePayloadChecked);
}

int sqlite3_blob_write(sqlite3_blob *pBlob, const void *z, int n, int iOffset) {
  return blobReadWrite(pBlob, (void *)z, n, iOffset, sqlite3BtreePutData);
}

int sqlite3_blob_bytes(sqlite3_blob *pBlob) {
  Incrblob *p = (Incrblob *)pBlob;
  return (p && p->pStmt) ? p->nByte : 0;
}

int sqlite3_blob_reopen(sqlite3_blob *pBlob, sqlite3_int64 iRow) {
  int rc;
  Incrblob *p = (Incrblob *)pBlob;
  sqlite3 *db;

  if (p == 0)
    return sqlite3MisuseError(106499);
  db = p->db;
  sqlite3_mutex_enter(db->mutex);

  if (p->pStmt == 0) {
    rc = SQLITE_ABORT;
  } else {
    char *zErr;
    ((Vdbe *)p->pStmt)->rc = SQLITE_OK;
    rc = blobSeekToRow(p, iRow, &zErr);
    if (rc != SQLITE_OK) {
      sqlite3ErrorWithMsg(db, rc, (zErr ? "%s" : (char *)0), zErr);
      sqlite3DbFree(db, zErr);
    }
  }

  rc = sqlite3ApiExit(db, rc);

  sqlite3_mutex_leave(db->mutex);
  return rc;
}
