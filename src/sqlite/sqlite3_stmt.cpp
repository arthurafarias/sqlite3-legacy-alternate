#include "sqlite/Vdbe.h"
#define _GNU_SOURCE 1
#include "sqlite/sqlite3_stmt.h"
#include "sqlite/Column.h"
#include "sqlite/Lookaside.h"
#include "sqlite/Mem.h"
#include "sqlite/VList.h"
#include "sqlite/Vdbe.h"
#include "sqlite/bft.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_destructor_type.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/sqlite3_uint64.h"
#include "sqlite/sqlite3_value.h"
#include "sqlite/sqlite_int64.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/ynVar.h"
#include "sqlite/SqliteFundamentalDatatype.h"
#include "sqlite/SqliteLimitCategory.h"
#include "sqlite/SqliteResultCode.h"
#include "sqlite/SqliteStmtStatusParameter.h"
#include "sqlite/SqliteTextEncoding.h"
static const char *const azExplainColNames8[] = {"addr", "opcode",  "p1", "p2",     "p3",      "p4",
                                                 "p5",   "comment", "id", "parent", "notused", "detail"};

static const u16 azExplainColNames16data[] = {'a', 'd', 'd', 'r', 0,   'o', 'p', 'c', 'o', 'd', 'e', 0,   'p', '1', 0,
                                              'p', '2', 0,   'p', '3', 0,   'p', '4', 0,   'p', '5', 0,   'c', 'o', 'm',
                                              'm', 'e', 'n', 't', 0,   'i', 'd', 0,   'p', 'a', 'r', 'e', 'n', 't', 0,
                                              'n', 'o', 't', 'u', 's', 'e', 'd', 0,   'd', 'e', 't', 'a', 'i', 'l', 0};

static const u8 iExplainColNames16[] = {0, 5, 12, 15, 18, 21, 24, 27, 35, 38, 45, 53};

/** NOTES: this is a safety issue over sqlite3 for avoid people trying
to inspect it's code. C offers the possibility of stealing their
code, I think. But it is just a supperficial obfuscation. The real
step on completely obfuscation is to hold a pointer to a data running
into a VM. for that use void* and things are going to become interesting.
Because the statement should be valid for the vm and only valid for it.
No reference to a statement field outside the VM. I think this is the
necessary step to protect the engine instead of pointing to a Vdbe to
an opaque type. Really unsafe! hahahaha C is awesome! */
struct sqlite3_stmt {
  union data {
    Vdbe vdbe;
  } udata;
};

int sqlite3_expired(sqlite3_stmt *pStmt) {
  int iRet = 1;
  if (pStmt) {
    Vdbe *p = (Vdbe *)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    iRet = p->expired;
    sqlite3_mutex_leave(p->db->mutex);
  }
  return iRet;
}

int sqlite3_finalize(sqlite3_stmt *pStmt) {
  int rc;
  if (pStmt == 0) {
    rc = SQLITE_OK;
  } else {
    Vdbe *v = (Vdbe *)pStmt;
    sqlite3 *db = v->db;
    if (vdbeSafety(v))
      return sqlite3MisuseError(93796);
    sqlite3_mutex_enter(db->mutex);
    if (((v)->startTime) > 0) {
      invokeProfileCallback(db, v);
    };

    rc = sqlite3VdbeReset(v);
    sqlite3VdbeDelete(v);
    rc = sqlite3ApiExit(db, rc);
    sqlite3LeaveMutexAndCloseZombie(db);
  }
  return rc;
}

int sqlite3_reset(sqlite3_stmt *pStmt) {
  int rc;
  if (pStmt == 0) {
    rc = SQLITE_OK;
  } else {
    Vdbe *v = (Vdbe *)pStmt;
    sqlite3 *db = v->db;
    sqlite3_mutex_enter(db->mutex);
    if (((v)->startTime) > 0) {
      invokeProfileCallback(db, v);
    };
    rc = sqlite3VdbeReset(v);
    sqlite3VdbeRewind(v);

    rc = sqlite3ApiExit(db, rc);
    sqlite3_mutex_leave(db->mutex);
  }
  return rc;
}

int sqlite3_clear_bindings(sqlite3_stmt *pStmt) {
  int i;
  int rc = SQLITE_OK;
  Vdbe *p = (Vdbe *)pStmt;

  sqlite3_mutex *mutex;

  mutex = p->db->mutex;

  sqlite3_mutex_enter(mutex);
  for (i = 0; i < p->nVar; i++) {
    sqlite3VdbeMemRelease(&p->aVar[i]);
    p->aVar[i].flags = 0x0001;
  }

  if (p->expmask) {
    p->expired = 1;
  }
  sqlite3_mutex_leave(mutex);
  return rc;
}

int sqlite3_step(sqlite3_stmt *pStmt) {
  int rc = SQLITE_OK;
  Vdbe *v = (Vdbe *)pStmt;
  int cnt = 0;
  sqlite3 *db;

  if (vdbeSafetyNotNull(v)) {
    return sqlite3MisuseError(94608);
  }
  db = v->db;
  sqlite3_mutex_enter(db->mutex);
  while ((rc = sqlite3Step(v)) == SQLITE_SCHEMA && cnt++ < 50) {
    int savedPc = v->pc;
    rc = sqlite3Reprepare(v);
    if (rc != SQLITE_OK) {
      const char *zErr = (const char *)sqlite3_value_text(db->pErr);
      sqlite3DbFree(db, v->zErrMsg);
      if (!db->mallocFailed) {
        v->zErrMsg = sqlite3DbStrDup(db, zErr);
        v->rc = rc = sqlite3ApiExit(db, rc);
      } else {
        v->zErrMsg = 0;
        v->rc = rc = 7;
      }
      break;
    }
    sqlite3_reset(pStmt);
    if (savedPc >= 0) {
      v->minWriteFileFormat = 254;
    }
  }
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

int sqlite3_column_count(sqlite3_stmt *pStmt) {
  Vdbe *pVm = (Vdbe *)pStmt;
  if (pVm == 0)
    return 0;
  return pVm->nResColumn;
}

int sqlite3_data_count(sqlite3_stmt *pStmt) {
  Vdbe *pVm = (Vdbe *)pStmt;
  if (pVm == 0 || pVm->pResultRow == 0)
    return 0;
  return pVm->nResColumn;
}

Mem *columnMem(sqlite3_stmt *pStmt, int i) {
  Vdbe *pVm;
  Mem *pOut;

  pVm = (Vdbe *)pStmt;
  if (pVm == 0)
    return (Mem *)columnNullValue();

  sqlite3_mutex_enter(pVm->db->mutex);
  if (pVm->pResultRow != 0 && i < pVm->nResColumn && i >= 0) {
    pOut = &pVm->pResultRow[i];
  } else {
    sqlite3Error(pVm->db, SQLITE_RANGE);
    pOut = (Mem *)columnNullValue();
  }
  return pOut;
}

void columnMallocFailure(sqlite3_stmt *pStmt) {
  Vdbe *p = (Vdbe *)pStmt;
  if (p) {
    p->rc = sqlite3ApiExit(p->db, p->rc);
    sqlite3_mutex_leave(p->db->mutex);
  }
}

const void *sqlite3_column_blob(sqlite3_stmt *pStmt, int i) {
  const void *val;
  val = sqlite3_value_blob(columnMem(pStmt, i));

  columnMallocFailure(pStmt);
  return val;
}

int sqlite3_column_bytes(sqlite3_stmt *pStmt, int i) {
  int val = sqlite3_value_bytes(columnMem(pStmt, i));
  columnMallocFailure(pStmt);
  return val;
}

int sqlite3_column_bytes16(sqlite3_stmt *pStmt, int i) {
  int val = sqlite3_value_bytes16(columnMem(pStmt, i));
  columnMallocFailure(pStmt);
  return val;
}

double sqlite3_column_double(sqlite3_stmt *pStmt, int i) {
  double val = sqlite3_value_double(columnMem(pStmt, i));
  columnMallocFailure(pStmt);
  return val;
}

int sqlite3_column_int(sqlite3_stmt *pStmt, int i) {
  int val = sqlite3_value_int(columnMem(pStmt, i));
  columnMallocFailure(pStmt);
  return val;
}

sqlite_int64 sqlite3_column_int64(sqlite3_stmt *pStmt, int i) {
  sqlite_int64 val = sqlite3_value_int64(columnMem(pStmt, i));
  columnMallocFailure(pStmt);
  return val;
}

const unsigned char *sqlite3_column_text(sqlite3_stmt *pStmt, int i) {
  const unsigned char *val = sqlite3_value_text(columnMem(pStmt, i));
  columnMallocFailure(pStmt);
  return val;
}

sqlite3_value *sqlite3_column_value(sqlite3_stmt *pStmt, int i) {
  Mem *pOut = columnMem(pStmt, i);
  if (pOut->flags & 0x2000) {
    pOut->flags &= ~0x2000;
    pOut->flags |= 0x4000;
  }
  columnMallocFailure(pStmt);
  return (sqlite3_value *)pOut;
}

const void *sqlite3_column_text16(sqlite3_stmt *pStmt, int i) {
  const void *val = sqlite3_value_text16(columnMem(pStmt, i));
  columnMallocFailure(pStmt);
  return val;
}

int sqlite3_column_type(sqlite3_stmt *pStmt, int i) {
  int iType = sqlite3_value_type(columnMem(pStmt, i));
  columnMallocFailure(pStmt);
  return iType;
}

const void *columnName(sqlite3_stmt *pStmt, int N, int useUtf16, int useType) {
  const void *ret;
  Vdbe *p;
  int n;
  sqlite3 *db;

  if (N < 0)
    return 0;
  ret = 0;
  p = (Vdbe *)pStmt;
  db = p->db;

  sqlite3_mutex_enter(db->mutex);

  if (p->explain) {
    if (useType > 0)
      goto columnName_end;
    n = p->explain == 1 ? 8 : 4;
    if (N >= n)
      goto columnName_end;
    if (useUtf16) {
      int i = iExplainColNames16[N + 8 * p->explain - 8];
      ret = (void *)&azExplainColNames16data[i];
    } else {
      ret = (void *)azExplainColNames8[N + 8 * p->explain - 8];
    }
    goto columnName_end;
  }
  n = p->nResColumn;
  if (N < n) {
    u8 prior_mallocFailed = db->mallocFailed;
    N += useType * n;

    if (useUtf16) {
      ret = sqlite3_value_text16((sqlite3_value *)&p->aColName[N]);
    } else {
      ret = sqlite3_value_text((sqlite3_value *)&p->aColName[N]);
    }

    if (db->mallocFailed > prior_mallocFailed) {
      sqlite3OomClear(db);
      ret = 0;
    }
  }
columnName_end:
  sqlite3_mutex_leave(db->mutex);
  return ret;
}

const char *sqlite3_column_name(sqlite3_stmt *pStmt, int N) {
  return columnName(pStmt, N, 0, 0);
}

const void *sqlite3_column_name16(sqlite3_stmt *pStmt, int N) {
  return columnName(pStmt, N, 1, 0);
}

const char *sqlite3_column_decltype(sqlite3_stmt *pStmt, int N) {
  return columnName(pStmt, N, 0, 1);
}

const void *sqlite3_column_decltype16(sqlite3_stmt *pStmt, int N) {
  return columnName(pStmt, N, 1, 1);
}

int bindText(sqlite3_stmt *pStmt, int i, const void *zData, i64 nData, void (*xDel)(void *), u8 encoding) {
  Vdbe *p = (Vdbe *)pStmt;
  Mem *pVar;
  int rc;

  rc = vdbeUnbind(p, (u32)(i - 1));
  if (rc == SQLITE_OK) {
    if (zData != 0) {
      pVar = &p->aVar[i - 1];
      if (encoding == SQLITE_UTF8) {
        rc = sqlite3VdbeMemSetText(pVar, zData, nData, xDel);
      } else if (encoding == SQLITE_UTF8_ZT) {
        rc = sqlite3VdbeMemSetText(pVar, zData, nData, xDel);
        pVar->flags |= 0x0200;
      } else {
        rc = sqlite3VdbeMemSetStr(pVar, zData, nData, encoding, xDel);
        if (encoding == 0)
          pVar->enc = ((p->db)->enc);
      }
      if (rc == SQLITE_OK && encoding != 0) {
        rc = sqlite3VdbeChangeEncoding(pVar, ((p->db)->enc));
      }
      if (rc) {
        sqlite3Error(p->db, rc);
        rc = sqlite3ApiExit(p->db, rc);
      }
    }
    sqlite3_mutex_leave(p->db->mutex);
  } else if (xDel != ((sqlite3_destructor_type)0) && xDel != ((sqlite3_destructor_type)-1)) {
    xDel((void *)zData);
  }
  return rc;
}

int sqlite3_bind_blob(sqlite3_stmt *pStmt, int i, const void *zData, int nData, void (*xDel)(void *)) {
  return bindText(pStmt, i, zData, nData, xDel, 0);
}

int sqlite3_bind_blob64(sqlite3_stmt *pStmt, int i, const void *zData, sqlite3_uint64 nData, void (*xDel)(void *)) {
  return bindText(pStmt, i, zData, nData, xDel, 0);
}

int sqlite3_bind_double(sqlite3_stmt *pStmt, int i, double rValue) {
  int rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i - 1));
  if (rc == SQLITE_OK) {
    sqlite3VdbeMemSetDouble(&p->aVar[i - 1], rValue);
    sqlite3_mutex_leave(p->db->mutex);
  }
  return rc;
}

int sqlite3_bind_int(sqlite3_stmt *p, int i, int iValue) {
  return sqlite3_bind_int64(p, i, (i64)iValue);
}

int sqlite3_bind_int64(sqlite3_stmt *pStmt, int i, sqlite_int64 iValue) {
  int rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i - 1));
  if (rc == SQLITE_OK) {
    sqlite3VdbeMemSetInt64(&p->aVar[i - 1], iValue);
    sqlite3_mutex_leave(p->db->mutex);
  }
  return rc;
}

int sqlite3_bind_null(sqlite3_stmt *pStmt, int i) {
  int rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i - 1));
  if (rc == SQLITE_OK) {
    sqlite3_mutex_leave(p->db->mutex);
  }
  return rc;
}

int sqlite3_bind_pointer(sqlite3_stmt *pStmt, int i, void *pPtr, const char *zPTtype, void (*xDestructor)(void *)) {
  int rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i - 1));
  if (rc == SQLITE_OK) {
    sqlite3VdbeMemSetPointer(&p->aVar[i - 1], pPtr, zPTtype, xDestructor);
    sqlite3_mutex_leave(p->db->mutex);
  } else if (xDestructor) {
    xDestructor(pPtr);
  }
  return rc;
}

int sqlite3_bind_text(sqlite3_stmt *pStmt, int i, const char *zData, int nData, void (*xDel)(void *)) {
  return bindText(pStmt, i, zData, nData, xDel, SQLITE_UTF8);
}

int sqlite3_bind_text64(sqlite3_stmt *pStmt, int i, const char *zData, sqlite3_uint64 nData, void (*xDel)(void *),
                        unsigned char enc) {
  if (enc != SQLITE_UTF8 && enc != SQLITE_UTF8_ZT) {
    if (enc == SQLITE_UTF16)
      enc = 2;
    nData &= ~(u64)1;
  }
  return bindText(pStmt, i, zData, nData, xDel, enc);
}

int sqlite3_bind_text16(sqlite3_stmt *pStmt, int i, const void *zData, int n, void (*xDel)(void *)) {
  return bindText(pStmt, i, zData, n & ~(u64)1, xDel, 2);
}

int sqlite3_bind_value(sqlite3_stmt *pStmt, int i, const sqlite3_value *pValue) {
  int rc;
  switch (sqlite3_value_type((sqlite3_value *)pValue)) {
    case SQLITE_INTEGER: {
      rc = sqlite3_bind_int64(pStmt, i, pValue->u.i);
      break;
    }
    case SQLITE_FLOAT: {
      rc = sqlite3_bind_double(pStmt, i, (pValue->flags & 0x0008) ? pValue->u.r : (double)pValue->u.i);
      break;
    }
    case SQLITE_BLOB: {
      if (pValue->flags & 0x0400) {
        rc = sqlite3_bind_zeroblob(pStmt, i, pValue->u.nZero);
      } else {
        rc = sqlite3_bind_blob(pStmt, i, pValue->z, pValue->n, ((sqlite3_destructor_type)-1));
      }
      break;
    }
    case 3: {
      rc = bindText(pStmt, i, pValue->z, pValue->n, ((sqlite3_destructor_type)-1), pValue->enc);
      break;
    }
    default: {
      rc = sqlite3_bind_null(pStmt, i);
      break;
    }
  }
  return rc;
}

int sqlite3_bind_zeroblob(sqlite3_stmt *pStmt, int i, int n) {
  int rc;
  Vdbe *p = (Vdbe *)pStmt;
  rc = vdbeUnbind(p, (u32)(i - 1));
  if (rc == SQLITE_OK) {
    sqlite3VdbeMemSetZeroBlob(&p->aVar[i - 1], n);

    sqlite3_mutex_leave(p->db->mutex);
  }
  return rc;
}

int sqlite3_bind_zeroblob64(sqlite3_stmt *pStmt, int i, sqlite3_uint64 n) {
  int rc;
  Vdbe *p = (Vdbe *)pStmt;

  sqlite3_mutex_enter(p->db->mutex);
  if (n > (u64)p->db->aLimit[SQLITE_LIMIT_LENGTH]) {
    rc = SQLITE_TOOBIG;
  } else {
    rc = sqlite3_bind_zeroblob(pStmt, i, n);
  }
  rc = sqlite3ApiExit(p->db, rc);
  sqlite3_mutex_leave(p->db->mutex);
  return rc;
}

int sqlite3_bind_parameter_count(sqlite3_stmt *pStmt) {
  Vdbe *p = (Vdbe *)pStmt;
  return p ? p->nVar : 0;
}

const char *sqlite3_bind_parameter_name(sqlite3_stmt *pStmt, int i) {
  Vdbe *p = (Vdbe *)pStmt;
  if (p == 0)
    return 0;
  return sqlite3VListNumToName(p->pVList, i);
}

int sqlite3_bind_parameter_index(sqlite3_stmt *pStmt, const char *zName) {
  return sqlite3VdbeParameterIndex((Vdbe *)pStmt, zName, sqlite3Strlen30(zName));
}

int sqlite3TransferBindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *pToStmt) {
  Vdbe *pFrom = (Vdbe *)pFromStmt;
  Vdbe *pTo = (Vdbe *)pToStmt;
  int i;

  sqlite3_mutex_enter(pTo->db->mutex);
  for (i = 0; i < pFrom->nVar; i++) {
    sqlite3VdbeMemMove(&pTo->aVar[i], &pFrom->aVar[i]);
  }
  sqlite3_mutex_leave(pTo->db->mutex);
  return SQLITE_OK;
}

int sqlite3_transfer_bindings(sqlite3_stmt *pFromStmt, sqlite3_stmt *pToStmt) {
  Vdbe *pFrom = (Vdbe *)pFromStmt;
  Vdbe *pTo = (Vdbe *)pToStmt;
  if (pFrom->nVar != pTo->nVar) {
    return SQLITE_ERROR;
  }

  if (pTo->expmask) {
    pTo->expired = 1;
  }

  if (pFrom->expmask) {
    pFrom->expired = 1;
  }
  return sqlite3TransferBindings(pFromStmt, pToStmt);
}

sqlite3 *sqlite3_db_handle(sqlite3_stmt *pStmt) {
  return pStmt ? ((Vdbe *)pStmt)->db : 0;
}

int sqlite3_stmt_readonly(sqlite3_stmt *pStmt) {
  return pStmt ? ((Vdbe *)pStmt)->readOnly : 1;
}

int sqlite3_stmt_isexplain(sqlite3_stmt *pStmt) {
  return pStmt ? ((Vdbe *)pStmt)->explain : 0;
}

int sqlite3_stmt_explain(sqlite3_stmt *pStmt, int eMode) {
  Vdbe *v = (Vdbe *)pStmt;
  int rc;

  sqlite3_mutex_enter(v->db->mutex);
  if (((int)v->explain) == eMode) {
    rc = SQLITE_OK;
  } else if (eMode < 0 || eMode > 2) {
    rc = SQLITE_ERROR;
  } else if ((v->prepFlags & 0x80) == 0) {
    rc = SQLITE_ERROR;
  } else if (v->eVdbeState != 1) {
    rc = SQLITE_BUSY;
  } else if (v->nMem >= 10 && (eMode != 2 || v->haveEqpOps)) {
    v->explain = eMode;
    rc = SQLITE_OK;
  } else {
    v->explain = eMode;
    rc = sqlite3Reprepare(v);
    v->haveEqpOps = eMode == 2;
  }
  if (v->explain) {
    v->nResColumn = 12 - 4 * v->explain;
  } else {
    v->nResColumn = v->nResAlloc;
  }
  sqlite3_mutex_leave(v->db->mutex);
  return rc;
}

int sqlite3_stmt_busy(sqlite3_stmt *pStmt) {
  Vdbe *v = (Vdbe *)pStmt;
  return v != 0 && v->eVdbeState == 2;
}

int sqlite3_stmt_status(sqlite3_stmt *pStmt, int op, int resetFlag) {
  Vdbe *pVdbe = (Vdbe *)pStmt;
  u32 v;

  if (op == SQLITE_STMTSTATUS_MEMUSED) {
    sqlite3 *db = pVdbe->db;
    sqlite3_mutex_enter(db->mutex);
    v = 0;
    db->pnBytesFreed = (int *)&v;

    db->lookaside.pEnd = db->lookaside.pStart;
    sqlite3VdbeDelete(pVdbe);
    db->pnBytesFreed = 0;
    db->lookaside.pEnd = db->lookaside.pTrueEnd;
    sqlite3_mutex_leave(db->mutex);
  } else {
    v = pVdbe->aCounter[op];
    if (resetFlag)
      pVdbe->aCounter[op] = 0;
  }
  return (int)v;
}

const char *sqlite3_sql(sqlite3_stmt *pStmt) {
  Vdbe *p = (Vdbe *)pStmt;
  return p ? p->zSql : 0;
}

char *sqlite3_expanded_sql(sqlite3_stmt *pStmt) {
  char *z = 0;
  const char *zSql = sqlite3_sql(pStmt);
  if (zSql) {
    Vdbe *p = (Vdbe *)pStmt;
    sqlite3_mutex_enter(p->db->mutex);
    z = sqlite3VdbeExpandSql(p, zSql);
    sqlite3_mutex_leave(p->db->mutex);
  }
  return z;
}