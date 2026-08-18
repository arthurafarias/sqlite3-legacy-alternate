#define _GNU_SOURCE 1
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sqlite/Parse.h"
#include "sqlite/AggInfo.h"
#include "sqlite/AuthContext.h"
#include "sqlite/AutoincInfo.h"
#include "sqlite/Btree.h"
#include "sqlite/BusyHandler.h"
#include "sqlite/CheckOnCtx.h"
#include "sqlite/CollSeq.h"
#include "sqlite/Column.h"
#include "sqlite/Cte.h"
#include "sqlite/CteUse.h"
#include "sqlite/DateTime.h"
#include "sqlite/Db.h"
#include "sqlite/DbFixer.h"
#include "sqlite/DistinctCtx.h"
#include "sqlite/Expr.h"
#include "sqlite/ExprList.h"
#include "sqlite/FKey.h"
#include "sqlite/FuncDef.h"
#include "sqlite/FuncDefHash.h"
#include "sqlite/Hash.h"
#include "sqlite/HashElem.h"
#include "sqlite/IdList.h"
#include "sqlite/Index.h"
#include "sqlite/IndexIterator.h"
#include "sqlite/IndexListTerm.h"
#include "sqlite/IndexedExpr.h"
#include "sqlite/KeyInfo.h"
#include "sqlite/LogEst.h"
#include "sqlite/Lookaside.h"
#include "sqlite/Mem.h"
#include "sqlite/Module.h"
#include "sqlite/NameContext.h"
#include "sqlite/OnOrUsing.h"
#include "sqlite/Op.h"
#include "sqlite/Pager.h"
#include "sqlite/ParseCleanup.h"
#include "sqlite/Pgno.h"
#include "sqlite/PragmaName.h"
#include "sqlite/RefSrcList.h"
#include "sqlite/RenameCtx.h"
#include "sqlite/RenameToken.h"
#include "sqlite/Returning.h"
#include "sqlite/RowLoadInfo.h"
#include "sqlite/RowSet.h"
#include "sqlite/Schema.h"
#include "sqlite/Select.h"
#include "sqlite/SelectDest.h"
#include "sqlite/SortCtx.h"
#include "sqlite/Sqlite3Config.h"
#include "sqlite/SrcItem.h"
#include "sqlite/SrcList.h"
#include "sqlite/StrAccum.h"
#include "sqlite/SubProgram.h"
#include "sqlite/Subquery.h"
#include "sqlite/SubrtnSig.h"
#include "sqlite/SubstContext.h"
#include "sqlite/Table.h"
#include "sqlite/TableLock.h"
#include "sqlite/Token.h"
#include "sqlite/Trigger.h"
#include "sqlite/TriggerPrg.h"
#include "sqlite/TriggerStep.h"
#include "sqlite/Upsert.h"
#include "sqlite/VList.h"
#include "sqlite/VTable.h"
#include "sqlite/Vdbe.h"
#include "sqlite/VdbeOp.h"
#include "sqlite/VdbeOpList.h"
#include "sqlite/Wal.h"
#include "sqlite/Walker.h"
#include "sqlite/WhereClause.h"
#include "sqlite/WhereConst.h"
#include "sqlite/WhereInfo.h"
#include "sqlite/WhereLevel.h"
#include "sqlite/WhereLoop.h"
#include "sqlite/WhereLoopBuilder.h"
#include "sqlite/WhereMaskSet.h"
#include "sqlite/WhereOrInfo.h"
#include "sqlite/WhereRightJoin.h"
#include "sqlite/WhereTerm.h"
#include "sqlite/Window.h"
#include "sqlite/WindowCodeArg.h"
#include "sqlite/WindowCsrAndReg.h"
#include "sqlite/WindowFunctionNames.h"
#include "sqlite/WindowRewrite.h"
#include "sqlite/With.h"
#include "sqlite/bft.h"
#include "sqlite/i16.h"
#include "sqlite/i64.h"
#include "sqlite/i8.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_context.h"
#include "sqlite/sqlite3_destructor_type.h"
#include "sqlite/sqlite3_filename.h"
#include "sqlite/sqlite3_index_info.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_module.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/sqlite3_soft_heap.h"
#include "sqlite/sqlite3_stmt.h"
#include "sqlite/sqlite3_str.h"
#include "sqlite/sqlite3_uint64.h"
#include "sqlite/sqlite3_value.h"
#include "sqlite/sqlite3_vfs.h"
#include "sqlite/sqlite3_vtab.h"
#include "sqlite/sqlite3_xauth.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/yDbMask.h"
#include "sqlite/ynVar.h"
#include "sqlite/yyParser.h"
#include "sqlite/SqliteAccessFlags.h"
#include "sqlite/SqliteAuthorizerActionCode.h"
#include "sqlite/SqliteAuthorizerReturnCode.h"
#include "sqlite/SqliteCheckpointMode.h"
#include "sqlite/SqliteFileControlOpcode.h"
#include "sqlite/SqliteFunctionFlags.h"
#include "sqlite/SqliteFundamentalDatatype.h"
#include "sqlite/SqliteIndexConstraintOp.h"
#include "sqlite/SqliteLimitCategory.h"
#include "sqlite/SqliteMutexType.h"
#include "sqlite/SqliteOpenFlags.h"
#include "sqlite/SqlitePrepareFlags.h"
#include "sqlite/SqliteResultCode.h"
#include "sqlite/SqliteStmtStatusParameter.h"
#include "sqlite/SqliteTextEncoding.h"
#include "sqlite/SqliteTxnState.h"
/* Private helpers, formerly declared in _Uncategorized.h. */
static const char *actionName(u8 action);
static int analyzeFilterKeyword(const unsigned char *z, int lastToken);
static int analyzeOverKeyword(const unsigned char *z, int lastToken);
static int analyzeWindowKeyword(const unsigned char *z);
static int collationMatch(const char *zColl, Index *pIndex);
static int getAutoVacuum(const char *z);
static int getLockingMode(const char *z);
static int getTempStore(const char *z);
static int getToken(const unsigned char **pz);
static int hasColumn(const i16 *aiCol, int nCol, int x);
static __attribute__((noinline)) int isValidSchemaTableName(const char *zTab, Table *pTab, const char *zDb);

static __attribute__((noinline)) int isValidSchemaTableName(const char *zTab, Table *pTab, const char *zDb) {
  const char *zLegacy;

  if (sqlite3_strnicmp(zTab, "sqlite_", 7) != 0)
    return 0;
  zLegacy = pTab->zName;
  if (strcmp(zLegacy + 7, &"sqlite_temp_master"[7]) == 0) {
    if (sqlite3StrICmp(zTab + 7, &"sqlite_temp_schema"[7]) == 0) {
      return 1;
    }
    if (zDb == 0)
      return 0;
    if (sqlite3StrICmp(zTab + 7, &"sqlite_master"[7]) == 0)
      return 1;
    if (sqlite3StrICmp(zTab + 7, &"sqlite_schema"[7]) == 0)
      return 1;
  } else {
    if (sqlite3StrICmp(zTab + 7, &"sqlite_schema"[7]) == 0)
      return 1;
  }
  return 0;
}

const struct ExprList_item zeroItem = {0};

static int hasColumn(const i16 *aiCol, int nCol, int x) {
  while (nCol-- > 0) {
    if (x == *(aiCol++)) {
      return 1;
    }
  }
  return 0;
}

static int collationMatch(const char *zColl, Index *pIndex) {
  int i;

  for (i = 0; i < pIndex->nColumn; i++) {
    const char *z = pIndex->azColl[i];

    if (0 == sqlite3StrICmp(z, zColl)) {
      return 1;
    }
  }
  return 0;
}

static const PragmaName aPragmaName[] = {

    {"analysis_limit", 1, 0x10, 0, 0, 0},
    {"application_id", 2, 0x04 | 0x10, 0, 0, 8},
    {"auto_vacuum", 3, 0x01 | 0x10 | 0x80 | 0x04, 0, 0, 0},
    {"automatic_index", 4, 0x10 | 0x04, 0, 0, 0x00008000},
    {"busy_timeout", 5, 0x10, 56, 1, 0},
    {"cache_size", 6, 0x01 | 0x10 | 0x80 | 0x04, 0, 0, 0},
    {"cache_spill", 7, 0x10 | 0x80 | 0x04, 0, 0, 0},
    {"case_sensitive_like", 8, 0x02, 0, 0, 0},
    {"cell_size_check", 4, 0x10 | 0x04, 0, 0, 0x00200000},
    {"checkpoint_fullfsync", 4, 0x10 | 0x04, 0, 0, 0x00000010},
    {"collation_list", 9, 0x10, 33, 2, 0},
    {"compile_options", 10, 0x10, 0, 0, 0},
    {"count_changes", 4, 0x10 | 0x04, 0, 0, ((u64)(0x00001) << 32)},
    {"data_version", 2, 0x08 | 0x10, 0, 0, 15},
    {"database_list", 12, 0x10, 50, 3, 0},
    {"default_cache_size", 13, 0x01 | 0x10 | 0x80 | 0x04, 55, 1, 0},
    {"defer_foreign_keys", 4, 0x10 | 0x04, 0, 0, 0x00080000},
    {"empty_result_callbacks", 4, 0x10 | 0x04, 0, 0, 0x00000100},
    {"encoding", 14, 0x10 | 0x04, 0, 0, 0},
    {"foreign_key_check", 15, 0x01 | 0x10 | 0x20 | 0x40, 43, 4, 0},
    {"foreign_key_list", 16, 0x01 | 0x20 | 0x40, 0, 8, 0},
    {"foreign_keys", 4, 0x10 | 0x04, 0, 0, 0x00004000},
    {"freelist_count", 2, 0x08 | 0x10, 0, 0, 0},
    {"full_column_names", 4, 0x10 | 0x04, 0, 0, 0x00000004},
    {"fullfsync", 4, 0x10 | 0x04, 0, 0, 0x00000008},
    {"function_list", 17, 0x10, 15, 6, 0},
    {"hard_heap_limit", 18, 0x10, 0, 0, 0},
    {"ignore_check_constraints", 4, 0x10 | 0x04, 0, 0, 0x00000200},
    {"incremental_vacuum", 19, 0x01 | 0x02, 0, 0, 0},
    {"index_info", 20, 0x01 | 0x20 | 0x40, 27, 3, 0},
    {"index_list", 21, 0x01 | 0x20 | 0x40, 33, 5, 0},
    {"index_xinfo", 20, 0x01 | 0x20 | 0x40, 27, 6, 1},
    {"integrity_check", 22, 0x01 | 0x10 | 0x20 | 0x40, 0, 0, 0},
    {"journal_mode", 23, 0x01 | 0x10 | 0x80, 0, 0, 0},
    {"journal_size_limit", 24, 0x10 | 0x80, 0, 0, 0},
    {"legacy_alter_table", 4, 0x10 | 0x04, 0, 0, 0x04000000},
    {"locking_mode", 26, 0x10 | 0x80, 0, 0, 0},
    {"max_page_count", 27, 0x01 | 0x10 | 0x80, 0, 0, 0},
    {"mmap_size", 28, 0, 0, 0, 0},
    {"module_list", 29, 0x10, 9, 1, 0},
    {"optimize", 30, 0x20 | 0x01, 0, 0, 0},
    {"page_count", 27, 0x01 | 0x10 | 0x80, 0, 0, 0},
    {"page_size", 31, 0x10 | 0x80 | 0x04, 0, 0, 0},
    {"pragma_list", 32, 0x10, 9, 1, 0},
    {"query_only", 4, 0x10 | 0x04, 0, 0, 0x00100000},
    {"quick_check", 22, 0x01 | 0x10 | 0x20 | 0x40, 0, 0, 0},
    {"read_uncommitted", 4, 0x10 | 0x04, 0, 0, ((u64)(0x00004) << 32)},
    {"recursive_triggers", 4, 0x10 | 0x04, 0, 0, 0x00002000},
    {"reverse_unordered_selects", 4, 0x10 | 0x04, 0, 0, 0x00001000},
    {"schema_version", 2, 0x04 | 0x10, 0, 0, 1},
    {"secure_delete", 33, 0x10, 0, 0, 0},
    {"short_column_names", 4, 0x10 | 0x04, 0, 0, 0x00000040},
    {"shrink_memory", 34, 0x02, 0, 0, 0},
    {"soft_heap_limit", 35, 0x10, 0, 0, 0},
    {"synchronous", 36, 0x01 | 0x10 | 0x80 | 0x04, 0, 0, 0},
    {"table_info", 37, 0x01 | 0x20 | 0x40, 8, 6, 0},
    {"table_list", 38, 0x01 | 0x20, 21, 6, 0},
    {"table_xinfo", 37, 0x01 | 0x20 | 0x40, 8, 7, 1},
    {"temp_store", 39, 0x10 | 0x04, 0, 0, 0},
    {"temp_store_directory", 40, 0x04, 0, 0, 0},
    {"threads", 41, 0x10, 0, 0, 0},
    {"trusted_schema", 4, 0x10 | 0x04, 0, 0, 0x00000080},
    {"user_version", 2, 0x04 | 0x10, 0, 0, 6},
    {"wal_autocheckpoint", 42, 0, 0, 0, 0},
    {"wal_checkpoint", 43, 0x01, 47, 3, 0},
    {"writable_schema", 4, 0x10 | 0x04, 0, 0, 0x00000001 | 0x08000000},
};

u8 getSafetyLevel(const char *z, int omitFull, u8 dflt) {
  static const char zText[] = "onoffalseyestruextrafull";
  static const u8 iOffset[] = {0, 1, 2, 4, 9, 12, 15, 20};
  static const u8 iLength[] = {2, 2, 3, 5, 3, 4, 5, 4};
  static const u8 iValue[] = {1, 0, 0, 0, 1, 1, 3, 2};

  int i, n;
  if ((sqlite3CtypeMap[(unsigned char)(*z)] & 0x04)) {
    return (u8)sqlite3Atoi(z);
  }
  n = sqlite3Strlen30(z);
  for (i = 0; i < ((int)(sizeof(iLength) / sizeof(iLength[0]))); i++) {
    if (iLength[i] == n && sqlite3_strnicmp(&zText[iOffset[i]], z, n) == 0 && (!omitFull || iValue[i] <= 1)) {
      return iValue[i];
    }
  }
  return dflt;
}

static int getLockingMode(const char *z) {
  if (z) {
    if (0 == sqlite3StrICmp(z, "exclusive"))
      return 1;
    if (0 == sqlite3StrICmp(z, "normal"))
      return 0;
  }
  return -1;
}

static int getAutoVacuum(const char *z) {
  int i;
  if (0 == sqlite3StrICmp(z, "none"))
    return 0;
  if (0 == sqlite3StrICmp(z, "full"))
    return 1;
  if (0 == sqlite3StrICmp(z, "incremental"))
    return 2;
  i = sqlite3Atoi(z);
  return (u8)((i >= 0 && i <= 2) ? i : 0);
}

static int getTempStore(const char *z) {
  if (z[0] >= '0' && z[0] <= '2') {
    return z[0] - '0';
  } else if (sqlite3StrICmp(z, "file") == 0) {
    return 1;
  } else if (sqlite3StrICmp(z, "memory") == 0) {
    return 2;
  } else {
    return 0;
  }
}

static const char *actionName(u8 action) {
  const char *zName;
  switch (action) {
    case 8:
      zName = "SET NULL";
      break;
    case 9:
      zName = "SET DEFAULT";
      break;
    case 10:
      zName = "CASCADE";
      break;
    case 7:
      zName = "RESTRICT";
      break;
    default:
      zName = "NO ACTION";

      break;
  }
  return zName;
}

const PragmaName *pragmaLocate(const char *zName) {
  int upr, lwr, mid = 0, rc;
  lwr = 0;
  upr = ((int)(sizeof(aPragmaName) / sizeof(aPragmaName[0]))) - 1;
  while (lwr <= upr) {
    mid = (lwr + upr) / 2;
    rc = sqlite3_stricmp(zName, aPragmaName[mid].zName);
    if (rc == 0)
      break;
    if (rc < 0) {
      upr = mid - 1;
    } else {
      lwr = mid + 1;
    }
  }
  return lwr > upr ? 0 : &aPragmaName[mid];
}

static int getToken(const unsigned char **pz) {
  const unsigned char *z = *pz;
  int t;
  do {
    z += sqlite3GetToken(z, &t);
  } while (t == 184 || t == 185);
  if (t == 60 || t == 118 || t == 119 || t == 165 || t == 166 || sqlite3ParserFallback(t) == 60) {
    t = 60;
  }
  *pz = z;
  return t;
}

static int analyzeWindowKeyword(const unsigned char *z) {
  int t;
  t = getToken(&z);
  if (t != 60)
    return 60;
  t = getToken(&z);
  if (t != 24)
    return 60;
  return 165;
}

static int analyzeOverKeyword(const unsigned char *z, int lastToken) {
  if (lastToken == 23) {
    int t = getToken(&z);
    if (t == 22 || t == 60)
      return 166;
  }
  return 60;
}

static int analyzeFilterKeyword(const unsigned char *z, int lastToken) {
  if (lastToken == 23 && getToken(&z) == 22) {
    return 167;
  }
  return 60;
}

static int parseTimezone(const char *zDate, DateTime *p) {
  int sgn = 0;
  int nHr, nMn;
  int c;
  while ((sqlite3CtypeMap[(unsigned char)(*zDate)] & 0x01)) {
    zDate++;
  }
  p->tz = 0;
  c = *zDate;
  if (c == '-') {
    sgn = -1;
  } else if (c == '+') {
    sgn = +1;
  } else if (c == 'Z' || c == 'z') {
    zDate++;
    p->isLocal = 0;
    p->isUtc = 1;
    goto zulu_time;
  } else {
    return c != 0;
  }
  zDate++;
  if (getDigits(zDate, "20b:20e", &nHr, &nMn) != 2) {
    return 1;
  }
  zDate += 5;
  p->tz = sgn * (nMn + nHr * 60);
  if (p->tz == 0) {
    p->isLocal = 0;
    p->isUtc = 1;
  }
zulu_time:
  while ((sqlite3CtypeMap[(unsigned char)(*zDate)] & 0x01)) {
    zDate++;
  }
  return *zDate != 0;
}

int parseHhMmSs(const char *zDate, DateTime *p) {
  int h, m, s;
  double ms = 0.0;
  if (getDigits(zDate, "20c:20e", &h, &m) != 2) {
    return 1;
  }
  zDate += 5;
  if (*zDate == ':') {
    zDate++;
    if (getDigits(zDate, "20e", &s) != 1) {
      return 1;
    }
    zDate += 2;
    if (*zDate == '.' && (sqlite3CtypeMap[(unsigned char)(zDate[1])] & 0x04)) {
      double rScale = 1.0;
      zDate++;
      while ((sqlite3CtypeMap[(unsigned char)(*zDate)] & 0x04)) {
        ms = ms * 10.0 + *zDate - '0';
        rScale *= 10.0;
        zDate++;
      }
      ms /= rScale;

      if (ms > 0.999)
        ms = 0.999;
    }
  } else {
    s = 0;
  }
  p->validJD = 0;
  p->rawS = 0;
  p->validHMS = 1;
  p->h = h;
  p->m = m;
  p->s = s + ms;
  if (parseTimezone(zDate, p))
    return 1;
  return 0;
}

int parseYyyyMmDd(const char *zDate, DateTime *p) {
  int Y, M, D, neg;

  if (zDate[0] == '-') {
    zDate++;
    neg = 1;
  } else {
    neg = 0;
  }
  if (getDigits(zDate, "40f-21a-21d", &Y, &M, &D) != 3) {
    return 1;
  }
  zDate += 10;
  while ((sqlite3CtypeMap[(unsigned char)(*zDate)] & 0x01) || 'T' == *(u8 *)zDate) {
    zDate++;
  }
  if (parseHhMmSs(zDate, p) == 0) {
  } else if (*zDate == 0) {
    p->validHMS = 0;
  } else {
    return 1;
  }
  p->validJD = 0;
  p->validYMD = 1;
  p->Y = neg ? -Y : Y;
  p->M = M;
  p->D = D;
  computeFloor(p);
  if (p->tz) {
    computeJD(p);
  }
  return 0;
}

void sqlite3ProgressCheck(Parse *p) {
  sqlite3 *db = p->db;
  if (__atomic_load_n((&db->u1.isInterrupted), 0)) {
    p->nErr++;
    p->rc = SQLITE_INTERRUPT;
  }

  if (db->xProgress) {
    if (p->rc == SQLITE_INTERRUPT) {
      p->nProgressSteps = 0;
    } else if ((++p->nProgressSteps) >= db->nProgressOps) {
      if (db->xProgress(db->pProgressArg)) {
        p->nErr++;
        p->rc = SQLITE_INTERRUPT;
      }
      p->nProgressSteps = 0;
    }
  }
}

void sqlite3ErrorMsg(Parse *pParse, const char *zFormat, ...) {
  char *zMsg;
  va_list ap;
  sqlite3 *db = pParse->db;

  db->errByteOffset = -2;

  va_start(ap, zFormat);
  zMsg = sqlite3VMPrintf(db, zFormat, ap);

  va_end(ap);
  if (db->errByteOffset < -1)
    db->errByteOffset = -1;
  if (db->suppressErr) {
    sqlite3DbFree(db, zMsg);
    if (db->mallocFailed) {
      pParse->nErr++;
      pParse->rc = SQLITE_NOMEM;
    }
  } else {
    pParse->nErr++;
    sqlite3DbFree(db, pParse->zErrMsg);
    pParse->zErrMsg = zMsg;
    pParse->rc = SQLITE_ERROR;
    pParse->pWith = 0;
  }
}

void sqlite3DequoteNumber(Parse *pParse, Expr *p) {
  if (p) {
    const char *pIn = p->u.zToken;
    char *pOut = p->u.zToken;
    int bHex = (pIn[0] == '0' && (pIn[1] == 'x' || pIn[1] == 'X'));
    int iValue;

    p->op = 156;
    do {
      if (*pIn != '_') {
        *pOut++ = *pIn;
        if (*pIn == 'e' || *pIn == 'E' || *pIn == '.')
          p->op = 154;
      } else {
        if ((bHex == 0 && (!(sqlite3CtypeMap[(unsigned char)(pIn[-1])] & 0x04) ||
                           !(sqlite3CtypeMap[(unsigned char)(pIn[1])] & 0x04))) ||
            (bHex == 1 && (!(sqlite3CtypeMap[(unsigned char)(pIn[-1])] & 0x08) ||
                           !(sqlite3CtypeMap[(unsigned char)(pIn[1])] & 0x08)))) {
          sqlite3ErrorMsg(pParse, "unrecognized token: \"%s\"", p->u.zToken);
        }
      }
    } while (*pIn++);
    if (bHex)
      p->op = 156;

    if (p->op == 156 && sqlite3GetInt32(p->u.zToken, &iValue)) {
      p->u.iValue = iValue;
      p->flags |= 0x000800;
    }
  }
}

Vdbe *sqlite3VdbeCreate(Parse *pParse) {
  sqlite3 *db = pParse->db;
  Vdbe *p;
  p = (Vdbe*)(sqlite3DbMallocRawNN(db, sizeof(Vdbe)));
  if (p == 0)
    return 0;
  memset(&p->aOp, 0, sizeof(Vdbe) - offsetof(Vdbe, aOp));
  p->db = db;
  if (db->pVdbe) {
    db->pVdbe->ppVPrev = &p->pVNext;
  }
  p->pVNext = db->pVdbe;
  p->ppVPrev = &db->pVdbe;
  db->pVdbe = p;

  p->pParse = pParse;
  pParse->pVdbe = p;

  sqlite3VdbeAddOp2(p, 8, 0, 1);
  return p;
}

int sqlite3VdbeAddFunctionCall(Parse *pParse, int p1, int p2, int p3, int nArg, const FuncDef *pFunc, int eCallCtx) {
  Vdbe *v = pParse->pVdbe;
  int addr;
  sqlite3_context *pCtx;

  pCtx = (sqlite3_context*)(sqlite3DbMallocRawNN(pParse->db, (offsetof(sqlite3_context, argv) + (nArg) * sizeof(sqlite3_value *))));
  if (pCtx == 0) {
    freeEphemeralFunction(pParse->db, (FuncDef *)pFunc);
    return 0;
  }
  pCtx->pOut = 0;
  pCtx->pFunc = (FuncDef *)pFunc;
  pCtx->pVdbe = 0;
  pCtx->isError = 0;
  pCtx->argc = nArg;
  pCtx->iOp = sqlite3VdbeCurrentAddr(v);
  addr = sqlite3VdbeAddOp4(v, eCallCtx ? 67 : 68, p1, p2, p3, (char *)pCtx, (-16));
  sqlite3VdbeChangeP5(v, eCallCtx & 0x00002e);
  sqlite3MayAbort(pParse);
  return addr;
}

int sqlite3VdbeExplainParent(Parse *pParse) {
  VdbeOp *pOp;
  if (pParse->addrExplain == 0)
    return 0;
  pOp = sqlite3VdbeGetOp(pParse->pVdbe, pParse->addrExplain);
  return pOp->p2;
}

int sqlite3VdbeExplain(Parse *pParse, u8 bPush, const char *zFmt, ...) {
  int addr = 0;

  if (pParse->explain == 2 || 0) {
    char *zMsg;
    Vdbe *v;
    va_list ap;
    int iThis;

    va_start(ap, zFmt);
    zMsg = sqlite3VMPrintf(pParse->db, zFmt, ap);

    va_end(ap);
    v = pParse->pVdbe;
    iThis = v->nOp;
    addr = sqlite3VdbeAddOp4(v, 190, iThis, pParse->addrExplain, 0, zMsg, (-7));
    if (bPush) {
      pParse->addrExplain = iThis;
    };
  }
  return addr;
}

void sqlite3VdbeExplainPop(Parse *pParse) {
  pParse->addrExplain = sqlite3VdbeExplainParent(pParse);
}

int sqlite3VdbeMakeLabel(Parse *pParse) {
  return --pParse->nLabel;
}

__attribute__((noinline)) void resizeResolveLabel(Parse *p, Vdbe *v, int j) {
  int nNewSize = 10 - p->nLabel;
  p->aLabel = (int*)(sqlite3DbReallocOrFree(p->db, p->aLabel, nNewSize * sizeof(p->aLabel[0])));
  if (p->aLabel == 0) {
    p->nLabelAlloc = 0;
  } else {
    if (nNewSize >= 100 && (nNewSize / 100) > (p->nLabelAlloc / 100)) {
      sqlite3ProgressCheck(p);
    }
    p->nLabelAlloc = nNewSize;
    p->aLabel[j] = v->nOp;
  }
}

void sqlite3VdbeSetP4KeyInfo(Parse *pParse, Index *pIdx) {
  Vdbe *v = pParse->pVdbe;
  KeyInfo *pKeyInfo;

  pKeyInfo = sqlite3KeyInfoOfIndex(pParse, pIdx);
  if (pKeyInfo)
    sqlite3VdbeAppendP4(v, pKeyInfo, (-9));
}

void resolveAlias(Parse *pParse, ExprList *pEList, int iCol, Expr *pExpr, int nSubquery) {
  Expr *pOrig;
  Expr *pDup;
  sqlite3 *db;

  pOrig = pEList->a[iCol].pExpr;

  if (pExpr->pAggInfo)
    return;
  db = pParse->db;
  pDup = sqlite3ExprDup(db, pOrig, 0);
  if (db->mallocFailed) {
    sqlite3ExprDelete(db, pDup);
    pDup = 0;
  } else {
    Expr temp;
    incrAggFunctionDepth(pDup, nSubquery);
    if (pExpr->op == 114) {
      pDup = sqlite3ExprAddCollateString(pParse, pDup, pExpr->u.zToken);
    }
    memcpy(&temp, pDup, sizeof(Expr));
    memcpy(pDup, pExpr, sizeof(Expr));
    memcpy(pExpr, &temp, sizeof(Expr));
    if ((((pExpr)->flags & (u32)(0x1000000)) != 0)) {
      if ((pExpr->y.pWin != 0)) {
        pExpr->y.pWin->pOwner = pExpr;
      }
    }
    sqlite3ExprDeferredDelete(pParse, pDup);
  }
}

void extendFJMatch(Parse *pParse, ExprList **ppList, SrcItem *pMatch, i16 iColumn) {
  Expr *pNew = sqlite3ExprAlloc(pParse->db, 168, 0, 0);
  if (pNew) {
    pNew->iTable = pMatch->iCursor;
    pNew->iColumn = iColumn;
    pNew->y.pTab = pMatch->pSTab;

    (pNew)->flags |= (u32)(0x200000);
    *ppList = sqlite3ExprListAppend(pParse, *ppList, pNew);
  }
}

int lookupName(Parse *pParse, const char *zDb, const char *zTab, const Expr *pRight, NameContext *pNC, Expr *pExpr) {
  int i, j;
  int cnt = 0;
  int cntTab = 0;
  int nSubquery = 0;
  sqlite3 *db = pParse->db;
  SrcItem *pItem;
  SrcItem *pMatch = 0;
  NameContext *pTopNC = pNC;
  Schema *pSchema = 0;
  int eNewExprOp = 168;
  Table *pTab = 0;
  ExprList *pFJMatch = 0;
  const char *zCol = pRight->u.zToken;

  pExpr->iTable = -1;

  if (zDb) {
    if ((pNC->ncFlags & (0x000002 | 0x000004)) != 0) {
      zDb = 0;
    } else {
      for (i = 0; i < db->nDb; i++) {
        if (sqlite3StrICmp(db->aDb[i].zDbSName, zDb) == 0) {
          pSchema = db->aDb[i].pSchema;
          break;
        }
      }
      if (i == db->nDb && sqlite3StrICmp("main", zDb) == 0) {
        pSchema = db->aDb[0].pSchema;
        zDb = db->aDb[0].zDbSName;
      }
    }
  }

  do {
    ExprList *pEList;
    SrcList *pSrcList = pNC->pSrcList;

    if (pSrcList) {
      for (i = 0, pItem = pSrcList->a; i < pSrcList->nSrc; i++, pItem++) {
        pTab = pItem->pSTab;

        if (pItem->fg.isNestedFrom) {
          int hit = 0;
          Select *pSel;

          pSel = pItem->u4.pSubq->pSelect;

          pEList = pSel->pEList;

          for (j = 0; j < pEList->nExpr; j++) {
            int bRowid = 0;
            if (!sqlite3MatchEName(&pEList->a[j], zCol, zTab, zDb, &bRowid)) {
              continue;
            }
            if (bRowid == 0) {
              if (cnt > 0) {
                if (pItem->fg.isUsing == 0 || sqlite3IdListIndex(pItem->u3.pUsing, zCol) < 0 || pMatch == pItem) {
                  sqlite3ExprListDelete(db, pFJMatch);
                  pFJMatch = 0;
                } else if ((pItem->fg.jointype & 0x10) == 0) {
                  continue;
                } else if ((pItem->fg.jointype & 0x08) == 0) {
                  cnt = 0;
                  sqlite3ExprListDelete(db, pFJMatch);
                  pFJMatch = 0;
                } else {
                  extendFJMatch(pParse, &pFJMatch, pMatch, pExpr->iColumn);
                }
              }
              cnt++;
              hit = 1;
            } else if (cnt > 0) {
              continue;
            }
            cntTab++;
            pMatch = pItem;
            pExpr->iColumn = j;
            pEList->a[j].fg.bUsed = 1;

            if (pEList->a[j].fg.bUsingTerm)
              break;
          }
          if (hit || zTab == 0)
            continue;
        }

        if (zTab) {
          if (zDb) {
            if (pTab->pSchema != pSchema)
              continue;
            if (pSchema == 0 && strcmp(zDb, "*") != 0)
              continue;
          }
          if (pItem->zAlias != 0) {
            if (sqlite3StrICmp(zTab, pItem->zAlias) != 0) {
              continue;
            }
          } else if (sqlite3StrICmp(zTab, pTab->zName) != 0) {
            if (pTab->tnum != 1)
              continue;
            if (!isValidSchemaTableName(zTab, pTab, zDb))
              continue;
          }

          if ((pParse->eParseMode >= 2) && pItem->zAlias) {
            sqlite3RenameTokenRemap(pParse, 0, (void *)&pExpr->y.pTab);
          }
        }
        j = sqlite3ColumnIndex(pTab, zCol);
        if (j >= 0) {
          if (cnt > 0) {
            if (pItem->fg.isUsing == 0 || sqlite3IdListIndex(pItem->u3.pUsing, zCol) < 0) {
              sqlite3ExprListDelete(db, pFJMatch);
              pFJMatch = 0;
            } else if ((pItem->fg.jointype & 0x10) == 0) {
              continue;
            } else if ((pItem->fg.jointype & 0x08) == 0) {
              cnt = 0;
              sqlite3ExprListDelete(db, pFJMatch);
              pFJMatch = 0;
            } else {
              extendFJMatch(pParse, &pFJMatch, pMatch, pExpr->iColumn);
            }
          }
          cnt++;
          pMatch = pItem;

          pExpr->iColumn = j == pTab->iPKey ? -1 : (i16)j;
          if (pItem->fg.isNestedFrom) {
            sqlite3SrcItemColumnUsed(pItem, j);
          }
        }
        if (0 == cnt && (((pTab)->tabFlags & 0x00000200) == 0)) {
          cntTab++;
          pMatch = pItem;
        }
      }
      if (pMatch) {
        pExpr->iTable = pMatch->iCursor;

        pExpr->y.pTab = pMatch->pSTab;
        if ((pMatch->fg.jointype & (0x08 | 0x40)) != 0) {
          (pExpr)->flags |= (u32)(0x200000);
        }
        pSchema = pExpr->y.pTab->pSchema;
      }
    }

    if (cnt == 0 && zDb == 0) {
      pTab = 0;

      if (pParse->pTriggerTab != 0) {
        int op = pParse->eTriggerOp;

        if (pParse->bReturning) {
          if ((pNC->ncFlags & 0x000400) != 0 && (zTab == 0 || sqlite3StrICmp(zTab, pParse->pTriggerTab->zName) == 0 ||
                                                 isValidSchemaTableName(zTab, pParse->pTriggerTab, 0))) {
            pExpr->iTable = op != 129;
            pTab = pParse->pTriggerTab;
          }
        } else if (op != 129 && zTab && sqlite3StrICmp("new", zTab) == 0) {
          pExpr->iTable = 1;
          pTab = pParse->pTriggerTab;
        } else if (op != 128 && zTab && sqlite3StrICmp("old", zTab) == 0) {
          pExpr->iTable = 0;
          pTab = pParse->pTriggerTab;
        }
      }

      if ((pNC->ncFlags & 0x000200) != 0 && zTab != 0) {
        Upsert *pUpsert = pNC->uNC.pUpsert;
        if (pUpsert && sqlite3StrICmp("excluded", zTab) == 0) {
          pTab = pUpsert->pUpsertSrc->a[0].pSTab;
          pExpr->iTable = 2;
        }
      }

      if (pTab) {
        int iCol;
        pSchema = pTab->pSchema;
        cntTab++;
        iCol = sqlite3ColumnIndex(pTab, zCol);
        if (iCol >= 0) {
          if (pTab->iPKey == iCol)
            iCol = -1;
        } else {
          if (sqlite3IsRowid(zCol) && (((pTab)->tabFlags & 0x00000200) == 0)) {
            iCol = -1;
          } else {
            iCol = pTab->nCol;
          }
        }
        if (iCol < pTab->nCol) {
          cnt++;
          pMatch = 0;

          if (pExpr->iTable == 2) {
            if ((pParse->eParseMode >= 2)) {
              pExpr->iColumn = iCol;
              pExpr->y.pTab = pTab;
              eNewExprOp = 168;
            } else {
              pExpr->iTable = pNC->uNC.pUpsert->regData + sqlite3TableColumnToStorage(pTab, iCol);
              eNewExprOp = 176;
            }
          } else {
            pExpr->y.pTab = pTab;
            if (pParse->bReturning) {
              eNewExprOp = 176;
              pExpr->op2 = 168;
              pExpr->iColumn = iCol;
              pExpr->iTable =
                  pNC->uNC.iBaseReg + (pTab->nCol + 1) * pExpr->iTable + sqlite3TableColumnToStorage(pTab, iCol) + 1;
            } else {
              pExpr->iColumn = (i16)iCol;
              eNewExprOp = 78;

              if (iCol < 0) {
                pExpr->affExpr = 0x44;
              } else if (pExpr->iTable == 0) {
                pParse->oldmask |= (iCol >= 32 ? 0xffffffff : (((u32)1) << iCol));
              } else {
                pParse->newmask |= (iCol >= 32 ? 0xffffffff : (((u32)1) << iCol));
              }
            }
          }
        }
      }
    }

    if (cnt == 0 && cntTab >= 1 && pMatch && (pNC->ncFlags & (0x000020 | 0x000008)) == 0 && sqlite3IsRowid(zCol) &&
        ((((pMatch->pSTab)->tabFlags & 0x00000200) == 0) || pMatch->fg.isNestedFrom)) {
      cnt = cntTab;

      if (pMatch->fg.isNestedFrom == 0)
        pExpr->iColumn = -1;
      pExpr->affExpr = 0x44;
    }

    if (cnt == 0 && (pNC->ncFlags & 0x000080) != 0 && zTab == 0) {
      pEList = pNC->uNC.pEList;

      for (j = 0; j < pEList->nExpr; j++) {
        char *zAs = pEList->a[j].zEName;
        if (pEList->a[j].fg.eEName == 0 && sqlite3_stricmp(zAs, zCol) == 0) {
          Expr *pOrig;

          pOrig = pEList->a[j].pExpr;
          if ((pNC->ncFlags & 0x000001) == 0 && (((pOrig)->flags & (u32)(0x000010)) != 0)) {
            sqlite3ErrorMsg(pParse, "misuse of aliased aggregate %s", zAs);
            return 2;
          }
          if ((((pOrig)->flags & (u32)(0x008000)) != 0) && ((pNC->ncFlags & 0x004000) == 0 || pNC != pTopNC)) {
            sqlite3ErrorMsg(pParse, "misuse of aliased window function %s", zAs);
            return 2;
          }
          if (sqlite3ExprVectorSize(pOrig) != 1) {
            sqlite3ErrorMsg(pParse, "row value misused");
            return 2;
          }
          resolveAlias(pParse, pEList, j, pExpr, nSubquery);
          cnt = 1;
          pMatch = 0;

          if ((pParse->eParseMode >= 2)) {
            sqlite3RenameTokenRemap(pParse, 0, (void *)pExpr);
          }
          goto lookupname_end;
        }
      }
    }

    if (cnt)
      break;
    pNC = pNC->pNext;
    nSubquery++;
  } while (pNC);

  if (cnt == 0 && zTab == 0) {
    if ((((pExpr)->flags & (u32)(0x000080)) != 0) && areDoubleQuotedStringsEnabled(db, pTopNC)) {
      sqlite3_log(SQLITE_WARNING, "double-quoted string literal: \"%w\"", zCol);

      pExpr->op = 118;
      memset(&pExpr->y, 0, sizeof(pExpr->y));
      return 1;
    }
    if (sqlite3ExprIdToTrueFalse(pExpr)) {
      return 1;
    }
  }

  if (cnt != 1) {
    const char *zErr;
    if (pFJMatch) {
      if (pFJMatch->nExpr == cnt - 1) {
        if ((((pExpr)->flags & (u32)(0x800000)) != 0)) {
          (pExpr)->flags &= ~(u32)(0x800000);
        } else {
          sqlite3ExprDelete(db, pExpr->pLeft);
          pExpr->pLeft = 0;
          sqlite3ExprDelete(db, pExpr->pRight);
          pExpr->pRight = 0;
        }
        extendFJMatch(pParse, &pFJMatch, pMatch, pExpr->iColumn);
        pExpr->op = 172;
        pExpr->u.zToken = (char*)("coalesce");
        pExpr->x.pList = pFJMatch;
        pExpr->affExpr = 0x58;
        cnt = 1;
        goto lookupname_end;
      } else {
        sqlite3ExprListDelete(db, pFJMatch);
        pFJMatch = 0;
      }
    }
    zErr = cnt == 0 ? "no such column" : "ambiguous column name";
    if (zDb) {
      sqlite3ErrorMsg(pParse, "%s: %s.%s.%s", zErr, zDb, zTab, zCol);
    } else if (zTab) {
      sqlite3ErrorMsg(pParse, "%s: %s.%s", zErr, zTab, zCol);
    } else if (cnt == 0 && (((pRight)->flags & (u32)(0x000080)) != 0)) {
      sqlite3ErrorMsg(pParse,
                      "%s: \"%s\" - should this be a"
                      " string literal in single-quotes?",
                      zErr, zCol);
    } else {
      sqlite3ErrorMsg(pParse, "%s: %s", zErr, zCol);
    }
    sqlite3RecordErrorOffsetOfExpr(pParse->db, pExpr);
    pParse->checkSchema = 1;
    pTopNC->nNcErr++;
    eNewExprOp = 122;
  }

  if (!(((pExpr)->flags & (u32)((0x010000 | 0x800000))) != 0)) {
    sqlite3ExprDelete(db, pExpr->pLeft);
    pExpr->pLeft = 0;
    sqlite3ExprDelete(db, pExpr->pRight);
    pExpr->pRight = 0;
    (pExpr)->flags |= (u32)(0x800000);
  }

  if (pMatch) {
    if (pExpr->iColumn >= 0) {
      pMatch->colUsed |= sqlite3ExprColUsed(pExpr);
    } else {
      pMatch->fg.rowidUsed = 1;
    }
  }

  pExpr->op = eNewExprOp;
lookupname_end:
  if (cnt == 1) {
    if (pParse->db->xAuth && (pExpr->op == 168 || pExpr->op == 78)) {
      sqlite3AuthRead(pParse, pExpr, pSchema, pNC->pSrcList);
    }

    for (;;) {
      pTopNC->nRef++;
      if (pTopNC == pNC)
        break;
      pTopNC = pTopNC->pNext;
    }
    return 1;
  } else {
    return 2;
  }
}

void notValidImpl(Parse *pParse, NameContext *pNC, const char *zMsg, Expr *pExpr, Expr *pError) {
  const char *zIn = "partial index WHERE clauses";
  if (pNC->ncFlags & 0x000020)
    zIn = "index expressions";

  else if (pNC->ncFlags & 0x000004)
    zIn = "CHECK constraints";

  else if (pNC->ncFlags & 0x000008)
    zIn = "generated columns";

  sqlite3ErrorMsg(pParse, "%s prohibited in %s", zMsg, zIn);
  if (pExpr)
    pExpr->op = 122;
  sqlite3RecordErrorOffsetOfExpr(pParse->db, pError);
}

int resolveAsName(Parse *pParse, ExprList *pEList, Expr *pE) {
  int i;

  (void)(pParse);

  if (pE->op == 60) {
    const char *zCol;

    zCol = pE->u.zToken;
    for (i = 0; i < pEList->nExpr; i++) {
      if (pEList->a[i].fg.eEName == 0 && sqlite3_stricmp(pEList->a[i].zEName, zCol) == 0) {
        return i + 1;
      }
    }
  }
  return 0;
}

int resolveOrderByTermToExprList(Parse *pParse, Select *pSelect, Expr *pE) {
  int i;
  ExprList *pEList;
  NameContext nc;
  sqlite3 *db;
  int rc;
  u8 savedSuppErr;

  pEList = pSelect->pEList;

  memset(&nc, 0, sizeof(nc));
  nc.pParse = pParse;
  nc.pSrcList = pSelect->pSrc;
  nc.uNC.pEList = pEList;
  nc.ncFlags = 0x000001 | 0x000080 | 0x080000;
  nc.nNcErr = 0;
  db = pParse->db;
  savedSuppErr = db->suppressErr;
  db->suppressErr = 1;
  rc = sqlite3ResolveExprNames(&nc, pE);
  db->suppressErr = savedSuppErr;
  if (rc)
    return 0;

  for (i = 0; i < pEList->nExpr; i++) {
    if (sqlite3ExprCompare(0, pEList->a[i].pExpr, pE, -1) < 2) {
      return i + 1;
    }
  }

  return 0;
}

void resolveOutOfRangeError(Parse *pParse, const char *zType, int i, int mx, Expr *pError) {
  sqlite3ErrorMsg(pParse,
                  "%r %s BY term out of range - should be "
                  "between 1 and %d",
                  i, zType, mx);
  sqlite3RecordErrorOffsetOfExpr(pParse->db, pError);
}

int resolveCompoundOrderBy(Parse *pParse, Select *pSelect) {
  int i;
  ExprList *pOrderBy;
  ExprList *pEList;
  sqlite3 *db;
  int moreToDo = 1;

  pOrderBy = pSelect->pOrderBy;
  if (pOrderBy == 0)
    return 0;
  db = pParse->db;
  if (pOrderBy->nExpr > db->aLimit[SQLITE_LIMIT_COLUMN]) {
    sqlite3ErrorMsg(pParse, "too many terms in ORDER BY clause");
    return 1;
  }
  for (i = 0; i < pOrderBy->nExpr; i++) {
    pOrderBy->a[i].fg.done = 0;
  }
  pSelect->pNext = 0;
  while (pSelect->pPrior) {
    pSelect->pPrior->pNext = pSelect;
    pSelect = pSelect->pPrior;
  }
  while (pSelect && moreToDo) {
    struct ExprList_item *pItem;
    moreToDo = 0;
    pEList = pSelect->pEList;

    for (i = 0, pItem = pOrderBy->a; i < pOrderBy->nExpr; i++, pItem++) {
      int iCol = -1;
      Expr *pE, *pDup;
      if (pItem->fg.done)
        continue;
      pE = sqlite3ExprSkipCollateAndLikely(pItem->pExpr);
      if (pE == 0)
        continue;
      if (sqlite3ExprIsInteger(pE, &iCol, 0)) {
        if (iCol <= 0 || iCol > pEList->nExpr) {
          resolveOutOfRangeError(pParse, "ORDER", i + 1, pEList->nExpr, pE);
          return 1;
        }
      } else {
        iCol = resolveAsName(pParse, pEList, pE);
        if (iCol == 0) {
          pDup = sqlite3ExprDup(db, pE, 0);
          if (!db->mallocFailed) {
            iCol = resolveOrderByTermToExprList(pParse, pSelect, pDup);
            if ((pParse->eParseMode >= 2) && iCol > 0) {
              resolveOrderByTermToExprList(pParse, pSelect, pE);
            }
          }
          sqlite3ExprDelete(db, pDup);
        }
      }
      if (iCol > 0) {
        if (!(pParse->eParseMode >= 2)) {
          Expr *pNew = sqlite3ExprInt32(db, iCol);
          if (pNew == 0)
            return 1;
          if (pItem->pExpr == pE) {
            pItem->pExpr = pNew;
          } else {
            Expr *pParent = pItem->pExpr;

            while (pParent->pLeft->op == 114)
              pParent = pParent->pLeft;

            pParent->pLeft = pNew;
          }
          sqlite3ExprDelete(db, pE);
          pItem->u.x.iOrderByCol = (u16)iCol;
        }
        pItem->fg.done = 1;
      } else {
        moreToDo = 1;
      }
    }
    pSelect = pSelect->pNext;
  }
  for (i = 0; i < pOrderBy->nExpr; i++) {
    if (pOrderBy->a[i].fg.done == 0) {
      sqlite3ErrorMsg(pParse,
                      "%r ORDER BY term does not match any "
                      "column in the result set",
                      i + 1);
      return 1;
    }
  }
  return 0;
}

int sqlite3ResolveOrderGroupBy(Parse *pParse, Select *pSelect, ExprList *pOrderBy, const char *zType) {
  int i;
  sqlite3 *db = pParse->db;
  ExprList *pEList;
  struct ExprList_item *pItem;

  if (pOrderBy == 0 || pParse->db->mallocFailed || (pParse->eParseMode >= 2))
    return 0;
  if (pOrderBy->nExpr > db->aLimit[SQLITE_LIMIT_COLUMN]) {
    sqlite3ErrorMsg(pParse, "too many terms in %s BY clause", zType);
    return 1;
  }
  pEList = pSelect->pEList;

  for (i = 0, pItem = pOrderBy->a; i < pOrderBy->nExpr; i++, pItem++) {
    if (pItem->u.x.iOrderByCol) {
      if (pItem->u.x.iOrderByCol > pEList->nExpr) {
        resolveOutOfRangeError(pParse, zType, i + 1, pEList->nExpr, 0);
        return 1;
      }
      resolveAlias(pParse, pEList, pItem->u.x.iOrderByCol - 1, pItem->pExpr, 0);
    }
  }
  return 0;
}

void sqlite3ResolveSelectNames(Parse *pParse, Select *p, NameContext *pOuterNC) {
  Walker w;

  w.xExprCallback = resolveExprStep;
  w.xSelectCallback = resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.pParse = pParse;
  w.u.pNC = pOuterNC;
  sqlite3WalkSelect(&w, p);
}

int sqlite3ResolveSelfReference(Parse *pParse, Table *pTab, int type, Expr *pExpr, ExprList *pList) {
  SrcList *pSrc;
  NameContext sNC;
  int rc;
  union {
    SrcList sSrc;
    u8 srcSpace[(offsetof(SrcList, a) + sizeof(SrcItem))];
  } uSrc;

  memset(&sNC, 0, sizeof(sNC));
  memset(&uSrc, 0, sizeof(uSrc));
  pSrc = &uSrc.sSrc;
  if (pTab) {
    pSrc->nSrc = 1;
    pSrc->a[0].zName = pTab->zName;
    pSrc->a[0].pSTab = pTab;
    pSrc->a[0].iCursor = -1;
    if (pTab->pSchema != pParse->db->aDb[1].pSchema) {
      type |= 0x040000;
    }
  }
  sNC.pParse = pParse;
  sNC.pSrcList = pSrc;
  sNC.ncFlags = type | 0x010000;
  if ((rc = sqlite3ResolveExprNames(&sNC, pExpr)) != SQLITE_OK)
    return rc;
  if (pList)
    rc = sqlite3ResolveExprListNames(&sNC, pList);
  return rc;
}

Expr *sqlite3ExprAddCollateToken(const Parse *pParse, Expr *pExpr, const Token *pCollName, int dequote) {
  if (pCollName->n > 0) {
    Expr *pNew = sqlite3ExprAlloc(pParse->db, 114, pCollName, dequote);
    if (pNew) {
      pNew->pLeft = pExpr;
      pNew->flags |= 0x000200 | 0x002000;
      pExpr = pNew;
    }
  }
  return pExpr;
}

Expr *sqlite3ExprAddCollateString(const Parse *pParse, Expr *pExpr, const char *zC) {
  Token s;

  sqlite3TokenInit(&s, (char *)zC);
  return sqlite3ExprAddCollateToken(pParse, pExpr, &s, 0);
}

CollSeq *sqlite3ExprCollSeq(Parse *pParse, const Expr *pExpr) {
  sqlite3 *db = pParse->db;
  CollSeq *pColl = 0;
  const Expr *p = pExpr;
  while (p) {
    int op = p->op;
    if (op == 176)
      op = p->op2;
    if ((op == 170 && p->y.pTab != 0) || op == 168 || op == 78) {
      int j;

      if ((j = p->iColumn) >= 0) {
        const char *zColl = sqlite3ColumnColl(&p->y.pTab->aCol[j]);
        pColl = sqlite3FindCollSeq(db, ((db)->enc), zColl, 0);
      }
      break;
    }
    if (op == 36 || op == 173) {
      p = p->pLeft;
      continue;
    }
    if (op == 177 || (op == 172 && p->affExpr == 0x58)) {
      p = p->x.pList->a[0].pExpr;
      continue;
    }
    if (op == 114) {
      pColl = sqlite3GetCollSeq(pParse, ((db)->enc), 0, p->u.zToken);
      break;
    }
    if (p->flags & 0x000200) {
      if (p->pLeft && (p->pLeft->flags & 0x000200) != 0) {
        p = p->pLeft;
      } else {
        Expr *pNext = p->pRight;

        if ((((p)->flags & 0x001000) == 0) && p->x.pList != 0 && !db->mallocFailed) {
          int i;
          for (i = 0; i < p->x.pList->nExpr; i++) {
            if ((((p->x.pList->a[i].pExpr)->flags & (u32)(0x000200)) != 0)) {
              pNext = p->x.pList->a[i].pExpr;
              break;
            }
          }
        }
        p = pNext;
      }
    } else {
      break;
    }
  }
  if (sqlite3CheckCollSeq(pParse, pColl)) {
    pColl = 0;
  }
  return pColl;
}

CollSeq *sqlite3ExprNNCollSeq(Parse *pParse, const Expr *pExpr) {
  CollSeq *p = sqlite3ExprCollSeq(pParse, pExpr);
  if (p == 0)
    p = pParse->db->pDfltColl;

  return p;
}

int sqlite3ExprCollSeqMatch(Parse *pParse, const Expr *pE1, const Expr *pE2) {
  CollSeq *pColl1 = sqlite3ExprNNCollSeq(pParse, pE1);
  CollSeq *pColl2 = sqlite3ExprNNCollSeq(pParse, pE2);
  return sqlite3StrICmp(pColl1->zName, pColl2->zName) == 0;
}

CollSeq *sqlite3BinaryCompareCollSeq(Parse *pParse, const Expr *pLeft, const Expr *pRight) {
  CollSeq *pColl;

  if (pLeft->flags & 0x000200) {
    pColl = sqlite3ExprCollSeq(pParse, pLeft);
  } else if (pRight && (pRight->flags & 0x000200) != 0) {
    pColl = sqlite3ExprCollSeq(pParse, pRight);
  } else {
    pColl = sqlite3ExprCollSeq(pParse, pLeft);
    if (!pColl) {
      pColl = sqlite3ExprCollSeq(pParse, pRight);
    }
  }
  return pColl;
}

CollSeq *sqlite3ExprCompareCollSeq(Parse *pParse, const Expr *p) {
  if ((((p)->flags & (u32)(0x000400)) != 0)) {
    return sqlite3BinaryCompareCollSeq(pParse, p->pRight, p->pLeft);
  } else {
    return sqlite3BinaryCompareCollSeq(pParse, p->pLeft, p->pRight);
  }
}

int codeCompare(Parse *pParse, Expr *pLeft, Expr *pRight, int opcode, int in1, int in2, int dest, int jumpIfNull,
                int isCommuted) {
  int p5;
  int addr;
  CollSeq *p4;

  if (pParse->nErr)
    return 0;
  if (isCommuted) {
    p4 = sqlite3BinaryCompareCollSeq(pParse, pRight, pLeft);
  } else {
    p4 = sqlite3BinaryCompareCollSeq(pParse, pLeft, pRight);
  }
  p5 = binaryCompareP5(pLeft, pRight, jumpIfNull);
  addr = sqlite3VdbeAddOp4(pParse->pVdbe, opcode, in2, dest, in1, (const char*)((void *)p4), (-2));
  sqlite3VdbeChangeP5(pParse->pVdbe, (u16)p5);
  return addr;
}

Expr *sqlite3ExprForVectorField(Parse *pParse, Expr *pVector, int iField, int nField) {
  Expr *pRet;
  if (pVector->op == 139) {
    pRet = sqlite3PExpr(pParse, 178, 0, 0);
    if (pRet) {
      (pRet)->flags |= (u32)(0x020000);
      pRet->iTable = nField;
      pRet->iColumn = iField;
      pRet->pLeft = pVector;
    }
  } else {
    if (pVector->op == 177) {
      Expr **ppVector;

      ppVector = &pVector->x.pList->a[iField].pExpr;
      pVector = *ppVector;
      if ((pParse->eParseMode >= 2)) {
        *ppVector = 0;
        return pVector;
      }
    }
    pRet = sqlite3ExprDup(pParse->db, pVector, 0);
  }
  return pRet;
}

int exprCodeSubselect(Parse *pParse, Expr *pExpr) {
  int reg = 0;

  if (pExpr->op == 139) {
    reg = sqlite3CodeSubselect(pParse, pExpr);
  }

  return reg;
}

int exprVectorRegister(Parse *pParse, Expr *pVector, int iField, int regSelect, Expr **ppExpr, int *pRegFree) {
  u8 op = pVector->op;

  if (op == 176) {
    *ppExpr = sqlite3VectorFieldSubexpr(pVector, iField);
    return pVector->iTable + iField;
  }
  if (op == 139) {
    *ppExpr = pVector->x.pSelect->pEList->a[iField].pExpr;
    return regSelect + iField;
  }
  if (op == 177) {
    *ppExpr = pVector->x.pList->a[iField].pExpr;
    return sqlite3ExprCodeTemp(pParse, *ppExpr, pRegFree);
  }
  return 0;
}

void codeVectorCompare(Parse *pParse, Expr *pExpr, int dest, u8 op, u8 p5) {
  Vdbe *v = pParse->pVdbe;
  Expr *pLeft = pExpr->pLeft;
  Expr *pRight = pExpr->pRight;
  int nLeft = sqlite3ExprVectorSize(pLeft);
  int i;
  int regLeft = 0;
  int regRight = 0;
  u8 opx = op;
  int addrCmp = 0;
  int addrDone = sqlite3VdbeMakeLabel(pParse);
  int isCommuted = (((pExpr)->flags & (u32)(0x000400)) != 0);

  if (pParse->nErr)
    return;
  if (nLeft != sqlite3ExprVectorSize(pRight)) {
    sqlite3ErrorMsg(pParse, "row value misused");
    return;
  }

  if (op == 56)
    opx = 57;
  if (op == 58)
    opx = 55;
  if (op == 53)
    opx = 54;

  regLeft = exprCodeSubselect(pParse, pLeft);
  regRight = exprCodeSubselect(pParse, pRight);

  sqlite3VdbeAddOp2(v, 73, 1, dest);
  for (i = 0; 1; i++) {
    int regFree1 = 0, regFree2 = 0;
    Expr *pL = 0, *pR = 0;
    int r1, r2;

    if (addrCmp)
      sqlite3VdbeJumpHere(v, addrCmp);
    r1 = exprVectorRegister(pParse, pLeft, i, regLeft, &pL, &regFree1);
    r2 = exprVectorRegister(pParse, pRight, i, regRight, &pR, &regFree2);
    addrCmp = sqlite3VdbeCurrentAddr(v);
    codeCompare(pParse, pL, pR, opx, r1, r2, addrDone, p5, isCommuted);
    sqlite3ReleaseTempReg(pParse, regFree1);
    sqlite3ReleaseTempReg(pParse, regFree2);
    if ((opx == 57 || opx == 55) && i < nLeft - 1) {
      addrCmp = sqlite3VdbeAddOp0(v, 59);
    }
    if (p5 == 0x80) {
      sqlite3VdbeAddOp2(v, 73, 0, dest);
    } else {
      sqlite3VdbeAddOp3(v, 94, r1, dest, r2);
    }
    if (i == nLeft - 1) {
      break;
    }
    if (opx == 54) {
      sqlite3VdbeAddOp2(v, 52, dest, addrDone);
    } else {
      sqlite3VdbeAddOp2(v, 9, 0, addrDone);
      if (i == nLeft - 2)
        opx = op;
    }
  }
  sqlite3VdbeJumpHere(v, addrCmp);
  sqlite3VdbeResolveLabel(v, addrDone);
  if (op == 53) {
    sqlite3VdbeAddOp2(v, 19, dest, dest);
  }
}

int sqlite3ExprCheckHeight(Parse *pParse, int nHeight) {
  int rc = SQLITE_OK;
  int mxHeight = pParse->db->aLimit[SQLITE_LIMIT_EXPR_DEPTH];
  if (nHeight > mxHeight) {
    sqlite3ErrorMsg(pParse, "Expression tree is too large (maximum depth %d)", mxHeight);
    rc = SQLITE_ERROR;
  }
  return rc;
}

void sqlite3ExprSetHeightAndFlags(Parse *pParse, Expr *p) {
  if (pParse->nErr)
    return;
  exprSetHeight(p);
  sqlite3ExprCheckHeight(pParse, p->nHeight);
}

Expr *sqlite3PExpr(Parse *pParse, int op, Expr *pLeft, Expr *pRight) {
  Expr *p;
  p = (Expr*)(sqlite3DbMallocRawNN(pParse->db, sizeof(Expr)));
  if (p) {
    memset(p, 0, sizeof(Expr));
    p->op = op & 0xff;
    p->iAgg = -1;
    sqlite3ExprAttachSubtrees(pParse->db, p, pLeft, pRight);
    sqlite3ExprCheckHeight(pParse, p->nHeight);
  } else {
    sqlite3ExprDelete(pParse->db, pLeft);
    sqlite3ExprDelete(pParse->db, pRight);
  }
  return p;
}

void sqlite3PExprAddSelect(Parse *pParse, Expr *pExpr, Select *pSelect) {
  if (pExpr) {
    pExpr->x.pSelect = pSelect;
    (pExpr)->flags |= (u32)(0x001000 | 0x400000);
    sqlite3ExprSetHeightAndFlags(pParse, pExpr);
  } else {
    sqlite3SelectDelete(pParse->db, pSelect);
  }
}

Select *sqlite3ExprListToValues(Parse *pParse, int nElem, ExprList *pEList) {
  int ii;
  Select *pRet = 0;

  for (ii = 0; ii < pEList->nExpr; ii++) {
    Select *pSel;
    Expr *pExpr = pEList->a[ii].pExpr;
    int nExprElem;
    if (pExpr->op == 177) {
      nExprElem = pExpr->x.pList->nExpr;
    } else {
      nExprElem = 1;
    }
    if (nExprElem != nElem) {
      sqlite3ErrorMsg(pParse, "IN(...) element has %d term%s - expected %d", nExprElem, nExprElem > 1 ? "s" : "",
                      nElem);
      break;
    }

    pSel = sqlite3SelectNew(pParse, pExpr->x.pList, 0, 0, 0, 0, 0, 0x0000200, 0);
    pExpr->x.pList = 0;
    if (pSel) {
      if (pRet) {
        pSel->op = 136;
        pSel->pPrior = pRet;
      }
      pRet = pSel;
    }
  }

  if (pRet && pRet->pPrior) {
    pRet->selFlags |= 0x0000400;
  }
  sqlite3ExprListDelete(pParse->db, pEList);
  return pRet;
}

Expr *sqlite3ExprAnd(Parse *pParse, Expr *pLeft, Expr *pRight) {
  sqlite3 *db = pParse->db;
  if (pLeft == 0) {
    return pRight;
  } else if (pRight == 0) {
    return pLeft;
  } else {
    u32 f = pLeft->flags | pRight->flags;
    if ((f & (0x000001 | 0x000002 | 0x20000000 | 0x000008)) == 0x20000000 && !(pParse->eParseMode >= 2)) {
      sqlite3ExprDeferredDelete(pParse, pLeft);
      sqlite3ExprDeferredDelete(pParse, pRight);
      return sqlite3ExprInt32(db, 0);
    } else {
      return sqlite3PExpr(pParse, 44, pLeft, pRight);
    }
  }
}

Expr *sqlite3ExprFunction(Parse *pParse, ExprList *pList, const Token *pToken, int eDistinct) {
  Expr *pNew;
  sqlite3 *db = pParse->db;

  pNew = sqlite3ExprAlloc(db, 172, pToken, 1);
  if (pNew == 0) {
    sqlite3ExprListDelete(db, pList);
    return 0;
  }

  pNew->w.iOfst = (int)(pToken->z - pParse->zTail);
  if (pList && pList->nExpr > pParse->db->aLimit[SQLITE_LIMIT_FUNCTION_ARG] && !pParse->nested) {
    sqlite3ErrorMsg(pParse, "too many arguments on function %T", pToken);
  }
  pNew->x.pList = pList;
  (pNew)->flags |= (u32)(0x000008);

  sqlite3ExprSetHeightAndFlags(pParse, pNew);
  if (eDistinct == 0x0000001)
    (pNew)->flags |= (u32)(0x000004);
  return pNew;
}

void sqlite3ExprOrderByAggregateError(Parse *pParse, Expr *p) {
  sqlite3ErrorMsg(pParse, "ORDER BY may not be used with non-aggregate %#T()", p);
}

void sqlite3ExprAddFunctionOrderBy(Parse *pParse, Expr *pExpr, ExprList *pOrderBy) {
  Expr *pOB;
  sqlite3 *db = pParse->db;
  if (pOrderBy == 0) {
    return;
  }
  if (pExpr == 0) {
    sqlite3ExprListDelete(db, pOrderBy);
    return;
  }

  if (pExpr->x.pList == 0 || (pExpr->x.pList->nExpr == 0)) {
    sqlite3ParserAddCleanup(pParse, sqlite3ExprListDeleteGeneric, pOrderBy);
    return;
  }
  if ((((((pExpr))->flags & (u32)(0x1000000)) != 0) && pExpr->y.pWin->eFrmType != 167)) {
    sqlite3ExprOrderByAggregateError(pParse, pExpr);
    sqlite3ExprListDelete(db, pOrderBy);
    return;
  }
  if (pOrderBy->nExpr > db->aLimit[SQLITE_LIMIT_COLUMN]) {
    sqlite3ErrorMsg(pParse, "too many terms in ORDER BY clause");
    sqlite3ExprListDelete(db, pOrderBy);
    return;
  }

  pOB = sqlite3ExprAlloc(db, 146, 0, 0);
  if (pOB == 0) {
    sqlite3ExprListDelete(db, pOrderBy);
    return;
  }
  pOB->x.pList = pOrderBy;

  pExpr->pLeft = pOB;
  (pOB)->flags |= (u32)(0x020000);
}

void sqlite3ExprFunctionUsable(Parse *pParse, const Expr *pExpr, const FuncDef *pDef) {
  if ((((pExpr)->flags & (u32)(0x40000000)) != 0) || pParse->prepFlags & SQLITE_PREPARE_FROM_DDL) {
    if ((pDef->funcFlags & 0x00080000) != 0 || (pParse->db->flags & 0x00000080) == 0) {
      sqlite3ErrorMsg(pParse, "unsafe use of %#T()", pExpr);
    }
  }
}

void sqlite3ExprAssignVarNumber(Parse *pParse, Expr *pExpr, u32 n) {
  sqlite3 *db = pParse->db;
  const char *z;
  ynVar x;

  if (pExpr == 0)
    return;

  z = pExpr->u.zToken;

  if (z[1] == 0) {
    x = (ynVar)(++pParse->nVar);
  } else {
    int doAdd = 0;
    if (z[0] == '?') {
      i64 i;
      int bOk;
      if (n == 2) {
        i = z[1] - '0';
        bOk = 1;
      } else {
        bOk = 0 == sqlite3Atoi64(&z[1], &i, n - 1, SQLITE_UTF8);
      };
      if (bOk == 0 || i < 1 || i > db->aLimit[SQLITE_LIMIT_VARIABLE_NUMBER]) {
        sqlite3ErrorMsg(pParse, "variable number must be between ?1 and ?%d", db->aLimit[SQLITE_LIMIT_VARIABLE_NUMBER]);
        sqlite3RecordErrorOffsetOfExpr(pParse->db, pExpr);
        return;
      }
      x = (ynVar)i;
      if (x > pParse->nVar) {
        pParse->nVar = (int)x;
        doAdd = 1;
      } else if (sqlite3VListNumToName(pParse->pVList, x) == 0) {
        doAdd = 1;
      }
    } else {
      x = (ynVar)sqlite3VListNameToNum(pParse->pVList, z, n);
      if (x == 0) {
        x = (ynVar)(++pParse->nVar);
        doAdd = 1;
      }
    }
    if (doAdd) {
      pParse->pVList = sqlite3VListAdd(db, pParse->pVList, z, n, x);
    }
  }
  pExpr->iColumn = x;
  if (x > db->aLimit[SQLITE_LIMIT_VARIABLE_NUMBER]) {
    sqlite3ErrorMsg(pParse, "too many SQL variables");
    sqlite3RecordErrorOffsetOfExpr(pParse->db, pExpr);
  }
}

int sqlite3ExprDeferredDelete(Parse *pParse, Expr *pExpr) {
  return 0 == sqlite3ParserAddCleanup(pParse, sqlite3ExprDeleteGeneric, pExpr);
}

void sqlite3ExprUnmapAndDelete(Parse *pParse, Expr *p) {
  if (p) {
    if ((pParse->eParseMode >= 2)) {
      sqlite3RenameExprUnmap(pParse, p);
    }
    sqlite3ExprDeleteNN(pParse->db, p);
  }
}

ExprList *sqlite3ExprListAppend(Parse *pParse, ExprList *pList, Expr *pExpr) {
  struct ExprList_item *pItem;
  if (pList == 0) {
    return sqlite3ExprListAppendNew(pParse->db, pExpr);
  }
  if (pList->nAlloc < pList->nExpr + 1) {
    return sqlite3ExprListAppendGrow(pParse->db, pList, pExpr);
  }
  pItem = &pList->a[pList->nExpr++];
  *pItem = zeroItem;
  pItem->pExpr = pExpr;
  return pList;
}

ExprList *sqlite3ExprListAppendVector(Parse *pParse, ExprList *pList, IdList *pColumns, Expr *pExpr) {
  sqlite3 *db = pParse->db;
  int n;
  int i;
  int iFirst = pList ? pList->nExpr : 0;

  if (pColumns == 0)
    goto vector_append_error;
  if (pExpr == 0)
    goto vector_append_error;

  if (pExpr->op != 139 && pColumns->nId != (n = sqlite3ExprVectorSize(pExpr))) {
    sqlite3ErrorMsg(pParse, "%d columns assigned %d values", pColumns->nId, n);
    goto vector_append_error;
  }

  for (i = 0; i < pColumns->nId; i++) {
    Expr *pSubExpr = sqlite3ExprForVectorField(pParse, pExpr, i, pColumns->nId);

    if (pSubExpr == 0)
      continue;
    pList = sqlite3ExprListAppend(pParse, pList, pSubExpr);
    if (pList) {
      pList->a[pList->nExpr - 1].zEName = pColumns->a[i].zName;
      pColumns->a[i].zName = 0;
    }
  }

  if (!db->mallocFailed && pExpr->op == 139 && (pList != 0)) {
    Expr *pFirst = pList->a[iFirst].pExpr;

    pFirst->pRight = pExpr;
    pExpr = 0;

    pFirst->iTable = pColumns->nId;
  }

vector_append_error:
  sqlite3ExprUnmapAndDelete(pParse, pExpr);
  sqlite3IdListDelete(db, pColumns);
  return pList;
}

void sqlite3ExprListSetName(Parse *pParse, ExprList *pList, const Token *pName, int dequote) {
  if (pList) {
    struct ExprList_item *pItem;

    pItem = &pList->a[pList->nExpr - 1];

    pItem->zEName = sqlite3DbStrNDup(pParse->db, pName->z, pName->n);
    if (dequote) {
      sqlite3Dequote(pItem->zEName);
      if ((pParse->eParseMode >= 2)) {
        sqlite3RenameTokenMap(pParse, (const void *)pItem->zEName, pName);
      }
    }
  }
}

void sqlite3ExprListSetSpan(Parse *pParse, ExprList *pList, const char *zStart, const char *zEnd) {
  sqlite3 *db = pParse->db;

  if (pList) {
    struct ExprList_item *pItem = &pList->a[pList->nExpr - 1];

    if (pItem->zEName == 0) {
      pItem->zEName = sqlite3DbSpanDup(db, zStart, zEnd);
      pItem->fg.eEName = 1;
    }
  }
}

void sqlite3ExprListCheckLength(Parse *pParse, ExprList *pEList, const char *zObject) {
  int mx = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];
  if (pEList && pEList->nExpr > mx) {
    sqlite3ErrorMsg(pParse, "too many columns in %s", zObject);
  }
}

int exprComputeOperands(Parse *pParse, Expr *pExpr, int *pR1, int *pR2, int *pFree1, int *pFree2) {
  int addrIsNull;
  int r1, r2;
  Vdbe *v = pParse->pVdbe;

  if (exprEvalRhsFirst(pExpr) && sqlite3ExprCanBeNull(pExpr->pRight)) {
    r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, pFree2);
    addrIsNull = sqlite3VdbeAddOp1(v, 51, r2);
  } else {
    r2 = 0;
    addrIsNull = 0;
  }
  r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, pFree1);
  if (addrIsNull == 0) {
    if ((((pExpr->pRight)->flags & (u32)(0x400000)) != 0) && sqlite3ExprCanBeNull(pExpr->pLeft)) {
      addrIsNull = sqlite3VdbeAddOp1(v, 51, r1);
    }
    r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, pFree2);
  }
  *pR1 = r1;
  *pR2 = r2;
  return addrIsNull;
}

int exprIsConst(Parse *pParse, Expr *p, int initFlag) {
  Walker w;
  w.eCode = initFlag;
  w.pParse = pParse;
  w.xExprCallback = exprNodeIsConstant;
  w.xSelectCallback = sqlite3SelectWalkFail;

  sqlite3WalkExpr(&w, p);
  return w.eCode;
}

int sqlite3ExprIsConstant(Parse *pParse, Expr *p) {
  return exprIsConst(pParse, p, 1);
}

int sqlite3ExprIsConstantNotJoin(Parse *pParse, Expr *p) {
  return exprIsConst(pParse, p, 2);
}

int sqlite3ExprIsConstantOrGroupBy(Parse *pParse, Expr *p, ExprList *pGroupBy) {
  Walker w;
  w.eCode = 1;
  w.xExprCallback = exprNodeIsConstantOrGroupBy;
  w.xSelectCallback = 0;
  w.u.pGroupBy = pGroupBy;
  w.pParse = pParse;
  sqlite3WalkExpr(&w, p);
  return w.eCode;
}

int sqlite3InRhsIsConstant(Parse *pParse, Expr *pIn) {
  Expr *pLHS;
  int res;

  pLHS = pIn->pLeft;
  pIn->pLeft = 0;
  res = sqlite3ExprIsConstant(pParse, pIn);
  pIn->pLeft = pLHS;
  return res;
}

int sqlite3FindInIndex(Parse *pParse, Expr *pX, u32 inFlags, int *prRhsHasNull, int *aiMap, int *piTab) {
  Select *p;
  int eType = 0;
  int iTab;
  int mustBeUnique;
  Vdbe *v = sqlite3GetVdbe(pParse);

  mustBeUnique = (inFlags & 0x0004) != 0;
  iTab = pParse->nTab++;

  if (prRhsHasNull && (((pX)->flags & 0x001000) != 0)) {
    int i;
    ExprList *pEList = pX->x.pSelect->pEList;
    for (i = 0; i < pEList->nExpr; i++) {
      if (sqlite3ExprCanBeNull(pEList->a[i].pExpr))
        break;
    }
    if (i == pEList->nExpr) {
      prRhsHasNull = 0;
    }
  }

  if (pParse->nErr == 0 && (p = isCandidateForInOpt(pX)) != 0) {
    sqlite3 *db = pParse->db;
    Table *pTab;
    int iDb;
    ExprList *pEList = p->pEList;
    int nExpr = pEList->nExpr;

    pTab = p->pSrc->a[0].pSTab;

    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);

    sqlite3CodeVerifySchema(pParse, iDb);
    sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);

    if (nExpr == 1 && pEList->a[0].pExpr->iColumn < 0) {
      int iAddr = sqlite3VdbeAddOp0(v, 15);

      sqlite3OpenTable(pParse, iTab, iDb, pTab, 114);
      eType = 1;
      sqlite3VdbeExplain(pParse, 0, "USING ROWID SEARCH ON TABLE %s FOR IN-OPERATOR", pTab->zName);
      sqlite3VdbeJumpHere(v, iAddr);
    } else {
      Index *pIdx;
      int affinity_ok = 1;
      int i;

      for (i = 0; i < nExpr && affinity_ok; i++) {
        Expr *pLhs = sqlite3VectorFieldSubexpr(pX->pLeft, i);
        int iCol = pEList->a[i].pExpr->iColumn;
        char idxaff = sqlite3TableColumnAffinity(pTab, iCol);
        char cmpaff = sqlite3CompareAffinity(pLhs, idxaff);
        switch (cmpaff) {
          case 0x41:
            break;
          case 0x42:
            break;
          default:
            affinity_ok = ((idxaff) >= 0x43);
        }
      }

      if (affinity_ok) {
        for (pIdx = pTab->pIndex; pIdx && eType == 0; pIdx = pIdx->pNext) {
          Bitmask colUsed;
          Bitmask mCol;
          if (pIdx->nColumn < nExpr)
            continue;
          if (pIdx->pPartIdxWhere != 0)
            continue;

          if (pIdx->nColumn >= ((int)(sizeof(Bitmask) * 8)) - 1)
            continue;
          if (mustBeUnique) {
            if (pIdx->nKeyCol > nExpr || (pIdx->nColumn > nExpr && !((pIdx)->onError != 0))) {
              continue;
            }
          }

          colUsed = 0;
          for (i = 0; i < nExpr; i++) {
            Expr *pLhs = sqlite3VectorFieldSubexpr(pX->pLeft, i);
            Expr *pRhs = pEList->a[i].pExpr;
            CollSeq *pReq = sqlite3BinaryCompareCollSeq(pParse, pLhs, pRhs);
            int j;

            for (j = 0; j < nExpr; j++) {
              if (pIdx->aiColumn[j] != pRhs->iColumn)
                continue;

              if (pReq != 0 && sqlite3StrICmp(pReq->zName, pIdx->azColl[j]) != 0) {
                continue;
              }
              break;
            }
            if (j == nExpr)
              break;
            mCol = (((Bitmask)1) << (j));
            if (mCol & colUsed)
              break;
            colUsed |= mCol;
            if (aiMap)
              aiMap[i] = j;
          }

          if (colUsed == ((((Bitmask)1) << (nExpr)) - 1)) {
            int iAddr = sqlite3VdbeAddOp0(v, 15);
            sqlite3VdbeExplain(pParse, 0, "USING INDEX %s FOR IN-OPERATOR", pIdx->zName);
            sqlite3VdbeAddOp3(v, 114, iTab, pIdx->tnum, iDb);
            sqlite3VdbeSetP4KeyInfo(pParse, pIdx);

            eType = 3 + pIdx->aSortOrder[0];

            if (prRhsHasNull) {
              *prRhsHasNull = ++pParse->nMem;
              if (nExpr == 1) {
                sqlite3SetHasNullFlag(v, iTab, *prRhsHasNull);
              }
            }
            sqlite3VdbeJumpHere(v, iAddr);
          }
        }
      }
    }
  }

  if (eType == 0 && (inFlags & 0x0001) && (((pX)->flags & 0x001000) == 0) &&
      (!sqlite3InRhsIsConstant(pParse, pX) || pX->x.pList->nExpr <= 2)) {
    pParse->nTab--;
    iTab = -1;
    eType = 5;
  }

  if (eType == 0) {
    u32 savedNQueryLoop = pParse->nQueryLoop;
    int rMayHaveNull = 0;
    int bloomOk = (inFlags & 0x0002) != 0;
    eType = 2;
    if (inFlags & 0x0004) {
      pParse->nQueryLoop = 0;
    } else if (prRhsHasNull) {
      *prRhsHasNull = rMayHaveNull = ++pParse->nMem;
    }

    if (!bloomOk && (((pX)->flags & 0x001000) != 0) && (pX->x.pSelect->selFlags & 0x0000020) != 0) {
      bloomOk = 1;
    }
    sqlite3CodeRhsOfIN(pParse, pX, iTab, bloomOk);
    if (rMayHaveNull) {
      sqlite3SetHasNullFlag(v, iTab, rMayHaveNull);
    }
    pParse->nQueryLoop = savedNQueryLoop;
  }

  if (aiMap && eType != 3 && eType != 4) {
    int i, n;
    n = sqlite3ExprVectorSize(pX->pLeft);
    for (i = 0; i < n; i++)
      aiMap[i] = i;
  }
  *piTab = iTab;
  return eType;
}

char *exprINAffinity(Parse *pParse, const Expr *pExpr) {
  Expr *pLeft = pExpr->pLeft;
  int nVal = sqlite3ExprVectorSize(pLeft);
  Select *pSelect = (((pExpr)->flags & 0x001000) != 0) ? pExpr->x.pSelect : 0;
  char *zRet;

  zRet = (char*)(sqlite3DbMallocRaw(pParse->db, 1 + (i64)nVal));
  if (zRet) {
    int i;
    for (i = 0; i < nVal; i++) {
      Expr *pA = sqlite3VectorFieldSubexpr(pLeft, i);
      char a = sqlite3ExprAffinity(pA);
      if (pSelect) {
        zRet[i] = sqlite3CompareAffinity(pSelect->pEList->a[i].pExpr, a);
      } else {
        zRet[i] = a;
      }
    }
    zRet[nVal] = '\0';
  }
  return zRet;
}

void sqlite3SubselectError(Parse *pParse, int nActual, int nExpect) {
  if (pParse->nErr == 0) {
    const char *zFmt = "sub-select returns %d columns - expected %d";
    sqlite3ErrorMsg(pParse, zFmt, nActual, nExpect);
  }
}

void sqlite3VectorErrorMsg(Parse *pParse, Expr *pExpr) {
  if ((((pExpr)->flags & 0x001000) != 0)) {
    sqlite3SubselectError(pParse, pExpr->x.pSelect->pEList->nExpr, 1);
  } else {
    sqlite3ErrorMsg(pParse, "row value misused");
  }
}

int findCompatibleInRhsSubrtn(Parse *pParse, Expr *pExpr, SubrtnSig *pNewSig) {
  VdbeOp *pOp, *pEnd;
  SubrtnSig *pSig;
  Vdbe *v;

  if (pNewSig == 0)
    return 0;
  if ((pParse->mSubrtnSig & (1 << (pNewSig->selId & 7))) == 0)
    return 0;

  v = pParse->pVdbe;

  pOp = sqlite3VdbeGetOp(v, 1);
  pEnd = sqlite3VdbeGetLastOp(v);
  for (; pOp < pEnd; pOp++) {
    if (pOp->p4type != (-18))
      continue;

    pSig = pOp->p4.pSubrtnSig;

    if (!pSig->bComplete)
      continue;
    if (pNewSig->selId != pSig->selId)
      continue;
    if (strcmp(pNewSig->zAff, pSig->zAff) != 0)
      continue;
    pExpr->y.sub.iAddr = pSig->iAddr;
    pExpr->y.sub.regReturn = pSig->regReturn;
    pExpr->iTable = pSig->iTable;
    (pExpr)->flags |= (u32)(0x2000000);
    return 1;
  }
  return 0;
}

void sqlite3CodeRhsOfIN(Parse *pParse, Expr *pExpr, int iTab, int allowBloom) {
  int addrOnce = 0;
  int addr;
  Expr *pLeft;
  KeyInfo *pKeyInfo = 0;
  int nVal;
  Vdbe *v;
  SubrtnSig *pSig = 0;

  v = pParse->pVdbe;

  if (!(((pExpr)->flags & (u32)(0x000040)) != 0) && pParse->iSelfTab == 0) {
    if ((((pExpr)->flags & 0x001000) != 0) && (pExpr->x.pSelect->selFlags & 0x0000002) == 0) {
      pSig = (SubrtnSig*)(sqlite3DbMallocRawNN(pParse->db, sizeof(pSig[0])));
      if (pSig) {
        pSig->selId = pExpr->x.pSelect->selId;
        pSig->zAff = exprINAffinity(pParse, pExpr);
      }
    }

    if ((((pExpr)->flags & (u32)(0x2000000)) != 0) || findCompatibleInRhsSubrtn(pParse, pExpr, pSig)) {
      addrOnce = sqlite3VdbeAddOp0(v, 15);
      if ((((pExpr)->flags & 0x001000) != 0)) {
        sqlite3VdbeExplain(pParse, 0, "REUSE LIST SUBQUERY %d", pExpr->x.pSelect->selId);
      }

      sqlite3VdbeAddOp2(v, 10, pExpr->y.sub.regReturn, pExpr->y.sub.iAddr);

      sqlite3VdbeAddOp2(v, 117, iTab, pExpr->iTable);
      sqlite3VdbeJumpHere(v, addrOnce);
      if (pSig) {
        sqlite3DbFree(pParse->db, pSig->zAff);
        sqlite3DbFree(pParse->db, pSig);
      }
      return;
    }

    (pExpr)->flags |= (u32)(0x2000000);

    pExpr->y.sub.regReturn = ++pParse->nMem;
    pExpr->y.sub.iAddr = sqlite3VdbeAddOp2(v, 76, 0, pExpr->y.sub.regReturn) + 1;
    if (pSig) {
      pSig->bComplete = 0;
      pSig->iAddr = pExpr->y.sub.iAddr;
      pSig->regReturn = pExpr->y.sub.regReturn;
      pSig->iTable = iTab;
      pParse->mSubrtnSig = 1 << (pSig->selId & 7);
      sqlite3VdbeChangeP4(v, -1, (const char *)pSig, (-18));
    }
    addrOnce = sqlite3VdbeAddOp0(v, 15);
  }

  pLeft = pExpr->pLeft;
  nVal = sqlite3ExprVectorSize(pLeft);

  pExpr->iTable = iTab;
  addr = sqlite3VdbeAddOp2(v, 120, pExpr->iTable, nVal);

  pKeyInfo = sqlite3KeyInfoAlloc(pParse->db, nVal, 1);

  if ((((pExpr)->flags & 0x001000) != 0)) {
    Select *pSelect = pExpr->x.pSelect;
    ExprList *pEList = pSelect->pEList;

    sqlite3VdbeExplain(pParse, 1, "%sLIST SUBQUERY %d", addrOnce ? "" : "CORRELATED ", pSelect->selId);

    if (pEList->nExpr == nVal) {
      Select *pCopy;
      SelectDest dest;
      int i;
      int rc;
      int addrBloom = 0;
      sqlite3SelectDestInit(&dest, 9, iTab);
      dest.zAffSdst = exprINAffinity(pParse, pExpr);
      pSelect->iLimit = 0;
      if (addrOnce && allowBloom && (((pParse->db)->dbOptFlags & (0x00080000)) == 0)) {
        int regBloom = ++pParse->nMem;
        addrBloom = sqlite3VdbeAddOp2(v, 79, 10000, regBloom);
        dest.iSDParm2 = regBloom;
      };
      pCopy = sqlite3SelectDup(pParse->db, pSelect, 0);
      rc = pParse->db->mallocFailed ? 1 : sqlite3Select(pParse, pCopy, &dest);
      sqlite3SelectDelete(pParse->db, pCopy);
      sqlite3DbFree(pParse->db, dest.zAffSdst);
      if (addrBloom) {
        sqlite3VdbeGetOp(v, addrOnce)->p3 = dest.iSDParm2;
        if (dest.iSDParm2 == 0) {
          sqlite3VdbeGetOp(v, addrBloom)->p1 = 10;
        }
      }
      if (rc) {
        sqlite3KeyInfoUnref(pKeyInfo);
        return;
      }

      for (i = 0; i < nVal; i++) {
        Expr *p = sqlite3VectorFieldSubexpr(pLeft, i);
        pKeyInfo->aColl[i] = sqlite3BinaryCompareCollSeq(pParse, p, pEList->a[i].pExpr);
      }
    }
  } else if ((pExpr->x.pList != 0)) {
    char affinity;
    int i;
    ExprList *pList = pExpr->x.pList;
    struct ExprList_item *pItem;
    int r1, r2;
    affinity = sqlite3ExprAffinity(pLeft);
    if (affinity <= 0x40) {
      affinity = 0x41;
    } else if (affinity == 0x45) {
      affinity = 0x43;
    }
    if (pKeyInfo) {
      pKeyInfo->aColl[0] = sqlite3ExprCollSeq(pParse, pExpr->pLeft);
    }

    r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3GetTempReg(pParse);
    for (i = pList->nExpr, pItem = pList->a; i > 0; i--, pItem++) {
      Expr *pE2 = pItem->pExpr;

      if (addrOnce && !sqlite3ExprIsConstant(pParse, pE2)) {
        sqlite3VdbeChangeToNoop(v, addrOnce - 1);
        sqlite3VdbeChangeToNoop(v, addrOnce);
        (pExpr)->flags &= ~(u32)(0x2000000);
        addrOnce = 0;
      }

      sqlite3ExprCode(pParse, pE2, r1);
      sqlite3VdbeAddOp4(v, 99, r1, 1, r2, &affinity, 1);
      sqlite3VdbeAddOp4Int(v, 140, iTab, r2, r1, 1);
    }
    sqlite3ReleaseTempReg(pParse, r1);
    sqlite3ReleaseTempReg(pParse, r2);
  }
  if (pSig)
    pSig->bComplete = 1;
  if (pKeyInfo) {
    sqlite3VdbeChangeP4(v, addr, (const char*)((void *)pKeyInfo), (-9));
  }
  if (addrOnce) {
    sqlite3VdbeAddOp1(v, 138, iTab);
    sqlite3VdbeJumpHere(v, addrOnce);

    sqlite3VdbeAddOp3(v, 69, pExpr->y.sub.regReturn, pExpr->y.sub.iAddr, 1);
    sqlite3ClearTempRegCache(pParse);
  }
}

int sqlite3CodeSubselect(Parse *pParse, Expr *pExpr) {
  int addrOnce = 0;
  int rReg = 0;
  Select *pSel;
  SelectDest dest;
  int nReg;
  Expr *pLimit;

  Vdbe *v = pParse->pVdbe;

  if (pParse->nErr)
    return 0;

  pSel = pExpr->x.pSelect;

  if ((((pExpr)->flags & (u32)(0x2000000)) != 0)) {
    sqlite3VdbeExplain(pParse, 0, "REUSE SUBQUERY %d", pSel->selId);

    sqlite3VdbeAddOp2(v, 10, pExpr->y.sub.regReturn, pExpr->y.sub.iAddr);
    return pExpr->iTable;
  }

  (pExpr)->flags |= (u32)(0x2000000);
  pExpr->y.sub.regReturn = ++pParse->nMem;
  pExpr->y.sub.iAddr = sqlite3VdbeAddOp2(v, 76, 0, pExpr->y.sub.regReturn) + 1;

  if (!(((pExpr)->flags & (u32)(0x000040)) != 0)) {
    addrOnce = sqlite3VdbeAddOp0(v, 15);
  }

  sqlite3VdbeExplain(pParse, 1, "%sSCALAR SUBQUERY %d", addrOnce ? "" : "CORRELATED ", pSel->selId);
  nReg = pExpr->op == 139 ? pSel->pEList->nExpr : 1;
  sqlite3SelectDestInit(&dest, 0, pParse->nMem + 1);
  pParse->nMem += nReg;
  if (pExpr->op == 139) {
    dest.eDest = 8;
    if ((pSel->selFlags & 0x0000001) && pSel->pLimit && pSel->pLimit->pRight) {
      dest.iSdst = pParse->nMem + 1;
      pParse->nMem += nReg;
    } else {
      dest.iSdst = dest.iSDParm;
    }
    dest.nSdst = nReg;
    sqlite3VdbeAddOp3(v, 77, 0, dest.iSDParm, pParse->nMem);
  } else {
    dest.eDest = 1;
    sqlite3VdbeAddOp2(v, 73, 0, dest.iSDParm);
  }
  if (pSel->pLimit) {
    Expr *pLeft = pSel->pLimit->pLeft;
    if ((((pLeft)->flags & (u32)(0x000800)) != 0) == 0 || (pLeft->u.iValue != 1 && pLeft->u.iValue != 0)) {
      sqlite3 *db = pParse->db;
      pLimit = sqlite3ExprInt32(db, 0);
      if (pLimit) {
        pLimit->affExpr = 0x43;
        pLimit = sqlite3PExpr(pParse, 53, sqlite3ExprDup(db, pLeft, 0), pLimit);
      }
      sqlite3ExprDeferredDelete(pParse, pLeft);
      pSel->pLimit->pLeft = pLimit;
    }
  } else {
    pLimit = sqlite3ExprInt32(pParse->db, 1);
    pSel->pLimit = sqlite3PExpr(pParse, 149, pLimit, 0);
  }
  pSel->iLimit = 0;
  if (sqlite3Select(pParse, pSel, &dest)) {
    pExpr->op2 = pExpr->op;
    pExpr->op = 182;
    return 0;
  }
  pExpr->iTable = rReg = dest.iSDParm;
  if (addrOnce) {
    sqlite3VdbeJumpHere(v, addrOnce);
  };

  sqlite3VdbeAddOp3(v, 69, pExpr->y.sub.regReturn, pExpr->y.sub.iAddr, 1);
  sqlite3ClearTempRegCache(pParse);
  return rReg;
}

int sqlite3ExprCheckIN(Parse *pParse, Expr *pIn) {
  int nVector = sqlite3ExprVectorSize(pIn->pLeft);
  if ((((pIn)->flags & 0x001000) != 0) && !pParse->db->mallocFailed) {
    if (nVector != pIn->x.pSelect->pEList->nExpr) {
      sqlite3SubselectError(pParse, pIn->x.pSelect->pEList->nExpr, nVector);
      return 1;
    }
  } else if (nVector != 1) {
    sqlite3VectorErrorMsg(pParse, pIn->pLeft);
    return 1;
  }
  return 0;
}

void sqlite3ExprCodeIN(Parse *pParse, Expr *pExpr, int destIfFalse, int destIfNull) {
  int rRhsHasNull = 0;
  int eType;
  int rLhs;
  Vdbe *v;
  int *aiMap = 0;
  char *zAff = 0;
  int nVector;
  int iDummy;
  Expr *pLeft;
  int i;
  int destStep2;
  int destStep6 = 0;
  int addrTruthOp;
  int destNotNull;
  int addrTop;
  int iTab = 0;
  u8 okConstFactor = pParse->okConstFactor;

  pLeft = pExpr->pLeft;
  if (sqlite3ExprCheckIN(pParse, pExpr))
    return;
  zAff = exprINAffinity(pParse, pExpr);
  nVector = sqlite3ExprVectorSize(pExpr->pLeft);
  aiMap = (int *)sqlite3DbMallocZero(pParse->db, nVector * sizeof(int));
  if (pParse->db->mallocFailed)
    goto sqlite3ExprCodeIN_oom_error;

  v = pParse->pVdbe;

  eType =
      sqlite3FindInIndex(pParse, pExpr, 0x0002 | 0x0001, destIfFalse == destIfNull ? 0 : &rRhsHasNull, aiMap, &iTab);

  pParse->okConstFactor = 0;
  rLhs = exprCodeVector(pParse, pLeft, &iDummy);
  pParse->okConstFactor = okConstFactor;

  if (eType == 5) {
    ExprList *pList;
    CollSeq *pColl;
    int labelOk = sqlite3VdbeMakeLabel(pParse);
    int r2, regToFree;
    int regCkNull = 0;
    int ii;

    pList = pExpr->x.pList;
    pColl = sqlite3ExprCollSeq(pParse, pExpr->pLeft);
    if (destIfNull != destIfFalse) {
      regCkNull = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp3(v, 103, rLhs, rLhs, regCkNull);
    }
    for (ii = 0; ii < pList->nExpr; ii++) {
      r2 = sqlite3ExprCodeTemp(pParse, pList->a[ii].pExpr, &regToFree);
      if (regCkNull && sqlite3ExprCanBeNull(pList->a[ii].pExpr)) {
        sqlite3VdbeAddOp3(v, 103, regCkNull, r2, regCkNull);
      }
      sqlite3ReleaseTempReg(pParse, regToFree);
      if (ii < pList->nExpr - 1 || destIfNull != destIfFalse) {
        int op = rLhs != r2 ? 54 : 52;
        sqlite3VdbeAddOp4(v, op, rLhs, labelOk, r2, (const char*)((void *)pColl), (-2));
        sqlite3VdbeChangeP5(v, zAff[0]);
      } else {
        int op = rLhs != r2 ? 53 : 51;

        sqlite3VdbeAddOp4(v, op, rLhs, destIfFalse, r2, (const char*)((void *)pColl), (-2));
        sqlite3VdbeChangeP5(v, zAff[0] | 0x10);
      }
    }
    if (regCkNull) {
      sqlite3VdbeAddOp2(v, 51, regCkNull, destIfNull);
      sqlite3VdbeGoto(v, destIfFalse);
    }
    sqlite3VdbeResolveLabel(v, labelOk);
    sqlite3ReleaseTempReg(pParse, regCkNull);
    goto sqlite3ExprCodeIN_finished;
  }

  if (eType != 1) {
    sqlite3VdbeAddOp4(v, 98, rLhs, nVector, 0, zAff, nVector);
    for (i = 0; i < nVector && aiMap[i] == i; i++) {
    }
    if (i != nVector) {
      int rLhsOrig = rLhs;
      rLhs = sqlite3GetTempRange(pParse, nVector);
      for (i = 0; i < nVector; i++) {
        sqlite3VdbeAddOp3(v, 82, rLhsOrig + i, rLhs + aiMap[i], 0);
      }
      sqlite3ReleaseTempReg(pParse, rLhsOrig);
    }
  }

  if (destIfNull == destIfFalse) {
    destStep2 = destIfFalse;
  } else {
    destStep2 = destStep6 = sqlite3VdbeMakeLabel(pParse);
  }
  for (i = 0; i < nVector; i++) {
    Expr *p = sqlite3VectorFieldSubexpr(pExpr->pLeft, i);
    if (pParse->nErr)
      goto sqlite3ExprCodeIN_oom_error;
    if (sqlite3ExprCanBeNull(p)) {
      sqlite3VdbeAddOp2(v, 51, rLhs + aiMap[i], destStep2);
    }
  }

  if (eType == 1) {
    sqlite3VdbeAddOp3(v, 30, iTab, destIfFalse, rLhs);
    addrTruthOp = sqlite3VdbeAddOp0(v, 9);
  } else {
    if (destIfFalse == destIfNull) {
      if ((((pExpr)->flags & (u32)(0x2000000)) != 0)) {
        const VdbeOp *pOp = sqlite3VdbeGetOp(v, pExpr->y.sub.iAddr);

        if (pOp->p3 > 0) {
          sqlite3VdbeAddOp4Int(v, 66, pOp->p3, destIfFalse, rLhs, nVector);
        }
      }
      sqlite3VdbeAddOp4Int(v, 28, iTab, destIfFalse, rLhs, nVector);
      goto sqlite3ExprCodeIN_finished;
    }

    addrTruthOp = sqlite3VdbeAddOp4Int(v, 29, iTab, 0, rLhs, nVector);
  }

  if (rRhsHasNull && nVector == 1) {
    sqlite3VdbeAddOp2(v, 52, rRhsHasNull, destIfFalse);
  }

  if (destIfFalse == destIfNull)
    sqlite3VdbeGoto(v, destIfFalse);

  if (destStep6)
    sqlite3VdbeResolveLabel(v, destStep6);
  addrTop = sqlite3VdbeAddOp2(v, 36, iTab, destIfFalse);
  if (nVector > 1) {
    destNotNull = sqlite3VdbeMakeLabel(pParse);
  } else {
    destNotNull = destIfFalse;
  }
  for (i = 0; i < nVector; i++) {
    Expr *p;
    CollSeq *pColl;
    int r3 = sqlite3GetTempReg(pParse);
    p = sqlite3VectorFieldSubexpr(pLeft, i);
    if ((((pExpr)->flags & 0x001000) != 0)) {
      Expr *pRhs = pExpr->x.pSelect->pEList->a[i].pExpr;
      pColl = sqlite3BinaryCompareCollSeq(pParse, p, pRhs);
    } else {
      pColl = sqlite3ExprCollSeq(pParse, p);
    }
    sqlite3VdbeAddOp3(v, 96, iTab, aiMap[i], r3);
    sqlite3VdbeAddOp4(v, 53, rLhs + aiMap[i], destNotNull, r3, (const char*)((void *)pColl), (-2));
    sqlite3ReleaseTempReg(pParse, r3);
  }
  sqlite3VdbeAddOp2(v, 9, 0, destIfNull);
  if (nVector > 1) {
    sqlite3VdbeResolveLabel(v, destNotNull);
    sqlite3VdbeAddOp2(v, 40, iTab, addrTop + 1);

    sqlite3VdbeAddOp2(v, 9, 0, destIfFalse);
  }

  sqlite3VdbeJumpHere(v, addrTruthOp);

sqlite3ExprCodeIN_finished:;
sqlite3ExprCodeIN_oom_error:
  sqlite3DbFree(pParse->db, aiMap);
  sqlite3DbFree(pParse->db, zAff);
}

void codeInteger(Parse *pParse, Expr *pExpr, int negFlag, int iMem) {
  Vdbe *v = pParse->pVdbe;
  if (pExpr->flags & 0x000800) {
    int i = pExpr->u.iValue;

    if (negFlag)
      i = -i;
    sqlite3VdbeAddOp2(v, 73, i, iMem);
  } else {
    int c;
    i64 value;
    const char *z = pExpr->u.zToken;

    c = sqlite3DecOrHexToI64(z, &value);
    if ((c == 3 && !negFlag) || (c == 2) ||
        (negFlag && value == (((i64)-1) - (0xffffffff | (((i64)0x7fffffff) << 32))))) {
      if (sqlite3_strnicmp(z, "0x", 2) == 0) {
        sqlite3ErrorMsg(pParse, "hex literal too big: %s%#T", negFlag ? "-" : "", pExpr);
      } else {
        codeReal(v, z, negFlag, iMem);
      }

    } else {
      if (negFlag) {
        value = c == 3 ? (((i64)-1) - (0xffffffff | (((i64)0x7fffffff) << 32))) : -value;
      }
      sqlite3VdbeAddOp4Dup8(v, 74, 0, iMem, 0, (u8 *)&value, (-14));
    }
  }
}

void sqlite3ExprCodeLoadIndexColumn(Parse *pParse, Index *pIdx, int iTabCur, int iIdxCol, int regOut) {
  i16 iTabCol = pIdx->aiColumn[iIdxCol];
  if (iTabCol == (-2)) {
    pParse->iSelfTab = iTabCur + 1;
    sqlite3ExprCodeCopy(pParse, pIdx->aColExpr->a[iIdxCol].pExpr, regOut);
    pParse->iSelfTab = 0;
  } else {
    sqlite3ExprCodeGetColumnOfTable(pParse->pVdbe, pIdx->pTable, iTabCur, iTabCol, regOut);
  }
}

void sqlite3ExprCodeGeneratedColumn(Parse *pParse, Table *pTab, Column *pCol, int regOut) {
  int iAddr;
  Vdbe *v = pParse->pVdbe;
  int nErr = pParse->nErr;

  if (pParse->iSelfTab > 0) {
    iAddr = sqlite3VdbeAddOp3(v, 20, pParse->iSelfTab - 1, 0, regOut);
  } else {
    iAddr = 0;
  }
  sqlite3ExprCodeCopy(pParse, sqlite3ColumnExpr(pTab, pCol), regOut);
  if ((pCol->colFlags & 0x0020) != 0 && (pTab->tabFlags & 0x00010000) != 0) {
    int p3 = 2 + (int)(pCol - pTab->aCol);
    sqlite3VdbeAddOp4(v, 97, regOut, 1, p3, (char *)pTab, (-5));
  } else if (pCol->affinity >= 0x42) {
    sqlite3VdbeAddOp4(v, 98, regOut, 1, 0, &pCol->affinity, 1);
  }
  if (iAddr)
    sqlite3VdbeJumpHere(v, iAddr);
  if (pParse->nErr > nErr)
    pParse->db->errByteOffset = -1;
}

int sqlite3ExprCodeGetColumn(Parse *pParse, Table *pTab, int iColumn, int iTable, int iReg, u8 p5) {
  sqlite3ExprCodeGetColumnOfTable(pParse->pVdbe, pTab, iTable, iColumn, iReg);
  if (p5) {
    VdbeOp *pOp = sqlite3VdbeGetLastOp(pParse->pVdbe);
    if (pOp->opcode == 96)
      pOp->p5 = p5;
    if (pOp->opcode == 178)
      pOp->p5 = (p5 & 0x01);
  }
  return iReg;
}

void sqlite3ExprCodeMove(Parse *pParse, int iFrom, int iTo, int nReg) {
  sqlite3VdbeAddOp3(pParse->pVdbe, 81, iFrom, iTo, nReg);
}

int exprCodeVector(Parse *pParse, Expr *p, int *piFreeable) {
  int iResult;
  int nResult = sqlite3ExprVectorSize(p);
  if (nResult == 1) {
    iResult = sqlite3ExprCodeTemp(pParse, p, piFreeable);
  } else {
    *piFreeable = 0;
    if (p->op == 139) {
      iResult = sqlite3CodeSubselect(pParse, p);

    } else {
      int i;
      iResult = pParse->nMem + 1;
      pParse->nMem += nResult;

      for (i = 0; i < nResult; i++) {
        sqlite3ExprCodeFactorable(pParse, p->x.pList->a[i].pExpr, i + iResult);
      }
    }
  }
  return iResult;
}

int exprCodeInlineFunction(Parse *pParse, ExprList *pFarg, int iFuncId, int target) {
  int nFarg;
  Vdbe *v = pParse->pVdbe;

  nFarg = pFarg->nExpr;

  switch (iFuncId) {
    case 0: {
      int endCoalesce = sqlite3VdbeMakeLabel(pParse);
      int i;

      sqlite3ExprCode(pParse, pFarg->a[0].pExpr, target);
      for (i = 1; i < nFarg; i++) {
        sqlite3VdbeAddOp2(v, 52, target, endCoalesce);
        sqlite3ExprCode(pParse, pFarg->a[i].pExpr, target);
      }
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endCoalesce);
      break;
    }
    case 5: {
      Expr caseExpr;
      memset(&caseExpr, 0, sizeof(caseExpr));
      caseExpr.op = 158;
      caseExpr.x.pList = pFarg;
      return sqlite3ExprCodeTarget(pParse, &caseExpr, target);
    }

    default: {
      target = sqlite3ExprCodeTarget(pParse, pFarg->a[0].pExpr, target);
      break;
    }

    case 3: {
      sqlite3VdbeAddOp2(v, 73, sqlite3ExprCompare(0, pFarg->a[0].pExpr, pFarg->a[1].pExpr, -1), target);
      break;
    }

    case 2: {
      sqlite3VdbeAddOp2(v, 73, sqlite3ExprImpliesExpr(pParse, pFarg->a[0].pExpr, pFarg->a[1].pExpr, -1), target);
      break;
    }

    case 1: {
      Expr *pA1;

      pA1 = pFarg->a[1].pExpr;
      if (pA1->op == 168) {
        sqlite3VdbeAddOp2(v, 73, sqlite3ExprImpliesNonNullRow(pFarg->a[0].pExpr, pA1->iTable, 1), target);
      } else {
        sqlite3VdbeAddOp2(v, 77, 0, target);
      }
      break;
    }

    case 4: {
      const char *azAff[] = {"blob", "text", "numeric", "integer", "real", "flexnum"};
      char aff;

      aff = sqlite3ExprAffinity(pFarg->a[0].pExpr);

      sqlite3VdbeLoadString(v, target, (aff <= 0x40) ? "none" : azAff[aff - 0x41]);
      break;
    }
  }
  return target;
}

int sqlite3ExprCanReturnSubtype(Parse *pParse, Expr *pExpr) {
  Walker w;
  memset(&w, 0, sizeof(w));
  w.pParse = pParse;
  w.xExprCallback = exprNodeCanReturnSubtype;
  sqlite3WalkExpr(&w, pExpr);
  return w.eCode;
}

__attribute__((noinline)) int sqlite3IndexedExprLookup(Parse *pParse, Expr *pExpr, int target) {
  IndexedExpr *p;
  Vdbe *v;
  for (p = pParse->pIdxEpr; p; p = p->pIENext) {
    u8 exprAff;
    int iDataCur = p->iDataCur;
    if (iDataCur < 0)
      continue;
    if (pParse->iSelfTab) {
      if (p->iDataCur != pParse->iSelfTab - 1)
        continue;
      iDataCur = -1;
    }
    if (sqlite3ExprCompare(0, pExpr, p->pExpr, iDataCur) != 0)
      continue;

    exprAff = sqlite3ExprAffinity(pExpr);
    if ((exprAff <= 0x41 && p->aff != 0x41) || (exprAff == 0x42 && p->aff != 0x42) ||
        (exprAff >= 0x43 && p->aff != 0x43)) {
      continue;
    }

    if ((((pExpr)->flags & (u32)(0x80000000)) != 0) && sqlite3ExprCanReturnSubtype(pParse, pExpr)) {
      continue;
    }

    v = pParse->pVdbe;

    if (p->bMaybeNullRow) {
      int addr = sqlite3VdbeCurrentAddr(v);
      sqlite3VdbeAddOp3(v, 20, p->iIdxCur, addr + 3, target);
      sqlite3VdbeAddOp3(v, 96, p->iIdxCur, p->iIdxCol, target);
      sqlite3VdbeGoto(v, 0);
      p = pParse->pIdxEpr;
      pParse->pIdxEpr = 0;
      sqlite3ExprCode(pParse, pExpr, target);
      pParse->pIdxEpr = p;
      sqlite3VdbeJumpHere(v, addr + 2);
    } else {
      sqlite3VdbeAddOp3(v, 96, p->iIdxCur, p->iIdxCol, target);
    }
    return target;
  }
  return -1;
}

int exprPartidxExprLookup(Parse *pParse, Expr *pExpr, int iTarget) {
  IndexedExpr *p;
  for (p = pParse->pIdxPartExpr; p; p = p->pIENext) {
    if (pExpr->iColumn == p->iIdxCol && pExpr->iTable == p->iDataCur) {
      Vdbe *v = pParse->pVdbe;
      int addr = 0;
      int ret;

      if (p->bMaybeNullRow) {
        addr = sqlite3VdbeAddOp1(v, 20, p->iIdxCur);
      }
      ret = sqlite3ExprCodeTarget(pParse, p->pExpr, iTarget);
      sqlite3VdbeAddOp4(pParse->pVdbe, 98, ret, 1, 0, (const char *)&p->aff, 1);
      if (addr) {
        sqlite3VdbeJumpHere(v, addr);
        sqlite3VdbeChangeP3(v, addr, ret);
      }
      return ret;
    }
  }
  return 0;
}

__attribute__((noinline)) int exprCodeTargetAndOr(Parse *pParse, Expr *pExpr, int target, int *pTmpReg) {
  int op;
  int skipOp;
  int addrSkip;
  int regSS = 0;
  int r1, r2;
  Expr *pAlt;
  Vdbe *v;

  op = pExpr->op;

  v = pParse->pVdbe;
  pAlt = sqlite3ExprSimplifiedAndOr(pExpr);
  if (pAlt != pExpr) {
    r1 = sqlite3ExprCodeTarget(pParse, pAlt, target);
    sqlite3VdbeAddOp3(v, 44, r1, r1, target);
    return target;
  }
  skipOp = op == 44 ? 17 : 16;
  if (exprEvalRhsFirst(pExpr)) {
    r2 = regSS = sqlite3ExprCodeTarget(pParse, pExpr->pRight, target);
    addrSkip = sqlite3VdbeAddOp1(v, skipOp, r2);
    r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, pTmpReg);
  } else {
    r1 = sqlite3ExprCodeTarget(pParse, pExpr->pLeft, target);
    if ((((pExpr->pRight)->flags & (u32)(0x400000)) != 0)) {
      regSS = r1;
      addrSkip = sqlite3VdbeAddOp1(v, skipOp, r1);
    } else {
      addrSkip = regSS = 0;
    }
    r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, pTmpReg);
  }
  sqlite3VdbeAddOp3(v, op, r2, r1, target);
  if (addrSkip) {
    sqlite3VdbeAddOp2(v, 9, 0, sqlite3VdbeCurrentAddr(v) + 2);
    sqlite3VdbeJumpHere(v, addrSkip);
    sqlite3VdbeAddOp3(v, 43, regSS, regSS, target);
  }
  return target;
}

int sqlite3ExprCodeTarget(Parse *pParse, Expr *pExpr, int target) {
  Vdbe *v = pParse->pVdbe;
  int op;
  int inReg = target;
  int regFree1 = 0;
  int regFree2 = 0;
  int r1, r2;
  Expr tempX;
  int p5 = 0;

expr_code_doover:
  if (pExpr == 0) {
    op = 122;
  } else if (pParse->pIdxEpr != 0 && !(((pExpr)->flags & (u32)(0x800000)) != 0) &&
             (r1 = sqlite3IndexedExprLookup(pParse, pExpr, target)) >= 0) {
    return r1;
  } else {
    op = pExpr->op;
  }

  switch (op) {
    case 170: {
      AggInfo *pAggInfo = pExpr->pAggInfo;
      struct AggInfo_col *pCol;

      if (pExpr->iAgg >= pAggInfo->nColumn) {
        sqlite3VdbeAddOp2(v, 77, 0, target);

        break;
      }
      pCol = &pAggInfo->aCol[pExpr->iAgg];
      if (!pAggInfo->directMode) {
        return ((pAggInfo)->iFirstReg + (pExpr->iAgg));
      } else if (pAggInfo->useSortingIdx) {
        Table *pTab = pCol->pTab;
        sqlite3VdbeAddOp3(v, 96, pAggInfo->sortingIdxPTab, pCol->iSorterColumn, target);
        if (pTab == 0) {
        } else if (pCol->iColumn < 0) {
        } else {
          if (pTab->aCol[pCol->iColumn].affinity == 0x45) {
            sqlite3VdbeAddOp1(v, 89, target);
          }
        }
        return target;
      } else if (pExpr->y.pTab == 0) {
        sqlite3VdbeAddOp3(v, 96, pExpr->iTable, pExpr->iColumn, target);
        return target;
      }

      __attribute__((fallthrough));
    }
    case 168: {
      int iTab = pExpr->iTable;
      int iReg;
      if ((((pExpr)->flags & (u32)(0x000020)) != 0)) {
        int aff;
        iReg = sqlite3ExprCodeTarget(pParse, pExpr->pLeft, target);

        aff = sqlite3TableColumnAffinity(pExpr->y.pTab, pExpr->iColumn);
        if (aff > 0x41) {
          static const char zAff[] = "B\000C\000D\000E\000F";

          sqlite3VdbeAddOp4(v, 98, iReg, 1, 0, &zAff[(aff - 'B') * 2], (-1));
        }
        return iReg;
      }
      if (iTab < 0) {
        if (pParse->iSelfTab < 0) {
          Column *pCol;
          Table *pTab;
          int iSrc;
          int iCol = pExpr->iColumn;

          pTab = pExpr->y.pTab;

          if (iCol < 0) {
            return -1 - pParse->iSelfTab;
          }
          pCol = pTab->aCol + iCol;
          iSrc = sqlite3TableColumnToStorage(pTab, iCol) - pParse->iSelfTab;

          if (pCol->colFlags & 0x0060) {
            if (pCol->colFlags & 0x0100) {
              sqlite3ErrorMsg(pParse, "generated column loop on \"%s\"", pCol->zCnName);
              return 0;
            }
            pCol->colFlags |= 0x0100;
            if (pCol->colFlags & 0x0080) {
              sqlite3ExprCodeGeneratedColumn(pParse, pTab, pCol, iSrc);
            }
            pCol->colFlags &= ~(0x0100 | 0x0080);
            return iSrc;
          } else if (pCol->affinity == 0x45) {
            sqlite3VdbeAddOp2(v, 83, iSrc, target);
            sqlite3VdbeAddOp1(v, 89, target);
            return target;
          } else {
            return iSrc;
          }
        } else {
          iTab = pParse->iSelfTab - 1;
        }
      } else if (pParse->pIdxPartExpr && 0 != (r1 = exprPartidxExprLookup(pParse, pExpr, target))) {
        return r1;
      }

      iReg = sqlite3ExprCodeGetColumn(pParse, pExpr->y.pTab, pExpr->iColumn, iTab, target, pExpr->op2);
      return iReg;
    }
    case 156: {
      codeInteger(pParse, pExpr, 0, target);
      return target;
    }
    case 171: {
      sqlite3VdbeAddOp2(v, 73, sqlite3ExprTruthValue(pExpr), target);
      return target;
    }

    case 154: {
      codeReal(v, pExpr->u.zToken, 0, target);
      return target;
    }

    case 118: {
      sqlite3VdbeLoadString(v, target, pExpr->u.zToken);
      return target;
    }
    case 83: {
      sqlite3VdbeAddOp3(v, 77, 0, target, target + pExpr->y.nReg - 1);
      return target;
    }
    default: {
      sqlite3VdbeAddOp2(v, 77, 0, target);
      return target;
    }

    case 155: {
      int n;
      const char *z;
      char *zBlob;

      z = &pExpr->u.zToken[2];
      n = sqlite3Strlen30(z) - 1;

      zBlob = (char*)(sqlite3HexToBlob(sqlite3VdbeDb(v), z, n));
      sqlite3VdbeAddOp4(v, 79, n / 2, target, 0, zBlob, (-7));
      return target;
    }

    case 157: {
      sqlite3VdbeAddOp2(v, 80, pExpr->iColumn, target);
      return target;
    }
    case 176: {
      return pExpr->iTable;
    }

    case 36: {
      sqlite3ExprCode(pParse, pExpr->pLeft, target);

      sqlite3VdbeAddOp2(v, 90, target, sqlite3AffinityType(pExpr->u.zToken, 0));
      return inReg;
    }

    case 45:
    case 46:
      op = (op == 45) ? 54 : 53;
      p5 = 0x80;
      __attribute__((fallthrough));
    case 57:
    case 56:
    case 55:
    case 58:
    case 53:
    case 54: {
      Expr *pLeft = pExpr->pLeft;
      int addrIsNull = 0;
      if (sqlite3ExprIsVector(pLeft)) {
        codeVectorCompare(pParse, pExpr, target, op, p5);
      } else {
        if ((((pExpr)->flags & (u32)(0x400000)) != 0) && p5 != 0x80) {
          addrIsNull = exprComputeOperands(pParse, pExpr, &r1, &r2, &regFree1, &regFree2);
        } else {
          r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
          r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, &regFree2);
        }
        sqlite3VdbeAddOp2(v, 73, 1, inReg);
        codeCompare(pParse, pLeft, pExpr->pRight, op, r1, r2, sqlite3VdbeCurrentAddr(v) + 2, p5,
                    (((pExpr)->flags & (u32)(0x000400)) != 0));

        if (p5 == 0x80) {
          sqlite3VdbeAddOp2(v, 73, 0, inReg);
        } else {
          sqlite3VdbeAddOp3(v, 94, r1, inReg, r2);
          if (addrIsNull) {
            sqlite3VdbeAddOp2(v, 9, 0, sqlite3VdbeCurrentAddr(v) + 2);
            sqlite3VdbeJumpHere(v, addrIsNull);
            sqlite3VdbeAddOp2(v, 77, 0, inReg);
          }
        };
      }
      break;
    }
    case 44:
    case 43: {
      inReg = exprCodeTargetAndOr(pParse, pExpr, target, &regFree1);
      break;
    }
    case 107:
    case 109:
    case 108:
    case 111:
    case 103:
    case 104:
    case 110:
    case 105:
    case 106:
    case 112: {
      int addrIsNull;

      if ((((pExpr)->flags & (u32)(0x400000)) != 0)) {
        addrIsNull = exprComputeOperands(pParse, pExpr, &r1, &r2, &regFree1, &regFree2);
      } else {
        r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
        r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, &regFree2);
        addrIsNull = 0;
      }
      sqlite3VdbeAddOp3(v, op, r2, r1, target);
      if (addrIsNull) {
        sqlite3VdbeAddOp2(v, 9, 0, sqlite3VdbeCurrentAddr(v) + 2);
        sqlite3VdbeJumpHere(v, addrIsNull);
        sqlite3VdbeAddOp2(v, 77, 0, target);
      }
      break;
    }
    case 174: {
      Expr *pLeft = pExpr->pLeft;

      if (pLeft->op == 156) {
        codeInteger(pParse, pLeft, 1, target);
        return target;

      } else if (pLeft->op == 154) {
        codeReal(v, pLeft->u.zToken, 1, target);
        return target;

      } else {
        tempX.op = 156;
        tempX.flags = 0x000800 | 0x010000;
        tempX.u.iValue = 0;
        r1 = sqlite3ExprCodeTemp(pParse, &tempX, &regFree1);
        r2 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree2);
        sqlite3VdbeAddOp3(v, 108, r2, r1, target);
      }
      break;
    }
    case 115:
    case 19: {
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      sqlite3VdbeAddOp2(v, op, r1, inReg);
      break;
    }
    case 175: {
      int isTrue;
      int bNormal;
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      isTrue = sqlite3ExprTruthValue(pExpr->pRight);
      bNormal = pExpr->op2 == 45;
      sqlite3VdbeAddOp4Int(v, 93, r1, inReg, !isTrue, isTrue ^ bNormal);
      break;
    }
    case 51:
    case 52: {
      int addr;

      sqlite3VdbeAddOp2(v, 73, 1, target);
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
      addr = sqlite3VdbeAddOp1(v, op, r1);
      sqlite3VdbeAddOp2(v, 73, 0, target);
      sqlite3VdbeJumpHere(v, addr);
      break;
    }
    case 169: {
      AggInfo *pInfo = pExpr->pAggInfo;
      if (pInfo == 0 || (pExpr->iAgg < 0) || (pExpr->iAgg >= pInfo->nFunc)) {
        sqlite3ErrorMsg(pParse, "misuse of aggregate: %#T()", pExpr);
      } else {
        return ((pInfo)->iFirstReg + (pInfo)->nColumn + (pExpr->iAgg));
      }
      break;
    }
    case 172: {
      ExprList *pFarg;
      int nFarg;
      FuncDef *pDef;
      const char *zId;
      u32 constMask = 0;
      int i;
      sqlite3 *db = pParse->db;
      u8 enc = ((db)->enc);
      CollSeq *pColl = 0;

      if ((((pExpr)->flags & (u32)(0x1000000)) != 0)) {
        return pExpr->y.pWin->regResult;
      }

      if (((pParse)->okConstFactor) && sqlite3ExprIsConstantNotJoin(pParse, pExpr)) {
        return sqlite3ExprCodeRunJustOnce(pParse, pExpr, -1);
      }

      pFarg = pExpr->x.pList;
      nFarg = pFarg ? pFarg->nExpr : 0;

      zId = pExpr->u.zToken;
      pDef = sqlite3FindFunction(db, zId, nFarg, enc, 0);

      if (pDef == 0 || pDef->xFinalize != 0) {
        sqlite3ErrorMsg(pParse, "unknown function: %#T()", pExpr);
        break;
      }
      if ((pDef->funcFlags & 0x00400000) != 0 && (pFarg != 0)) {
        return exprCodeInlineFunction(pParse, pFarg, ((int)(intptr_t)(pDef->pUserData)), target);
      } else if (pDef->funcFlags & (0x00080000 | 0x00200000)) {
        sqlite3ExprFunctionUsable(pParse, pExpr, pDef);
      }

      for (i = 0; i < nFarg; i++) {
        if (i < 32 && sqlite3ExprIsConstant(pParse, pFarg->a[i].pExpr)) {
          constMask |= (((unsigned int)1) << (i));
        }
        if ((pDef->funcFlags & 0x0020) != 0 && !pColl) {
          pColl = sqlite3ExprCollSeq(pParse, pFarg->a[i].pExpr);
        }
      }
      if (pFarg) {
        if (constMask) {
          r1 = pParse->nMem + 1;
          pParse->nMem += nFarg;
        } else {
          r1 = sqlite3GetTempRange(pParse, nFarg);
        }

        if ((pDef->funcFlags & (0x0040 | 0x0080)) != 0) {
          u8 exprOp;

          exprOp = pFarg->a[0].pExpr->op;
          if (exprOp == 168 || exprOp == 170) {
            pFarg->a[0].pExpr->op2 = pDef->funcFlags & 0xc0;
          }
        }

        sqlite3ExprCodeExprList(pParse, pFarg, r1, 0, 0x02);
      } else {
        r1 = 0;
      }

      if (nFarg >= 2 && (((pExpr)->flags & (u32)(0x000100)) != 0)) {
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[1].pExpr);
      } else if (nFarg > 0) {
        pDef = sqlite3VtabOverloadFunction(db, pDef, nFarg, pFarg->a[0].pExpr);
      }

      if (pDef->funcFlags & 0x0020) {
        if (!pColl)
          pColl = db->pDfltColl;
        sqlite3VdbeAddOp4(v, 87, 0, 0, 0, (char *)pColl, (-2));
      }
      sqlite3VdbeAddFunctionCall(pParse, constMask, r1, target, nFarg, pDef, pExpr->op2);
      if (nFarg) {
        if (constMask == 0) {
          sqlite3ReleaseTempRange(pParse, r1, nFarg);
        } else {
        }
      }
      return target;
    }

    case 20:
    case 139: {
      int nCol;
      if (pParse->db->mallocFailed) {
        return 0;
      } else if (op == 139 && ((((pExpr)->flags & 0x001000) != 0)) && (nCol = pExpr->x.pSelect->pEList->nExpr) != 1) {
        sqlite3SubselectError(pParse, nCol, 1);
      } else {
        return sqlite3CodeSubselect(pParse, pExpr);
      }
      break;
    }
    case 178: {
      int n;
      Expr *pLeft = pExpr->pLeft;
      if (pLeft->iTable == 0 || pParse->withinRJSubrtn > pLeft->op2) {
        pLeft->iTable = sqlite3CodeSubselect(pParse, pLeft);
        pLeft->op2 = pParse->withinRJSubrtn;
      }

      n = sqlite3ExprVectorSize(pLeft);
      if (pExpr->iTable != n) {
        sqlite3ErrorMsg(pParse, "%d columns assigned %d values", pExpr->iTable, n);
      }
      return pLeft->iTable + pExpr->iColumn;
    }
    case 50: {
      int destIfFalse = sqlite3VdbeMakeLabel(pParse);
      int destIfNull = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeAddOp2(v, 77, 0, target);
      sqlite3ExprCodeIN(pParse, pExpr, destIfFalse, destIfNull);
      sqlite3VdbeAddOp2(v, 73, 1, target);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      sqlite3VdbeAddOp2(v, 88, target, 0);
      sqlite3VdbeResolveLabel(v, destIfNull);
      return target;
    }

    case 49: {
      exprCodeBetween(pParse, pExpr, target, 0, 0);
      return target;
    }
    case 114: {
      if (!(((pExpr)->flags & (u32)(0x000200)) != 0)) {
        sqlite3ExprCode(pParse, pExpr->pLeft, target);
        sqlite3VdbeAddOp1(v, 182, target);
        return target;
      } else {
        pExpr = pExpr->pLeft;
        goto expr_code_doover;
      }
    }
    case 181:
    case 173: {
      pExpr = pExpr->pLeft;
      goto expr_code_doover;
    }

    case 78: {
      Table *pTab;
      int iCol;
      int p1;

      pTab = pExpr->y.pTab;
      iCol = pExpr->iColumn;
      p1 = pExpr->iTable * (pTab->nCol + 1) + 1 + sqlite3TableColumnToStorage(pTab, iCol);

      sqlite3VdbeAddOp2(v, 159, p1, target);

      if (iCol >= 0 && pTab->aCol[iCol].affinity == 0x45) {
        sqlite3VdbeAddOp1(v, 89, target);
      }

      break;
    }

    case 177: {
      sqlite3ErrorMsg(pParse, "row value misused");
      break;
    }

    case 179: {
      int addrINR;
      u8 okConstFactor = pParse->okConstFactor;
      AggInfo *pAggInfo = pExpr->pAggInfo;
      if (pAggInfo) {
        if (!pAggInfo->directMode) {
          inReg = ((pAggInfo)->iFirstReg + (pExpr->iAgg));
          break;
        }
        if (pExpr->pAggInfo->useSortingIdx) {
          sqlite3VdbeAddOp3(v, 96, pAggInfo->sortingIdxPTab, pAggInfo->aCol[pExpr->iAgg].iSorterColumn, target);
          inReg = target;
          break;
        }
      }
      addrINR = sqlite3VdbeAddOp3(v, 20, pExpr->iTable, 0, target);

      pParse->okConstFactor = 0;
      sqlite3ExprCode(pParse, pExpr->pLeft, target);

      pParse->okConstFactor = okConstFactor;
      sqlite3VdbeJumpHere(v, addrINR);
      break;
    }

    case 158: {
      int endLabel;
      int nextCase;
      int nExpr;
      int i;
      ExprList *pEList;
      struct ExprList_item *aListelem;
      Expr opCompare;
      Expr *pX;
      Expr *pTest = 0;
      Expr *pDel = 0;
      sqlite3 *db = pParse->db;

      pEList = pExpr->x.pList;
      aListelem = pEList->a;
      nExpr = pEList->nExpr;
      endLabel = sqlite3VdbeMakeLabel(pParse);
      if ((pX = pExpr->pLeft) != 0) {
        pDel = sqlite3ExprDup(db, pX, 0);
        if (db->mallocFailed) {
          sqlite3ExprDelete(db, pDel);
          break;
        };
        sqlite3ExprToRegister(pDel, exprCodeVector(pParse, pDel, &regFree1));
        memset(&opCompare, 0, sizeof(opCompare));
        opCompare.op = 54;
        opCompare.pLeft = pDel;
        pTest = &opCompare;

        regFree1 = 0;
      }
      for (i = 0; i < nExpr - 1; i = i + 2) {
        if (pX) {
          opCompare.pRight = aListelem[i].pExpr;
        } else {
          pTest = aListelem[i].pExpr;
        }
        nextCase = sqlite3VdbeMakeLabel(pParse);
        sqlite3ExprIfFalse(pParse, pTest, nextCase, 0x10);
        sqlite3ExprCode(pParse, aListelem[i + 1].pExpr, target);
        sqlite3VdbeGoto(v, endLabel);
        sqlite3VdbeResolveLabel(v, nextCase);
      }
      if ((nExpr & 1) != 0) {
        sqlite3ExprCode(pParse, pEList->a[nExpr - 1].pExpr, target);
      } else {
        sqlite3VdbeAddOp2(v, 77, 0, target);
      }
      sqlite3ExprDelete(db, pDel);
      setDoNotMergeFlagOnCopy(v);
      sqlite3VdbeResolveLabel(v, endLabel);
      break;
    }

    case 72: {
      if (!pParse->pTriggerTab && !pParse->nested) {
        sqlite3ErrorMsg(pParse, "RAISE() may only be used within a trigger-program");
        return 0;
      }
      if (pExpr->affExpr == 2) {
        sqlite3MayAbort(pParse);
      }

      if (pExpr->affExpr == 4) {
        sqlite3VdbeAddOp2(v, 72, SQLITE_OK, 4);
      } else {
        r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
        sqlite3VdbeAddOp3(v, 72, pParse->pTriggerTab ? (19 | (7 << 8)) : SQLITE_ERROR, pExpr->affExpr, r1);
      }
      break;
    }
  }
  sqlite3ReleaseTempReg(pParse, regFree1);
  sqlite3ReleaseTempReg(pParse, regFree2);
  return inReg;
}

int sqlite3ExprCodeRunJustOnce(Parse *pParse, Expr *pExpr, int regDest) {
  ExprList *p;

  p = pParse->pConstExpr;
  if (regDest < 0 && p) {
    struct ExprList_item *pItem;
    int i;
    for (pItem = p->a, i = p->nExpr; i > 0; pItem++, i--) {
      if (pItem->fg.reusable && sqlite3ExprCompare(0, pItem->pExpr, pExpr, -1) == 0) {
        return pItem->u.iConstExprReg;
      }
    }
  }
  pExpr = sqlite3ExprDup(pParse->db, pExpr, 0);
  if (pExpr != 0 && (((pExpr)->flags & (u32)(0x000008)) != 0)) {
    Vdbe *v = pParse->pVdbe;
    int addr;

    addr = sqlite3VdbeAddOp0(v, 15);
    pParse->okConstFactor = 0;
    if (!pParse->db->mallocFailed) {
      if (regDest < 0)
        regDest = ++pParse->nMem;
      sqlite3ExprCode(pParse, pExpr, regDest);
    }
    pParse->okConstFactor = 1;
    sqlite3ExprDelete(pParse->db, pExpr);
    sqlite3VdbeJumpHere(v, addr);
  } else {
    p = sqlite3ExprListAppend(pParse, p, pExpr);
    if (p) {
      struct ExprList_item *pItem = &p->a[p->nExpr - 1];
      pItem->fg.reusable = regDest < 0;
      if (regDest < 0)
        regDest = ++pParse->nMem;
      pItem->u.iConstExprReg = regDest;
    }
    pParse->pConstExpr = p;
  }
  return regDest;
}

__attribute__((noinline)) void sqlite3ExprNullRegisterRange(Parse *pParse, int iReg, int nReg) {
  u8 okConstFactor = pParse->okConstFactor;
  Expr t;
  memset(&t, 0, sizeof(t));
  t.op = 83;
  t.y.nReg = nReg;
  pParse->okConstFactor = 1;
  sqlite3ExprCodeRunJustOnce(pParse, &t, iReg);
  pParse->okConstFactor = okConstFactor;
}

int sqlite3ExprCodeTemp(Parse *pParse, Expr *pExpr, int *pReg) {
  int r2;
  pExpr = sqlite3ExprSkipCollateAndLikely(pExpr);
  if (((pParse)->okConstFactor) && (pExpr != 0) && pExpr->op != 176 && sqlite3ExprIsConstantNotJoin(pParse, pExpr)) {
    *pReg = 0;
    r2 = sqlite3ExprCodeRunJustOnce(pParse, pExpr, -1);
  } else {
    int r1 = sqlite3GetTempReg(pParse);
    r2 = sqlite3ExprCodeTarget(pParse, pExpr, r1);
    if (r2 == r1) {
      *pReg = r1;
    } else {
      sqlite3ReleaseTempReg(pParse, r1);
      *pReg = 0;
    }
  }
  return r2;
}

void sqlite3ExprCode(Parse *pParse, Expr *pExpr, int target) {
  int inReg;

  if (pParse->pVdbe == 0)
    return;
  inReg = sqlite3ExprCodeTarget(pParse, pExpr, target);
  if (inReg != target) {
    u8 op;
    Expr *pX = sqlite3ExprSkipCollateAndLikely(pExpr);
    if ((pX) && ((((pX)->flags & (u32)(0x400000)) != 0) || pX->op == 176)) {
      op = 82;
    } else {
      op = 83;
    }
    sqlite3VdbeAddOp2(pParse->pVdbe, op, inReg, target);
  }
}

void sqlite3ExprCodeCopy(Parse *pParse, Expr *pExpr, int target) {
  sqlite3 *db = pParse->db;
  pExpr = sqlite3ExprDup(db, pExpr, 0);
  if (!db->mallocFailed)
    sqlite3ExprCode(pParse, pExpr, target);
  sqlite3ExprDelete(db, pExpr);
}

void sqlite3ExprCodeFactorable(Parse *pParse, Expr *pExpr, int target) {
  if (pParse->okConstFactor && sqlite3ExprIsConstantNotJoin(pParse, pExpr)) {
    sqlite3ExprCodeRunJustOnce(pParse, pExpr, target);
  } else {
    sqlite3ExprCodeCopy(pParse, pExpr, target);
  }
}

int sqlite3ExprCodeExprList(Parse *pParse, ExprList *pList, int target, int srcReg, u8 flags) {
  struct ExprList_item *pItem;
  int i, j, n;
  u8 copyOp = (flags & 0x01) ? 82 : 83;
  Vdbe *v = pParse->pVdbe;

  n = pList->nExpr;
  if (!((pParse)->okConstFactor))
    flags &= ~0x02;
  for (pItem = pList->a, i = 0; i < n; i++, pItem++) {
    Expr *pExpr = pItem->pExpr;

    if ((flags & 0x04) != 0 && (j = pItem->u.x.iOrderByCol) > 0) {
      if (flags & 0x08) {
        i--;
        n--;
      } else {
        sqlite3VdbeAddOp2(v, copyOp, j + srcReg - 1, target + i);
      }
    } else if ((flags & 0x02) != 0 && sqlite3ExprIsConstantNotJoin(pParse, pExpr)) {
      sqlite3ExprCodeRunJustOnce(pParse, pExpr, target + i);
    } else {
      int inReg = sqlite3ExprCodeTarget(pParse, pExpr, target + i);
      if (inReg != target + i) {
        VdbeOp *pOp;
        if (copyOp == 82 && (pOp = sqlite3VdbeGetLastOp(v))->opcode == 82 && pOp->p1 + pOp->p3 + 1 == inReg &&
            pOp->p2 + pOp->p3 + 1 == target + i && pOp->p5 == 0) {
          pOp->p3++;
        } else {
          sqlite3VdbeAddOp2(v, copyOp, inReg, target + i);
        }
      }
    }
  }
  return n;
}

void exprCodeBetween(Parse *pParse, Expr *pExpr, int dest, void (*xJump)(Parse *, Expr *, int, int), int jumpIfNull) {
  Expr exprAnd;
  Expr compLeft;
  Expr compRight;
  int regFree1 = 0;
  Expr *pDel = 0;
  sqlite3 *db = pParse->db;

  memset(&compLeft, 0, sizeof(Expr));
  memset(&compRight, 0, sizeof(Expr));
  memset(&exprAnd, 0, sizeof(Expr));

  pDel = sqlite3ExprDup(db, pExpr->pLeft, 0);
  if (db->mallocFailed == 0) {
    exprAnd.op = 44;
    exprAnd.pLeft = &compLeft;
    exprAnd.pRight = &compRight;
    compLeft.op = 58;
    compLeft.pLeft = pDel;
    compLeft.pRight = pExpr->x.pList->a[0].pExpr;
    compRight.op = 56;
    compRight.pLeft = pDel;
    compRight.pRight = pExpr->x.pList->a[1].pExpr;
    sqlite3ExprToRegister(pDel, exprCodeVector(pParse, pDel, &regFree1));
    if (xJump) {
      xJump(pParse, &exprAnd, dest, jumpIfNull);
    } else {
      pDel->flags |= 0x000001;
      sqlite3ExprCodeTarget(pParse, &exprAnd, dest);
    }
    sqlite3ReleaseTempReg(pParse, regFree1);
  }
  sqlite3ExprDelete(db, pDel);
}

void sqlite3ExprIfTrue(Parse *pParse, Expr *pExpr, int dest, int jumpIfNull) {
  Vdbe *v = pParse->pVdbe;
  int op = 0;
  int regFree1 = 0;
  int regFree2 = 0;
  int r1, r2;

  if (v == 0)
    return;
  if (pExpr == 0)
    return;

  op = pExpr->op;
  switch (op) {
    case 44:
    case 43: {
      Expr *pAlt = sqlite3ExprSimplifiedAndOr(pExpr);
      if (pAlt != pExpr) {
        sqlite3ExprIfTrue(pParse, pAlt, dest, jumpIfNull);
      } else {
        Expr *pFirst, *pSecond;
        if (exprEvalRhsFirst(pExpr)) {
          pFirst = pExpr->pRight;
          pSecond = pExpr->pLeft;
        } else {
          pFirst = pExpr->pLeft;
          pSecond = pExpr->pRight;
        }
        if (op == 44) {
          int d2 = sqlite3VdbeMakeLabel(pParse);
          sqlite3ExprIfFalse(pParse, pFirst, d2, jumpIfNull ^ 0x10);
          sqlite3ExprIfTrue(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        } else {
          sqlite3ExprIfTrue(pParse, pFirst, dest, jumpIfNull);
          sqlite3ExprIfTrue(pParse, pSecond, dest, jumpIfNull);
        }
      }
      break;
    }
    case 19: {
      sqlite3ExprIfFalse(pParse, pExpr->pLeft, dest, jumpIfNull);
      break;
    }
    case 175: {
      int isNot;
      int isTrue;
      isNot = pExpr->op2 == 46;
      isTrue = sqlite3ExprTruthValue(pExpr->pRight);
      if (isTrue ^ isNot) {
        sqlite3ExprIfTrue(pParse, pExpr->pLeft, dest, isNot ? 0x10 : 0);
      } else {
        sqlite3ExprIfFalse(pParse, pExpr->pLeft, dest, isNot ? 0x10 : 0);
      }
      break;
    }
    case 45:
    case 46:;
      op = (op == 45) ? 54 : 53;
      jumpIfNull = 0x80;
      __attribute__((fallthrough));
    case 57:
    case 56:
    case 55:
    case 58:
    case 53:
    case 54: {
      int addrIsNull;
      if (sqlite3ExprIsVector(pExpr->pLeft))
        goto default_expr;
      if ((((pExpr)->flags & (u32)(0x400000)) != 0) && jumpIfNull != 0x80) {
        addrIsNull = exprComputeOperands(pParse, pExpr, &r1, &r2, &regFree1, &regFree2);
      } else {
        r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
        r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, pExpr->pLeft, pExpr->pRight, op, r1, r2, dest, jumpIfNull,
                  (((pExpr)->flags & (u32)(0x000400)) != 0));

      if (addrIsNull) {
        if (jumpIfNull) {
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        } else {
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      break;
    }
    case 51:
    case 52: {
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);

      if (regFree1)
        sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      break;
    }
    case 49: {
      exprCodeBetween(pParse, pExpr, dest, sqlite3ExprIfTrue, jumpIfNull);
      break;
    }

    case 50: {
      int destIfFalse = sqlite3VdbeMakeLabel(pParse);
      int destIfNull = jumpIfNull ? dest : destIfFalse;
      sqlite3ExprCodeIN(pParse, pExpr, destIfFalse, destIfNull);
      sqlite3VdbeGoto(v, dest);
      sqlite3VdbeResolveLabel(v, destIfFalse);
      break;
    }

    default: {
    default_expr:
      if ((((pExpr)->flags & (0x000001 | 0x10000000)) == 0x10000000)) {
        sqlite3VdbeGoto(v, dest);
      } else if ((((pExpr)->flags & (0x000001 | 0x20000000)) == 0x20000000)) {
      } else {
        r1 = sqlite3ExprCodeTemp(pParse, pExpr, &regFree1);
        sqlite3VdbeAddOp3(v, 16, r1, dest, jumpIfNull != 0);
      }
      break;
    }
  }
  sqlite3ReleaseTempReg(pParse, regFree1);
  sqlite3ReleaseTempReg(pParse, regFree2);
}

void sqlite3ExprIfFalse(Parse *pParse, Expr *pExpr, int dest, int jumpIfNull) {
  Vdbe *v = pParse->pVdbe;
  int op = 0;
  int regFree1 = 0;
  int regFree2 = 0;
  int r1, r2;

  if (v == 0)
    return;
  if (pExpr == 0)
    return;

  op = ((pExpr->op + (51 & 1)) ^ 1) - (51 & 1);

  switch (pExpr->op) {
    case 44:
    case 43: {
      Expr *pAlt = sqlite3ExprSimplifiedAndOr(pExpr);
      if (pAlt != pExpr) {
        sqlite3ExprIfFalse(pParse, pAlt, dest, jumpIfNull);
      } else {
        Expr *pFirst, *pSecond;
        if (exprEvalRhsFirst(pExpr)) {
          pFirst = pExpr->pRight;
          pSecond = pExpr->pLeft;
        } else {
          pFirst = pExpr->pLeft;
          pSecond = pExpr->pRight;
        }
        if (pExpr->op == 44) {
          sqlite3ExprIfFalse(pParse, pFirst, dest, jumpIfNull);
          sqlite3ExprIfFalse(pParse, pSecond, dest, jumpIfNull);
        } else {
          int d2 = sqlite3VdbeMakeLabel(pParse);
          sqlite3ExprIfTrue(pParse, pFirst, d2, jumpIfNull ^ 0x10);
          sqlite3ExprIfFalse(pParse, pSecond, dest, jumpIfNull);
          sqlite3VdbeResolveLabel(v, d2);
        }
      }
      break;
    }
    case 19: {
      sqlite3ExprIfTrue(pParse, pExpr->pLeft, dest, jumpIfNull);
      break;
    }
    case 175: {
      int isNot;
      int isTrue;
      isNot = pExpr->op2 == 46;
      isTrue = sqlite3ExprTruthValue(pExpr->pRight);
      if (isTrue ^ isNot) {
        sqlite3ExprIfFalse(pParse, pExpr->pLeft, dest, isNot ? 0 : 0x10);

      } else {
        sqlite3ExprIfTrue(pParse, pExpr->pLeft, dest, isNot ? 0 : 0x10);
      }
      break;
    }
    case 45:
    case 46:;
      op = (pExpr->op == 45) ? 53 : 54;
      jumpIfNull = 0x80;
      __attribute__((fallthrough));
    case 57:
    case 56:
    case 55:
    case 58:
    case 53:
    case 54: {
      int addrIsNull;
      if (sqlite3ExprIsVector(pExpr->pLeft))
        goto default_expr;
      if ((((pExpr)->flags & (u32)(0x400000)) != 0) && jumpIfNull != 0x80) {
        addrIsNull = exprComputeOperands(pParse, pExpr, &r1, &r2, &regFree1, &regFree2);
      } else {
        r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);
        r2 = sqlite3ExprCodeTemp(pParse, pExpr->pRight, &regFree2);
        addrIsNull = 0;
      }
      codeCompare(pParse, pExpr->pLeft, pExpr->pRight, op, r1, r2, dest, jumpIfNull,
                  (((pExpr)->flags & (u32)(0x000400)) != 0));

      if (addrIsNull) {
        if (jumpIfNull) {
          sqlite3VdbeChangeP2(v, addrIsNull, dest);
        } else {
          sqlite3VdbeJumpHere(v, addrIsNull);
        }
      }
      break;
    }
    case 51:
    case 52: {
      r1 = sqlite3ExprCodeTemp(pParse, pExpr->pLeft, &regFree1);

      if (regFree1)
        sqlite3VdbeTypeofColumn(v, r1);
      sqlite3VdbeAddOp2(v, op, r1, dest);
      break;
    }
    case 49: {
      exprCodeBetween(pParse, pExpr, dest, sqlite3ExprIfFalse, jumpIfNull);
      break;
    }

    case 50: {
      if (jumpIfNull) {
        sqlite3ExprCodeIN(pParse, pExpr, dest, dest);
      } else {
        int destIfNull = sqlite3VdbeMakeLabel(pParse);
        sqlite3ExprCodeIN(pParse, pExpr, dest, destIfNull);
        sqlite3VdbeResolveLabel(v, destIfNull);
      }
      break;
    }

    default: {
    default_expr:
      if ((((pExpr)->flags & (0x000001 | 0x20000000)) == 0x20000000)) {
        sqlite3VdbeGoto(v, dest);
      } else if ((((pExpr)->flags & (0x000001 | 0x10000000)) == 0x10000000)) {
      } else {
        r1 = sqlite3ExprCodeTemp(pParse, pExpr, &regFree1);
        sqlite3VdbeAddOp3(v, 17, r1, dest, jumpIfNull != 0);
      }
      break;
    }
  }
  sqlite3ReleaseTempReg(pParse, regFree1);
  sqlite3ReleaseTempReg(pParse, regFree2);
}

void sqlite3ExprIfFalseDup(Parse *pParse, Expr *pExpr, int dest, int jumpIfNull) {
  sqlite3 *db = pParse->db;
  Expr *pCopy = sqlite3ExprDup(db, pExpr, 0);
  if (db->mallocFailed == 0) {
    sqlite3ExprIfFalse(pParse, pCopy, dest, jumpIfNull);
  }
  sqlite3ExprDelete(db, pCopy);
}

__attribute__((noinline)) int exprCompareVariable(const Parse *pParse, const Expr *pVar, const Expr *pExpr) {
  int res = 2;
  int iVar;
  sqlite3_value *pL, *pR = 0;

  if (pExpr->op == 157 && pVar->iColumn == pExpr->iColumn) {
    return 0;
  }
  if ((pParse->db->flags & 0x00800000) != 0)
    return 2;
  sqlite3ValueFromExpr(pParse->db, pExpr, SQLITE_UTF8, 0x41, &pR);
  if (pR) {
    iVar = pVar->iColumn;
    sqlite3VdbeSetVarmask(pParse->pVdbe, iVar);
    pL = sqlite3VdbeGetBoundValue(pParse->pReprepare, iVar, 0x41);
    if (pL) {
      if (sqlite3_value_type(pL) == 3) {
        sqlite3_value_text(pL);
      }
      res = sqlite3MemCompare(pL, pR, 0) ? 2 : 0;
    }
    sqlite3ValueFree(pR);
    sqlite3ValueFree(pL);
  }
  return res;
}

int sqlite3ExprCompare(const Parse *pParse, const Expr *pA, const Expr *pB, int iTab) {
  u32 combinedFlags;
  if (pA == 0 || pB == 0) {
    return pB == pA ? 0 : 2;
  }
  if (pParse && pA->op == 157) {
    return exprCompareVariable(pParse, pA, pB);
  }
  combinedFlags = pA->flags | pB->flags;
  if (combinedFlags & 0x000800) {
    if ((pA->flags & pB->flags & 0x000800) != 0 && pA->u.iValue == pB->u.iValue) {
      return 0;
    }
    return 2;
  }
  if (pA->op != pB->op || pA->op == 72) {
    if (pA->op == 114 && sqlite3ExprCompare(pParse, pA->pLeft, pB, iTab) < 2) {
      return 1;
    }
    if (pB->op == 114 && sqlite3ExprCompare(pParse, pA, pB->pLeft, iTab) < 2) {
      return 1;
    }
    if (pA->op == 170 && pB->op == 168 && pB->iTable < 0 && pA->iTable == iTab) {
    } else {
      return 2;
    }
  }

  if (pA->u.zToken) {
    if (pA->op == 172 || pA->op == 169) {
      if (sqlite3StrICmp(pA->u.zToken, pB->u.zToken) != 0)
        return 2;

      if ((((pA)->flags & (u32)(0x1000000)) != 0) != (((pB)->flags & (u32)(0x1000000)) != 0)) {
        return 2;
      }
      if ((((pA)->flags & (u32)(0x1000000)) != 0)) {
        if (sqlite3WindowCompare(pParse, pA->y.pWin, pB->y.pWin, 1) != 0) {
          return 2;
        }
      }

    } else if (pA->op == 122) {
      return 0;
    } else if (pA->op == 114) {
      if (sqlite3_stricmp(pA->u.zToken, pB->u.zToken) != 0)
        return 2;
    } else if (pB->u.zToken != 0 && pA->op != 168 && pA->op != 170 && strcmp(pA->u.zToken, pB->u.zToken) != 0) {
      return 2;
    }
  }
  if ((pA->flags & (0x000004 | 0x000400)) != (pB->flags & (0x000004 | 0x000400)))
    return 2;
  if (((combinedFlags & 0x010000) == 0)) {
    if (combinedFlags & 0x001000)
      return 2;
    if ((combinedFlags & 0x000020) == 0 && sqlite3ExprCompare(pParse, pA->pLeft, pB->pLeft, iTab))
      return 2;
    if (sqlite3ExprCompare(pParse, pA->pRight, pB->pRight, iTab))
      return 2;
    if (sqlite3ExprListCompare(pA->x.pList, pB->x.pList, iTab))
      return 2;
    if (pA->op != 118 && pA->op != 171 && ((combinedFlags & 0x004000) == 0)) {
      if (pA->iColumn != pB->iColumn)
        return 2;
      if (pA->op2 != pB->op2 && pA->op == 175)
        return 2;
      if (pA->op != 50 && pA->iTable != pB->iTable && pA->iTable != iTab) {
        return 2;
      }
    }
  }
  return 0;
}

int exprImpliesNotNull(const Parse *pParse, const Expr *p, const Expr *pNN, int iTab, int seenNot) {
  if (sqlite3ExprCompare(pParse, p, pNN, iTab) == 0) {
    return pNN->op != 122;
  }
  switch (p->op) {
    case 50: {
      if (seenNot && (((p)->flags & (u32)(0x001000)) != 0))
        return 0;

      return exprImpliesNotNull(pParse, p->pLeft, pNN, iTab, 1);
    }
    case 49: {
      ExprList *pList;

      pList = p->x.pList;

      if (seenNot)
        return 0;
      if (exprImpliesNotNull(pParse, pList->a[0].pExpr, pNN, iTab, 1) ||
          exprImpliesNotNull(pParse, pList->a[1].pExpr, pNN, iTab, 1)) {
        return 1;
      }
      return exprImpliesNotNull(pParse, p->pLeft, pNN, iTab, 1);
    }
    case 54:
    case 53:
    case 57:
    case 56:
    case 55:
    case 58:
    case 107:
    case 108:
    case 104:
    case 105:
    case 106:
    case 112:
      seenNot = 1;
      __attribute__((fallthrough));
    case 109:
    case 111:
    case 103:
    case 110: {
      if (exprImpliesNotNull(pParse, p->pRight, pNN, iTab, seenNot))
        return 1;
      __attribute__((fallthrough));
    }
    case 181:
    case 114:
    case 173:
    case 174: {
      return exprImpliesNotNull(pParse, p->pLeft, pNN, iTab, seenNot);
    }
    case 175: {
      if (seenNot)
        return 0;
      if (p->op2 != 45)
        return 0;
      return exprImpliesNotNull(pParse, p->pLeft, pNN, iTab, 1);
    }
    case 115:
    case 19: {
      return exprImpliesNotNull(pParse, p->pLeft, pNN, iTab, 1);
    }
  }
  return 0;
}

int sqlite3ExprImpliesExpr(const Parse *pParse, const Expr *pE1, const Expr *pE2, int iTab) {
  if (sqlite3ExprCompare(pParse, pE1, pE2, iTab) == 0) {
    return 1;
  }
  if (pE2->op == 43 && (sqlite3ExprImpliesExpr(pParse, pE1, pE2->pLeft, iTab) ||
                        sqlite3ExprImpliesExpr(pParse, pE1, pE2->pRight, iTab))) {
    return 1;
  }
  if (pE2->op == 52 && exprImpliesNotNull(pParse, pE1, pE2->pLeft, iTab, 0)) {
    return 1;
  }
  if (sqlite3ExprIsIIF(pParse->db, pE1)) {
    return sqlite3ExprImpliesExpr(pParse, pE1->x.pList->a[0].pExpr, pE2, iTab);
  }
  return 0;
}

int sqlite3ReferencesSrcList(Parse *pParse, Expr *pExpr, SrcList *pSrcList) {
  Walker w;
  struct RefSrcList x;

  memset(&w, 0, sizeof(w));
  memset(&x, 0, sizeof(x));
  w.xExprCallback = exprRefToSrcList;
  w.xSelectCallback = selectRefEnter;
  w.xSelectCallback2 = selectRefLeave;
  w.u.pRefSrcList = &x;
  x.db = pParse->db;
  x.pRef = pSrcList;

  sqlite3WalkExprList(&w, pExpr->x.pList);
  if (pExpr->pLeft) {
    sqlite3WalkExprList(&w, pExpr->pLeft->x.pList);
  }

  if ((((pExpr)->flags & (u32)(0x1000000)) != 0)) {
    sqlite3WalkExpr(&w, pExpr->y.pWin->pFilter);
  }

  if (x.aiExclude)
    sqlite3DbNNFreeNN(pParse->db, x.aiExclude);
  if (w.eCode & 0x01) {
    return 1;
  } else if (w.eCode) {
    return 0;
  } else {
    return -1;
  }
}

void findOrCreateAggInfoColumn(Parse *pParse, AggInfo *pAggInfo, Expr *pExpr) {
  struct AggInfo_col *pCol;
  int k;
  int mxTerm = pParse->db->aLimit[SQLITE_LIMIT_COLUMN];

  pCol = pAggInfo->aCol;
  for (k = 0; k < pAggInfo->nColumn; k++, pCol++) {
    if (pCol->pCExpr == pExpr)
      return;
    if (pCol->iTable == pExpr->iTable && pCol->iColumn == pExpr->iColumn && pExpr->op != 179) {
      goto fix_up_expr;
    }
  }
  k = addAggInfoColumn(pParse->db, pAggInfo);
  if (k < 0) {
    return;
  }
  if (k > mxTerm) {
    sqlite3ErrorMsg(pParse, "more than %d aggregate terms", mxTerm);
    k = mxTerm;
  }
  pCol = &pAggInfo->aCol[k];

  pCol->pTab = pExpr->y.pTab;
  pCol->iTable = pExpr->iTable;
  pCol->iColumn = pExpr->iColumn;
  pCol->iSorterColumn = -1;
  pCol->pCExpr = pExpr;
  if (pAggInfo->pGroupBy && pExpr->op != 179) {
    int j, n;
    ExprList *pGB = pAggInfo->pGroupBy;
    struct ExprList_item *pTerm = pGB->a;
    n = pGB->nExpr;
    for (j = 0; j < n; j++, pTerm++) {
      Expr *pE = pTerm->pExpr;
      if (pE->op == 168 && pE->iTable == pExpr->iTable && pE->iColumn == pExpr->iColumn) {
        pCol->iSorterColumn = j;
        break;
      }
    }
  }
  if (pCol->iSorterColumn < 0) {
    pCol->iSorterColumn = pAggInfo->nSortingColumn++;
  }
fix_up_expr:;

  pExpr->pAggInfo = pAggInfo;
  if (pExpr->op == 168) {
    pExpr->op = 170;
  }

  pExpr->iAgg = (i16)k;
}

int sqlite3GetTempReg(Parse *pParse) {
  if (pParse->nTempReg == 0) {
    return ++pParse->nMem;
  }
  return pParse->aTempReg[--pParse->nTempReg];
}

void sqlite3ReleaseTempReg(Parse *pParse, int iReg) {
  if (iReg) {
    if (pParse->nTempReg < ((int)(sizeof(pParse->aTempReg) / sizeof(pParse->aTempReg[0])))) {
      pParse->aTempReg[pParse->nTempReg++] = iReg;
    }
  }
}

int sqlite3GetTempRange(Parse *pParse, int nReg) {
  int i, n;
  if (nReg == 1)
    return sqlite3GetTempReg(pParse);
  i = pParse->iRangeReg;
  n = pParse->nRangeReg;
  if (nReg <= n) {
    pParse->iRangeReg += nReg;
    pParse->nRangeReg -= nReg;
  } else {
    i = pParse->nMem + 1;
    pParse->nMem += nReg;
  }
  return i;
}

void sqlite3ReleaseTempRange(Parse *pParse, int iReg, int nReg) {
  if (nReg == 1) {
    sqlite3ReleaseTempReg(pParse, iReg);
    return;
  };
  if (nReg > pParse->nRangeReg) {
    pParse->nRangeReg = nReg;
    pParse->iRangeReg = iReg;
  }
}

void sqlite3ClearTempRegCache(Parse *pParse) {
  pParse->nTempReg = 0;
  pParse->nRangeReg = 0;
}

void sqlite3TouchRegister(Parse *pParse, int iReg) {
  if (pParse->nMem < iReg)
    pParse->nMem = iReg;
}

int isAlterableTable(Parse *pParse, Table *pTab) {
  if (0 == sqlite3_strnicmp(pTab->zName, "sqlite_", 7) || (pTab->tabFlags & 0x00008000) != 0 ||
      ((pTab->tabFlags & 0x00001000) != 0 && sqlite3ReadOnlyShadowTables(pParse->db))) {
    sqlite3ErrorMsg(pParse, "table %s may not be altered", pTab->zName);
    return 1;
  }
  return 0;
}

void renameTestSchema(Parse *pParse, const char *zDb, int bTemp, const char *zWhen, int bNoDQS) {
  pParse->colNamesSet = 1;
  sqlite3NestedParse(pParse,
                     "SELECT 1 "
                     "FROM \"%w\"."
                     "sqlite_master"
                     " "
                     "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
                     " AND sql NOT LIKE 'create virtual%%'"
                     " AND sqlite_rename_test(%Q, sql, type, name, %d, %Q, %d)=NULL ",
                     zDb, zDb, bTemp, zWhen, bNoDQS);

  if (bTemp == 0) {
    sqlite3NestedParse(pParse,
                       "SELECT 1 "
                       "FROM temp."
                       "sqlite_master"
                       " "
                       "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
                       " AND sql NOT LIKE 'create virtual%%'"
                       " AND sqlite_rename_test(%Q, sql, type, name, 1, %Q, %d)=NULL ",
                       zDb, zWhen, bNoDQS);
  }
}

void renameFixQuotes(Parse *pParse, const char *zDb, int bTemp) {
  sqlite3NestedParse(pParse,
                     "UPDATE \"%w\"."
                     "sqlite_master"
                     " SET sql = sqlite_rename_quotefix(%Q, sql)"
                     "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
                     " AND sql NOT LIKE 'create virtual%%'",
                     zDb, zDb);
  if (bTemp == 0) {
    sqlite3NestedParse(pParse,
                       "UPDATE temp."
                       "sqlite_master"
                       " SET sql = sqlite_rename_quotefix('temp', sql)"
                       "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X'"
                       " AND sql NOT LIKE 'create virtual%%'");
  }
}

void renameReloadSchema(Parse *pParse, int iDb, u16 p5) {
  Vdbe *v = pParse->pVdbe;
  if (v) {
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3VdbeAddParseSchemaOp(pParse->pVdbe, iDb, 0, p5);
    if (iDb != 1)
      sqlite3VdbeAddParseSchemaOp(pParse->pVdbe, 1, 0, p5);
  }
}

void sqlite3AlterRenameTable(Parse *pParse, SrcList *pSrc, Token *pName) {
  int iDb;
  char *zDb;
  Table *pTab;
  char *zName = 0;
  sqlite3 *db = pParse->db;
  int nTabName;
  const char *zTabName;
  Vdbe *v;
  VTable *pVTab = 0;

  if ((db->mallocFailed))
    goto exit_rename_table;

  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  if (!pTab)
    goto exit_rename_table;
  iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
  zDb = db->aDb[iDb].zDbSName;

  zName = sqlite3NameFromToken(db, pName);
  if (!zName)
    goto exit_rename_table;

  if (sqlite3FindTable(db, zName, zDb) || sqlite3FindIndex(db, zName, zDb) || sqlite3IsShadowTableOf(db, pTab, zName)) {
    sqlite3ErrorMsg(pParse, "there is already another table or index with this name: %s", zName);
    goto exit_rename_table;
  }

  if (SQLITE_OK != isAlterableTable(pParse, pTab)) {
    goto exit_rename_table;
  }
  if (SQLITE_OK != sqlite3CheckObjectName(pParse, zName, "table", zName)) {
    goto exit_rename_table;
  }

  if ((pTab)->eTabType == 2) {
    sqlite3ErrorMsg(pParse, "view %s may not be altered", pTab->zName);
    goto exit_rename_table;
  }

  if (sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0)) {
    goto exit_rename_table;
  }

  if (sqlite3ViewGetColumnNames(pParse, pTab)) {
    goto exit_rename_table;
  }
  if ((pTab)->eTabType == 1) {
    pVTab = sqlite3GetVTable(db, pTab);
    if (pVTab->pVtab->pModule->xRename == 0) {
      pVTab = 0;
    }
  }

  v = sqlite3GetVdbe(pParse);
  if (v == 0) {
    goto exit_rename_table;
  }
  sqlite3MayAbort(pParse);

  zTabName = pTab->zName;
  nTabName = sqlite3Utf8CharLen(zTabName, -1);

  sqlite3NestedParse(pParse,
                     "UPDATE \"%w\"."
                     "sqlite_master"
                     " SET "
                     "sql = sqlite_rename_table(%Q, type, name, sql, %Q, %Q, %d) "
                     "WHERE (type!='index' OR tbl_name=%Q COLLATE nocase)"
                     "AND   name NOT LIKE 'sqliteX_%%' ESCAPE 'X'",
                     zDb, zDb, zTabName, zName, (iDb == 1), zTabName);

  sqlite3NestedParse(pParse,
                     "UPDATE %Q."
                     "sqlite_master"
                     " SET "
                     "tbl_name = %Q, "
                     "name = CASE "
                     "WHEN type='table' THEN %Q "
                     "WHEN name LIKE 'sqliteX_autoindex%%' ESCAPE 'X' "
                     "     AND type='index' THEN "
                     "'sqlite_autoindex_' || %Q || substr(name,%d+18) "
                     "ELSE name END "
                     "WHERE tbl_name=%Q COLLATE nocase AND "
                     "(type='table' OR type='index' OR type='trigger');",
                     zDb, zName, zName, zName, nTabName, zTabName);

  if (sqlite3FindTable(db, "sqlite_sequence", zDb)) {
    sqlite3NestedParse(pParse, "UPDATE \"%w\".sqlite_sequence set name = %Q WHERE name = %Q", zDb, zName, pTab->zName);
  }

  if (iDb != 1) {
    sqlite3NestedParse(pParse,
                       "UPDATE sqlite_temp_schema SET "
                       "sql = sqlite_rename_table(%Q, type, name, sql, %Q, %Q, 1), "
                       "tbl_name = "
                       "CASE WHEN tbl_name=%Q COLLATE nocase AND "
                       "  sqlite_rename_test(%Q, sql, type, name, 1, 'after rename', 0) "
                       "THEN %Q ELSE tbl_name END "
                       "WHERE type IN ('view', 'trigger')",
                       zDb, zTabName, zName, zTabName, zDb, zName);
  }

  if (pVTab) {
    int i = ++pParse->nMem;
    sqlite3VdbeLoadString(v, i, zName);
    sqlite3VdbeAddOp4(v, 179, i, 0, 0, (const char *)pVTab, (-12));
  }

  renameReloadSchema(pParse, iDb, 0x0001);
  renameTestSchema(pParse, zDb, iDb == 1, "after rename", 0);

exit_rename_table:
  sqlite3SrcListDelete(db, pSrc);
  sqlite3DbFree(db, zName);
}

void sqlite3ErrorIfNotEmpty(Parse *pParse, const char *zDb, const char *zTab, const char *zErr) {
  sqlite3NestedParse(pParse, "SELECT raise(ABORT,%Q) FROM \"%w\".\"%w\"", zErr, zDb, zTab);
}

void sqlite3AlterFinishAddColumn(Parse *pParse, Token *pColDef) {
  Table *pNew;
  Table *pTab;
  int iDb;
  const char *zDb;
  const char *zTab;
  char *zCol;
  Column *pCol;
  Expr *pDflt;
  sqlite3 *db;
  Vdbe *v;
  int r1;

  db = pParse->db;

  if (pParse->nErr)
    return;

  pNew = pParse->pNewTable;

  iDb = sqlite3SchemaToIndex(db, pNew->pSchema);
  zDb = db->aDb[iDb].zDbSName;
  zTab = &pNew->zName[16];
  pCol = &pNew->aCol[pNew->nCol - 1];
  pDflt = sqlite3ColumnExpr(pNew, pCol);
  pTab = sqlite3FindTable(db, zTab, zDb);

  if (sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0)) {
    return;
  }

  if (pCol->colFlags & 0x0001) {
    sqlite3ErrorMsg(pParse, "Cannot add a PRIMARY KEY column");
    return;
  }
  if (pNew->pIndex) {
    sqlite3ErrorMsg(pParse, "Cannot add a UNIQUE column");
    return;
  }
  if ((pCol->colFlags & 0x0060) == 0) {
    if (pDflt && pDflt->pLeft->op == 122) {
      pDflt = 0;
    }

    if ((db->flags & 0x00004000) && pNew->u.tab.pFKey && pDflt) {
      sqlite3ErrorIfNotEmpty(pParse, zDb, zTab, "Cannot add a REFERENCES column with non-NULL default value");
    }
    if (pCol->notNull && !pDflt) {
      sqlite3ErrorIfNotEmpty(pParse, zDb, zTab, "Cannot add a NOT NULL column with default value NULL");
    }

    if (pDflt) {
      sqlite3_value *pVal = 0;
      int rc;
      rc = sqlite3ValueFromExpr(db, pDflt, SQLITE_UTF8, 0x41, &pVal);

      if (rc != SQLITE_OK) {
        return;
      }
      if (!pVal) {
        sqlite3ErrorIfNotEmpty(pParse, zDb, zTab, "Cannot add a column with non-constant default");
      }
      sqlite3ValueFree(pVal);
    }
  } else if (pCol->colFlags & 0x0040) {
    sqlite3ErrorIfNotEmpty(pParse, zDb, zTab, "cannot add a STORED column");
  }

  zCol = sqlite3DbStrNDup(db, (char *)pColDef->z, pColDef->n);
  if (zCol) {
    char *zEnd = &zCol[pColDef->n - 1];
    while (zEnd > zCol && (*zEnd == ';' || (sqlite3CtypeMap[(unsigned char)(*zEnd)] & 0x01))) {
      *zEnd-- = '\0';
    }

    sqlite3NestedParse(pParse,
                       "UPDATE \"%w\"."
                       "sqlite_master"
                       " SET "
                       "sql = printf('%%.%ds, ',sql) || %Q"
                       " || substr(sql,1+length(printf('%%.%ds',sql))) "
                       "WHERE type = 'table' AND name = %Q",
                       zDb, pNew->u.tab.addColOffset, zCol, pNew->u.tab.addColOffset, zTab);
    sqlite3DbFree(db, zCol);
  }

  v = sqlite3GetVdbe(pParse);
  if (v) {
    r1 = sqlite3GetTempReg(pParse);
    sqlite3VdbeAddOp3(v, 101, iDb, r1, 2);
    sqlite3VdbeUsesBtree(v, iDb);
    sqlite3VdbeAddOp2(v, 88, r1, -2);
    sqlite3VdbeAddOp2(v, 61, r1, sqlite3VdbeCurrentAddr(v) + 2);
    sqlite3VdbeAddOp3(v, 102, iDb, 2, 3);
    sqlite3ReleaseTempReg(pParse, r1);

    renameReloadSchema(pParse, iDb, 0x0003);

    if (pNew->pCheck != 0 || (pCol->notNull && (pCol->colFlags & 0x0060) != 0) || (pTab->tabFlags & 0x00010000) != 0) {
      sqlite3NestedParse(pParse,
                         "SELECT CASE WHEN quick_check GLOB 'CHECK*'"
                         " THEN raise(ABORT,'CHECK constraint failed')"
                         " WHEN quick_check GLOB 'non-* value in*'"
                         " THEN raise(ABORT,'type mismatch on DEFAULT')"
                         " ELSE raise(ABORT,'NOT NULL constraint failed')"
                         " END"
                         "  FROM pragma_quick_check(%Q,%Q)"
                         " WHERE quick_check GLOB 'CHECK*'"
                         " OR quick_check GLOB 'NULL*'"
                         " OR quick_check GLOB 'non-* value in*'",
                         zTab, zDb);
    }
  }
}

void sqlite3AlterBeginAddColumn(Parse *pParse, SrcList *pSrc) {
  Table *pNew;
  Table *pTab;
  int iDb;
  int i;
  int nAlloc;
  sqlite3 *db = pParse->db;

  if ((db->mallocFailed))
    goto exit_begin_add_column;
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  if (!pTab)
    goto exit_begin_add_column;

  if ((pTab)->eTabType == 1) {
    sqlite3ErrorMsg(pParse, "virtual tables may not be altered");
    goto exit_begin_add_column;
  }

  if ((pTab)->eTabType == 2) {
    sqlite3ErrorMsg(pParse, "Cannot add a column to a view");
    goto exit_begin_add_column;
  }
  if (SQLITE_OK != isAlterableTable(pParse, pTab)) {
    goto exit_begin_add_column;
  }

  sqlite3MayAbort(pParse);

  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);

  pNew = (Table *)sqlite3DbMallocZero(db, sizeof(Table));
  if (!pNew)
    goto exit_begin_add_column;
  pParse->pNewTable = pNew;
  pNew->nTabRef = 1;
  pNew->nCol = pTab->nCol;

  nAlloc = (((pNew->nCol - 1) / 8) * 8) + 8;

  pNew->aCol = (Column *)sqlite3DbMallocZero(db, sizeof(Column) * (u32)nAlloc);
  pNew->zName = sqlite3MPrintf(db, "sqlite_altertab_%s", pTab->zName);
  if (!pNew->aCol || !pNew->zName) {
    goto exit_begin_add_column;
  }
  memcpy(pNew->aCol, pTab->aCol, sizeof(Column) * (size_t)pNew->nCol);
  for (i = 0; i < pNew->nCol; i++) {
    Column *pCol = &pNew->aCol[i];
    pCol->zCnName = sqlite3DbStrDup(db, pCol->zCnName);
    pCol->hName = sqlite3StrIHash(pCol->zCnName);
  }

  pNew->u.tab.pDfltList = sqlite3ExprListDup(db, pTab->u.tab.pDfltList, 0);
  pNew->pSchema = db->aDb[iDb].pSchema;
  pNew->u.tab.addColOffset = pTab->u.tab.addColOffset;

exit_begin_add_column:
  sqlite3SrcListDelete(db, pSrc);
  return;
}

int isRealTable(Parse *pParse, Table *pTab, int iOp) {
  const char *zType = 0;

  if ((pTab)->eTabType == 2) {
    zType = "view";
  }

  if ((pTab)->eTabType == 1) {
    zType = "virtual table";
  }

  if (zType) {
    const char *azMsg[] = {"rename columns of", "drop column from", "edit constraints of"};

    sqlite3ErrorMsg(pParse, "cannot %s %s \"%s\"", azMsg[iOp], zType, pTab->zName);
    return 1;
  }
  return 0;
}

void sqlite3AlterRenameColumn(Parse *pParse, SrcList *pSrc, Token *pOld, Token *pNew) {
  sqlite3 *db = pParse->db;
  Table *pTab;
  int iCol;
  char *zOld = 0;
  char *zNew = 0;
  const char *zDb;
  int iSchema;
  int bQuote;

  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  if (!pTab)
    goto exit_rename_column;

  if (SQLITE_OK != isAlterableTable(pParse, pTab))
    goto exit_rename_column;
  if (SQLITE_OK != isRealTable(pParse, pTab, 0))
    goto exit_rename_column;

  iSchema = sqlite3SchemaToIndex(db, pTab->pSchema);

  zDb = db->aDb[iSchema].zDbSName;

  if (sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, 0)) {
    goto exit_rename_column;
  }

  zOld = sqlite3NameFromToken(db, pOld);
  if (!zOld)
    goto exit_rename_column;
  iCol = sqlite3ColumnIndex(pTab, zOld);
  if (iCol < 0) {
    sqlite3ErrorMsg(pParse, "no such column: \"%T\"", pOld);
    goto exit_rename_column;
  }

  renameTestSchema(pParse, zDb, iSchema == 1, "", 0);
  renameFixQuotes(pParse, zDb, iSchema == 1);

  sqlite3MayAbort(pParse);
  zNew = sqlite3NameFromToken(db, pNew);
  if (!zNew)
    goto exit_rename_column;

  bQuote = (sqlite3CtypeMap[(unsigned char)(pNew->z[0])] & 0x80);
  sqlite3NestedParse(pParse,
                     "UPDATE \"%w\"."
                     "sqlite_master"
                     " SET "
                     "sql = sqlite_rename_column(sql, type, name, %Q, %Q, %d, %Q, %d, %d) "
                     "WHERE name NOT LIKE 'sqliteX_%%' ESCAPE 'X' "
                     " AND (type != 'index' OR tbl_name = %Q)",
                     zDb, zDb, pTab->zName, iCol, zNew, bQuote, iSchema == 1, pTab->zName);

  sqlite3NestedParse(pParse,
                     "UPDATE temp."
                     "sqlite_master"
                     " SET "
                     "sql = sqlite_rename_column(sql, type, name, %Q, %Q, %d, %Q, %d, 1) "
                     "WHERE type IN ('trigger', 'view')",
                     zDb, pTab->zName, iCol, zNew, bQuote);

  renameReloadSchema(pParse, iSchema, 0x0001);
  renameTestSchema(pParse, zDb, iSchema == 1, "after rename", 1);

exit_rename_column:
  sqlite3SrcListDelete(db, pSrc);
  sqlite3DbFree(db, zOld);
  sqlite3DbFree(db, zNew);
  return;
}

const void *sqlite3RenameTokenMap(Parse *pParse, const void *pPtr, const Token *pToken) {
  RenameToken *pNew;

  if ((pParse->eParseMode != 3)) {
    pNew = (RenameToken*)(sqlite3DbMallocZero(pParse->db, sizeof(RenameToken)));
    if (pNew) {
      pNew->p = pPtr;
      pNew->t = *pToken;
      pNew->pNext = pParse->pRename;
      pParse->pRename = pNew;
    }
  }

  return pPtr;
}

void sqlite3RenameTokenRemap(Parse *pParse, const void *pTo, const void *pFrom) {
  RenameToken *p;
  for (p = pParse->pRename; p; p = p->pNext) {
    if (p->p == pFrom) {
      p->p = pTo;
      break;
    }
  }
}

void unmapColumnIdlistNames(Parse *pParse, const IdList *pIdList) {
  int ii;

  for (ii = 0; ii < pIdList->nId; ii++) {
    sqlite3RenameTokenRemap(pParse, 0, (const void *)pIdList->a[ii].zName);
  }
}

void sqlite3RenameExprUnmap(Parse *pParse, Expr *pExpr) {
  u8 eMode = pParse->eParseMode;
  Walker sWalker;
  memset(&sWalker, 0, sizeof(Walker));
  sWalker.pParse = pParse;
  sWalker.xExprCallback = renameUnmapExprCb;
  sWalker.xSelectCallback = renameUnmapSelectCb;
  pParse->eParseMode = 3;
  sqlite3WalkExpr(&sWalker, pExpr);
  pParse->eParseMode = eMode;
}

void sqlite3RenameExprlistUnmap(Parse *pParse, ExprList *pEList) {
  if (pEList) {
    int i;
    Walker sWalker;
    memset(&sWalker, 0, sizeof(Walker));
    sWalker.pParse = pParse;
    sWalker.xExprCallback = renameUnmapExprCb;
    sqlite3WalkExprList(&sWalker, pEList);
    for (i = 0; i < pEList->nExpr; i++) {
      if (pEList->a[i].fg.eEName == 0) {
        sqlite3RenameTokenRemap(pParse, 0, (void *)pEList->a[i].zEName);
      }
    }
  }
}

RenameToken *renameTokenFind(Parse *pParse, struct RenameCtx *pCtx, const void *pPtr) {
  RenameToken **pp;
  if (pPtr == 0) {
    return 0;
  }
  for (pp = &pParse->pRename; (*pp); pp = &(*pp)->pNext) {
    if ((*pp)->p == pPtr) {
      RenameToken *pToken = *pp;
      if (pCtx) {
        *pp = pToken->pNext;
        pToken->pNext = pCtx->pList;
        pCtx->pList = pToken;
        pCtx->nList++;
      }
      return pToken;
    }
  }
  return 0;
}

void renameColumnElistNames(Parse *pParse, RenameCtx *pCtx, const ExprList *pEList, const char *zOld) {
  if (pEList) {
    int i;
    for (i = 0; i < pEList->nExpr; i++) {
      const char *zName = pEList->a[i].zEName;
      if ((pEList->a[i].fg.eEName == 0) && (zName != 0) && 0 == sqlite3_stricmp(zName, zOld)) {
        renameTokenFind(pParse, pCtx, (const void *)zName);
      }
    }
  }
}

void renameColumnIdlistNames(Parse *pParse, RenameCtx *pCtx, const IdList *pIdList, const char *zOld) {
  if (pIdList) {
    int i;
    for (i = 0; i < pIdList->nId; i++) {
      const char *zName = pIdList->a[i].zName;
      if (0 == sqlite3_stricmp(zName, zOld)) {
        renameTokenFind(pParse, pCtx, (const void *)zName);
      }
    }
  }
}

int renameParseSql(Parse *p, const char *zDb, sqlite3 *db, const char *zSql, int bTemp) {
  int rc;
  u64 flags;

  sqlite3ParseObjectInit(p, db);
  if (zSql == 0) {
    return SQLITE_NOMEM;
  }
  if (sqlite3_strnicmp(zSql, "CREATE ", 7) != 0) {
    return sqlite3CorruptError(121725);
  }
  if (bTemp) {
    db->init.iDb = 1;
  } else {
    int iDb = sqlite3FindDbName(db, zDb);

    db->init.iDb = (u8)iDb;
  }
  p->eParseMode = 2;
  p->db = db;
  p->nQueryLoop = 1;
  flags = db->flags;
  db->flags |= ((u64)(0x00040) << 32);
  rc = sqlite3RunParser(p, zSql);
  db->flags = flags;
  if (db->mallocFailed)
    rc = SQLITE_NOMEM;
  if (rc == SQLITE_OK && (p->pNewTable == 0 && p->pNewIndex == 0 && p->pNewTrigger == 0)) {
    rc = sqlite3CorruptError(121746);
  }

  db->init.iDb = 0;
  return rc;
}

int renameResolveTrigger(Parse *pParse) {
  sqlite3 *db = pParse->db;
  Trigger *pNew = pParse->pNewTrigger;
  TriggerStep *pStep;
  NameContext sNC;
  int rc = SQLITE_OK;

  memset(&sNC, 0, sizeof(sNC));
  sNC.pParse = pParse;

  pParse->pTriggerTab = sqlite3FindTable(db, pNew->table, db->aDb[sqlite3SchemaToIndex(db, pNew->pTabSchema)].zDbSName);
  pParse->eTriggerOp = pNew->op;

  if ((pParse->pTriggerTab)) {
    rc = sqlite3ViewGetColumnNames(pParse, pParse->pTriggerTab) != 0;
  }

  if (rc == SQLITE_OK && pNew->pWhen) {
    rc = sqlite3ResolveExprNames(&sNC, pNew->pWhen);
  }

  for (pStep = pNew->step_list; rc == SQLITE_OK && pStep; pStep = pStep->pNext) {
    if (pStep->pSelect) {
      sqlite3SelectPrep(pParse, pStep->pSelect, &sNC);
      if (pParse->nErr)
        rc = pParse->rc;
    }
    if (rc == SQLITE_OK && pStep->pSrc) {
      SrcList *pSrc = sqlite3SrcListDup(db, pStep->pSrc, 0);
      if (pSrc) {
        Select *pSel = sqlite3SelectNew(pParse, pStep->pExprList, pSrc, 0, 0, 0, 0, 0, 0);
        if (pSel == 0) {
          pStep->pExprList = 0;
          pSrc = 0;
          rc = SQLITE_NOMEM;
        } else {
          renameSetENames(pStep->pExprList, 1);
          sqlite3SelectPrep(pParse, pSel, 0);
          renameSetENames(pStep->pExprList, 0);
          rc = pParse->nErr ? SQLITE_ERROR : 0;

          if (pStep->pExprList)
            pSel->pEList = 0;
          pSel->pSrc = 0;
          sqlite3SelectDelete(db, pSel);
        }
        if ((pStep->pSrc)) {
          int i;
          for (i = 0; i < pStep->pSrc->nSrc && rc == SQLITE_OK; i++) {
            SrcItem *p = &pStep->pSrc->a[i];
            if (p->fg.isSubquery) {
              sqlite3SelectPrep(pParse, p->u4.pSubq->pSelect, 0);
            }
          }
        }

        if (db->mallocFailed) {
          rc = SQLITE_NOMEM;
        }
        sNC.pSrcList = pSrc;
        if (rc == SQLITE_OK && pStep->pWhere) {
          rc = sqlite3ResolveExprNames(&sNC, pStep->pWhere);
        }
        if (rc == SQLITE_OK) {
          rc = sqlite3ResolveExprListNames(&sNC, pStep->pExprList);
        }

        if (pStep->pUpsert && rc == SQLITE_OK) {
          Upsert *pUpsert = pStep->pUpsert;
          pUpsert->pUpsertSrc = pSrc;
          sNC.uNC.pUpsert = pUpsert;
          sNC.ncFlags = 0x000200;
          rc = sqlite3ResolveExprListNames(&sNC, pUpsert->pUpsertTarget);
          if (rc == SQLITE_OK) {
            ExprList *pUpsertSet = pUpsert->pUpsertSet;
            rc = sqlite3ResolveExprListNames(&sNC, pUpsertSet);
          }
          if (rc == SQLITE_OK) {
            rc = sqlite3ResolveExprNames(&sNC, pUpsert->pUpsertWhere);
          }
          if (rc == SQLITE_OK) {
            rc = sqlite3ResolveExprNames(&sNC, pUpsert->pUpsertTargetWhere);
          }
          sNC.ncFlags = 0;
        }
        sNC.pSrcList = 0;
        sqlite3SrcListDelete(db, pSrc);
      } else {
        rc = SQLITE_NOMEM;
      }
    }
  }
  return rc;
}

void renameParseCleanup(Parse *pParse) {
  sqlite3 *db = pParse->db;
  Index *pIdx;
  if (pParse->pVdbe) {
    sqlite3VdbeFinalize(pParse->pVdbe);
  }
  sqlite3DeleteTable(db, pParse->pNewTable);
  while ((pIdx = pParse->pNewIndex) != 0) {
    pParse->pNewIndex = pIdx->pNext;
    sqlite3FreeIndex(db, pIdx);
  }
  sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  sqlite3DbFree(db, pParse->zErrMsg);
  renameTokenFree(db, pParse->pRename);
  sqlite3ParseObjectReset(pParse);
}

void sqlite3AlterDropColumn(Parse *pParse, SrcList *pSrc, const Token *pName) {
  sqlite3 *db = pParse->db;
  Table *pTab;
  int iDb;
  const char *zDb;
  char *zCol = 0;
  int iCol;

  if ((db->mallocFailed))
    goto exit_drop_column;
  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  if (!pTab)
    goto exit_drop_column;

  if (SQLITE_OK != isAlterableTable(pParse, pTab))
    goto exit_drop_column;
  if (SQLITE_OK != isRealTable(pParse, pTab, 1))
    goto exit_drop_column;

  zCol = sqlite3NameFromToken(db, pName);
  if (zCol == 0) {
    goto exit_drop_column;
  }
  iCol = sqlite3ColumnIndex(pTab, zCol);
  if (iCol < 0) {
    sqlite3ErrorMsg(pParse, "no such column: \"%T\"", pName);
    goto exit_drop_column;
  }

  if (pTab->aCol[iCol].colFlags & (0x0001 | 0x0008)) {
    sqlite3ErrorMsg(pParse, "cannot drop %s column: \"%s\"",
                    (pTab->aCol[iCol].colFlags & 0x0001) ? "PRIMARY KEY" : "UNIQUE", zCol);
    goto exit_drop_column;
  }

  if (pTab->nCol <= 1) {
    sqlite3ErrorMsg(pParse, "cannot drop column \"%s\": no other columns exist", zCol);
    goto exit_drop_column;
  }

  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);

  zDb = db->aDb[iDb].zDbSName;

  if (sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, zCol)) {
    goto exit_drop_column;
  }

  renameTestSchema(pParse, zDb, iDb == 1, "", 0);
  renameFixQuotes(pParse, zDb, iDb == 1);
  sqlite3NestedParse(pParse,
                     "UPDATE \"%w\"."
                     "sqlite_master"
                     " SET "
                     "sql = sqlite_drop_column(%d, sql, %d) "
                     "WHERE (type=='table' AND tbl_name=%Q COLLATE nocase)",
                     zDb, iDb, iCol, pTab->zName);

  renameReloadSchema(pParse, iDb, 0x0002);
  renameTestSchema(pParse, zDb, iDb == 1, "after drop column", 1);

  if (pParse->nErr == 0 && (pTab->aCol[iCol].colFlags & 0x0020) == 0) {
    int i;
    int addr;
    int reg;
    int regRec;
    Index *pPk = 0;
    int nField = 0;
    int iCur;
    Vdbe *v = sqlite3GetVdbe(pParse);
    iCur = pParse->nTab++;
    sqlite3OpenTable(pParse, iCur, iDb, pTab, 116);
    addr = sqlite3VdbeAddOp1(v, 36, iCur);
    reg = ++pParse->nMem;
    if ((((pTab)->tabFlags & 0x00000080) == 0)) {
      sqlite3VdbeAddOp2(v, 137, iCur, reg);
      pParse->nMem += pTab->nCol;
    } else {
      pPk = sqlite3PrimaryKeyIndex(pTab);
      pParse->nMem += pPk->nColumn;
      for (i = 0; i < pPk->nKeyCol; i++) {
        sqlite3VdbeAddOp3(v, 96, iCur, i, reg + i + 1);
      }
      nField = pPk->nKeyCol;
    }
    regRec = ++pParse->nMem;
    for (i = 0; i < pTab->nCol; i++) {
      if (i != iCol && (pTab->aCol[i].colFlags & 0x0020) == 0) {
        int regOut;
        if (pPk) {
          int iPos = sqlite3TableColumnToIndex(pPk, i);
          int iColPos = sqlite3TableColumnToIndex(pPk, iCol);
          if (iPos < pPk->nKeyCol)
            continue;
          regOut = reg + 1 + iPos - (iPos > iColPos);
        } else {
          regOut = reg + 1 + nField;
        }
        if (i == pTab->iPKey) {
          sqlite3VdbeAddOp2(v, 77, 0, regOut);
        } else {
          char aff = pTab->aCol[i].affinity;
          if (aff == 0x45) {
            pTab->aCol[i].affinity = 0x43;
          }
          sqlite3ExprCodeGetColumnOfTable(v, pTab, iCur, i, regOut);
          pTab->aCol[i].affinity = aff;
        }
        nField++;
      }
    }
    if (nField == 0) {
      pParse->nMem++;
      sqlite3VdbeAddOp2(v, 77, 0, reg + 1);
      nField = 1;
    }
    sqlite3VdbeAddOp3(v, 99, reg + 1, nField, regRec);
    if (pPk) {
      sqlite3VdbeAddOp4Int(v, 140, iCur, regRec, reg + 1, pPk->nKeyCol);
    } else {
      sqlite3VdbeAddOp3(v, 130, iCur, regRec, reg);
    }
    sqlite3VdbeChangeP5(v, 0x02);

    sqlite3VdbeAddOp2(v, 40, iCur, addr + 1);
    sqlite3VdbeJumpHere(v, addr);
  }

exit_drop_column:
  sqlite3DbFree(db, zCol);
  sqlite3SrcListDelete(db, pSrc);
}

int alterFindCol(Parse *pParse, Table *pTab, Token *pCol, int *piCol) {
  sqlite3 *db = pParse->db;
  char *zName = sqlite3NameFromToken(db, pCol);
  int rc = SQLITE_NOMEM;
  int iCol = -1;

  if (zName) {
    iCol = sqlite3ColumnIndex(pTab, zName);
    if (iCol < 0) {
      sqlite3ErrorMsg(pParse, "no such column: %s", zName);
      rc = SQLITE_ERROR;
    } else {
      rc = SQLITE_OK;
    }
  }

  if (rc == SQLITE_OK) {
    const char *zDb = db->aDb[sqlite3SchemaToIndex(db, pTab->pSchema)].zDbSName;
    const char *zCol = pTab->aCol[iCol].zCnName;
    if (sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, zDb, pTab->zName, zCol)) {
      pTab = 0;
    }
  }

  sqlite3DbFree(db, zName);
  *piCol = iCol;
  return rc;
}

Table *alterFindTable(Parse *pParse, SrcList *pSrc, int *piDb, const char **pzDb, int bAuth) {
  sqlite3 *db = pParse->db;
  Table *pTab = 0;

  pTab = sqlite3LocateTableItem(pParse, 0, &pSrc->a[0]);
  if (pTab) {
    int iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    *pzDb = db->aDb[iDb].zDbSName;
    *piDb = iDb;

    if (SQLITE_OK != isRealTable(pParse, pTab, 2) || SQLITE_OK != isAlterableTable(pParse, pTab)) {
      pTab = 0;
    }
  }

  if (pTab && bAuth) {
    if (sqlite3AuthCheck(pParse, SQLITE_ALTER_TABLE, *pzDb, pTab->zName, 0)) {
      pTab = 0;
    }
  }

  sqlite3SrcListDelete(db, pSrc);
  return pTab;
}

void sqlite3AlterDropConstraint(Parse *pParse, SrcList *pSrc, Token *pCons, Token *pCol) {
  sqlite3 *db = pParse->db;
  Table *pTab = 0;
  int iDb = 0;
  const char *zDb = 0;
  char *zArg = 0;

  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, pCons != 0);
  if (!pTab)
    return;

  if (pCons) {
    char *z = sqlite3NameFromToken(db, pCons);
    zArg = sqlite3MPrintf(db, "%Q", z);
    sqlite3DbFree(db, z);
  } else {
    int iCol;
    if (alterFindCol(pParse, pTab, pCol, &iCol))
      return;
    zArg = sqlite3MPrintf(db, "%d", iCol);
  }

  sqlite3NestedParse(pParse,
                     "UPDATE \"%w\"."
                     "sqlite_master"
                     " SET "
                     "sql = sqlite_drop_constraint(sql, %s) "
                     "WHERE type='table' AND tbl_name=%Q COLLATE nocase",
                     zDb, zArg, pTab->zName);
  sqlite3DbFree(db, zArg);

  renameReloadSchema(pParse, iDb, 0x0004);
}

void sqlite3AlterSetNotNull(Parse *pParse, SrcList *pSrc, Token *pCol, Token *pFirst) {
  Table *pTab = 0;
  int iCol = 0;
  int iDb = 0;
  const char *zDb = 0;
  const char *pCons = 0;
  int nCons = 0;

  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, 0);
  if (!pTab)
    return;

  if (alterFindCol(pParse, pTab, pCol, &iCol)) {
    return;
  }

  pCons = pFirst->z;
  nCons = alterRtrimConstraint(pParse->db, pCons, pParse->sLastToken.z - pCons);

  sqlite3NestedParse(pParse,
                     "SELECT sqlite_fail('constraint failed', %d) "
                     "FROM %Q.%Q AS x WHERE x.%.*s IS NULL",
                     SQLITE_CONSTRAINT, zDb, pTab->zName, (int)pCol->n, pCol->z);

  sqlite3NestedParse(pParse,
                     "UPDATE \"%w\"."
                     "sqlite_master"
                     " SET "
                     "sql = sqlite_add_constraint(sqlite_drop_constraint(sql, %d), %.*Q, %d) "
                     "WHERE type='table' AND tbl_name=%Q COLLATE nocase",
                     zDb, iCol, nCons, pCons, iCol, pTab->zName);

  renameReloadSchema(pParse, iDb, 0x0004);
}

void sqlite3AlterAddConstraint(Parse *pParse, SrcList *pSrc, Token *pFirst, Token *pName, const char *zExpr, int nExpr,
                               Expr *pExpr) {
  Table *pTab = 0;
  int iDb = 0;
  const char *zDb = 0;
  const char *pCons = 0;
  int nCons;
  int rc;

  pTab = alterFindTable(pParse, pSrc, &iDb, &zDb, 1);
  if (!pTab) {
    sqlite3ExprDelete(pParse->db, pExpr);
    return;
  }

  rc = sqlite3ResolveSelfReference(pParse, pTab, 0x000004, pExpr, 0);
  sqlite3ExprDelete(pParse->db, pExpr);
  if (rc)
    return;

  if (pName) {
    char *zName = sqlite3NameFromToken(pParse->db, pName);

    sqlite3NestedParse(pParse,
                       "SELECT sqlite_fail('constraint %q already exists', %d) "
                       "FROM \"%w\"."
                       "sqlite_master"
                       " "
                       "WHERE type='table' AND tbl_name=%Q COLLATE nocase "
                       "AND sqlite_find_constraint(sql, %Q)",
                       zName, SQLITE_ERROR, zDb, pTab->zName, zName);
    sqlite3DbFree(pParse->db, zName);
  }

  sqlite3NestedParse(pParse,
                     "SELECT sqlite_fail('constraint failed', %d) "
                     "FROM %Q.%Q WHERE (%.*s) IS NOT TRUE",
                     SQLITE_CONSTRAINT, zDb, pTab->zName, nExpr, zExpr);

  pCons = pFirst->z;
  nCons = alterRtrimConstraint(pParse->db, pCons, pParse->sLastToken.z - pCons);

  sqlite3NestedParse(pParse,
                     "UPDATE \"%w\"."
                     "sqlite_master"
                     " SET "
                     "sql = sqlite_add_constraint(sql, %.*Q, -1) "
                     "WHERE type='table' AND tbl_name=%Q COLLATE nocase",
                     zDb, nCons, pCons, pTab->zName);

  renameReloadSchema(pParse, iDb, 0x0004);
}

void openStatTable(Parse *pParse, int iDb, int iStatCur, const char *zWhere, const char *zWhereType) {
  static const struct {
    const char *zName;
    const char *zCols;
  } aTable[] = {
      {"sqlite_stat1", "tbl,idx,stat"},
      {"sqlite_stat4", 0},
      {"sqlite_stat3", 0},
  };
  int i;
  sqlite3 *db = pParse->db;
  Db *pDb;
  Vdbe *v = sqlite3GetVdbe(pParse);
  u32 aRoot[((int)(sizeof(aTable) / sizeof(aTable[0])))];
  u8 aCreateTbl[((int)(sizeof(aTable) / sizeof(aTable[0])))];

  const int nToOpen = 1;

  if (v == 0)
    return;

  pDb = &db->aDb[iDb];

  for (i = 0; i < ((int)(sizeof(aTable) / sizeof(aTable[0]))); i++) {
    const char *zTab = aTable[i].zName;
    Table *pStat;
    aCreateTbl[i] = 0;
    if ((pStat = sqlite3FindTable(db, zTab, pDb->zDbSName)) == 0) {
      if (i < nToOpen) {
        sqlite3NestedParse(pParse, "CREATE TABLE %Q.%s(%s)", pDb->zDbSName, zTab, aTable[i].zCols);

        aRoot[i] = (u32)pParse->u1.cr.regRoot;
        aCreateTbl[i] = 0x10;
      }
    } else {
      aRoot[i] = pStat->tnum;
      sqlite3TableLock(pParse, iDb, aRoot[i], 1, zTab);
      if (zWhere) {
        sqlite3NestedParse(pParse, "DELETE FROM %Q.%s WHERE %s=%Q", pDb->zDbSName, zTab, zWhereType, zWhere);

      } else {
        sqlite3VdbeAddOp2(v, 147, (int)aRoot[i], iDb);
      }
    }
  }

  for (i = 0; i < nToOpen; i++) {
    sqlite3VdbeAddOp4Int(v, 116, iStatCur + i, (int)aRoot[i], iDb, 3);
    sqlite3VdbeChangeP5(v, aCreateTbl[i]);
  }
}

void callStatGet(Parse *pParse, int regStat, int iParam, int regOut) {
  (void)(iParam);

  sqlite3VdbeAddFunctionCall(pParse, 0, regStat, regOut, 1 + 0, &statGetFuncdef, 0);
}

void analyzeOneTable(Parse *pParse, Table *pTab, Index *pOnlyIdx, int iStatCur, int iMem, int iTab) {
  sqlite3 *db = pParse->db;
  Index *pIdx;
  int iIdxCur;
  int iTabCur;
  Vdbe *v;
  int i;
  int jZeroRows = -1;
  int iDb;
  u8 needTableCnt = 1;
  int regNewRowid = iMem++;
  int regStat = iMem++;
  int regChng = iMem++;
  int regRowid = iMem++;
  int regTemp = iMem++;
  int regTemp2 = iMem++;
  int regTabname = iMem++;
  int regIdxname = iMem++;
  int regStat1 = iMem++;
  int regPrev = iMem;

  sqlite3TouchRegister(pParse, iMem);

  v = sqlite3GetVdbe(pParse);
  if (v == 0 || (pTab == 0)) {
    return;
  }
  if (!((pTab)->eTabType == 0)) {
    return;
  }
  if (sqlite3_strlike("sqlite\\_%", pTab->zName, '\\') == 0) {
    return;
  }

  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);

  if (sqlite3AuthCheck(pParse, SQLITE_ANALYZE, pTab->zName, 0, db->aDb[iDb].zDbSName)) {
    return;
  }

  sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);
  iTabCur = iTab++;
  iIdxCur = iTab++;
  pParse->nTab = ((pParse->nTab) > (iTab) ? (pParse->nTab) : (iTab));
  sqlite3OpenTable(pParse, iTabCur, iDb, pTab, 114);
  sqlite3VdbeLoadString(v, regTabname, pTab->zName);

  for (pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext) {
    int nCol;
    int addrGotoEnd;
    int addrNextRow;
    const char *zIdxName;
    int nColTest;

    if (pOnlyIdx && pOnlyIdx != pIdx)
      continue;
    if (pIdx->pPartIdxWhere == 0)
      needTableCnt = 0;
    if (!(((pTab)->tabFlags & 0x00000080) == 0) && ((pIdx)->idxType == 2)) {
      nCol = pIdx->nKeyCol;
      zIdxName = pTab->zName;
      nColTest = nCol - 1;
    } else {
      nCol = pIdx->nColumn;
      zIdxName = pIdx->zName;
      nColTest = pIdx->uniqNotNull ? pIdx->nKeyCol - 1 : nCol - 1;
    }

    sqlite3VdbeLoadString(v, regIdxname, zIdxName);

    sqlite3TouchRegister(pParse, regPrev + nColTest);

    sqlite3VdbeAddOp3(v, 114, iIdxCur, pIdx->tnum, iDb);
    sqlite3VdbeSetP4KeyInfo(pParse, pIdx);

    sqlite3VdbeAddOp2(v, 73, db->nAnalysisLimit, regTemp2);

    sqlite3VdbeAddOp2(v, 73, nCol, regStat + 1);

    sqlite3VdbeAddOp2(v, 73, pIdx->nKeyCol, regRowid);
    sqlite3VdbeAddOp3(v, 100, iIdxCur, regTemp, (((db)->dbOptFlags & (0x00000800)) != 0));
    sqlite3VdbeAddFunctionCall(pParse, 0, regStat + 1, regStat, 4, &statInitFuncdef, 0);
    addrGotoEnd = sqlite3VdbeAddOp1(v, 36, iIdxCur);

    sqlite3VdbeAddOp2(v, 73, 0, regChng);
    addrNextRow = sqlite3VdbeCurrentAddr(v);

    if (nColTest > 0) {
      int endDistinctTest = sqlite3VdbeMakeLabel(pParse);
      int *aGotoChng;
      aGotoChng = (int*)(sqlite3DbMallocRawNN(db, sizeof(int) * nColTest));
      if (aGotoChng == 0)
        continue;

      sqlite3VdbeAddOp0(v, 9);
      addrNextRow = sqlite3VdbeCurrentAddr(v);
      if (nColTest == 1 && pIdx->nKeyCol == 1 && ((pIdx)->onError != 0)) {
        sqlite3VdbeAddOp2(v, 52, regPrev, endDistinctTest);
      }
      for (i = 0; i < nColTest; i++) {
        char *pColl = (char *)sqlite3LocateCollSeq(pParse, pIdx->azColl[i]);
        sqlite3VdbeAddOp2(v, 73, i, regChng);
        sqlite3VdbeAddOp3(v, 96, iIdxCur, i, regTemp);
        aGotoChng[i] = sqlite3VdbeAddOp4(v, 53, regTemp, 0, regPrev + i, pColl, (-2));
        sqlite3VdbeChangeP5(v, 0x80);
      }
      sqlite3VdbeAddOp2(v, 73, nColTest, regChng);
      sqlite3VdbeGoto(v, endDistinctTest);

      sqlite3VdbeJumpHere(v, addrNextRow - 1);
      for (i = 0; i < nColTest; i++) {
        sqlite3VdbeJumpHere(v, aGotoChng[i]);
        sqlite3VdbeAddOp3(v, 96, iIdxCur, i, regPrev + i);
      }
      sqlite3VdbeResolveLabel(v, endDistinctTest);
      sqlite3DbFree(db, aGotoChng);
    }

    {
      sqlite3VdbeAddFunctionCall(pParse, 1, regStat, regTemp, 2 + 0, &statPushFuncdef, 0);
      if (db->nAnalysisLimit) {
        int j1, j2, j3;
        j1 = sqlite3VdbeAddOp1(v, 51, regTemp);
        j2 = sqlite3VdbeAddOp1(v, 16, regTemp);
        j3 = sqlite3VdbeAddOp4Int(v, 24, iIdxCur, 0, regPrev, 1);
        sqlite3VdbeJumpHere(v, j1);
        sqlite3VdbeAddOp2(v, 40, iIdxCur, addrNextRow);
        sqlite3VdbeJumpHere(v, j2);
        sqlite3VdbeJumpHere(v, j3);
      } else {
        sqlite3VdbeAddOp2(v, 40, iIdxCur, addrNextRow);
      }
    }

    if (pIdx->pPartIdxWhere) {
      sqlite3VdbeJumpHere(v, addrGotoEnd);
      addrGotoEnd = 0;
    }
    callStatGet(pParse, regStat, 0, regStat1);

    sqlite3VdbeAddOp4(v, 99, regTabname, 3, regTemp, "BBB", 0);
    sqlite3VdbeAddOp2(v, 129, iStatCur, regNewRowid);
    sqlite3VdbeAddOp3(v, 130, iStatCur, regTemp, regNewRowid);

    sqlite3VdbeChangeP5(v, 0x08);

    if (addrGotoEnd)
      sqlite3VdbeJumpHere(v, addrGotoEnd);
  }

  if (pOnlyIdx == 0 && needTableCnt) {
    sqlite3VdbeAddOp2(v, 100, iTabCur, regStat1);
    jZeroRows = sqlite3VdbeAddOp1(v, 17, regStat1);
    sqlite3VdbeAddOp2(v, 77, 0, regIdxname);

    sqlite3VdbeAddOp4(v, 99, regTabname, 3, regTemp, "BBB", 0);
    sqlite3VdbeAddOp2(v, 129, iStatCur, regNewRowid);
    sqlite3VdbeAddOp3(v, 130, iStatCur, regTemp, regNewRowid);
    sqlite3VdbeChangeP5(v, 0x08);

    sqlite3VdbeJumpHere(v, jZeroRows);
  }
}

void loadAnalysis(Parse *pParse, int iDb) {
  Vdbe *v = sqlite3GetVdbe(pParse);
  if (v) {
    sqlite3VdbeAddOp1(v, 152, iDb);
  }
}

void analyzeDatabase(Parse *pParse, int iDb) {
  sqlite3 *db = pParse->db;
  Schema *pSchema = db->aDb[iDb].pSchema;
  HashElem *k;
  int iStatCur;
  int iMem;
  int iTab;

  sqlite3BeginWriteOperation(pParse, 0, iDb);
  iStatCur = pParse->nTab;
  pParse->nTab += 3;
  openStatTable(pParse, iDb, iStatCur, 0, 0);
  iMem = pParse->nMem + 1;
  iTab = pParse->nTab;

  for (k = ((&pSchema->tblHash)->first); k; k = ((k)->next)) {
    Table *pTab = (Table *)((k)->data);
    analyzeOneTable(pParse, pTab, 0, iStatCur, iMem, iTab);
  }
  loadAnalysis(pParse, iDb);
}

void analyzeTable(Parse *pParse, Table *pTab, Index *pOnlyIdx) {
  int iDb;
  int iStatCur;

  iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
  sqlite3BeginWriteOperation(pParse, 0, iDb);
  iStatCur = pParse->nTab;
  pParse->nTab += 3;
  if (pOnlyIdx) {
    openStatTable(pParse, iDb, iStatCur, pOnlyIdx->zName, "idx");
  } else {
    openStatTable(pParse, iDb, iStatCur, pTab->zName, "tbl");
  }
  analyzeOneTable(pParse, pTab, pOnlyIdx, iStatCur, pParse->nMem + 1, pParse->nTab);
  loadAnalysis(pParse, iDb);
}

void sqlite3Analyze(Parse *pParse, Token *pName1, Token *pName2) {
  sqlite3 *db = pParse->db;
  int iDb;
  int i;
  char *z, *zDb;
  Table *pTab;
  Index *pIdx;
  Token *pTableName;
  Vdbe *v;

  if (SQLITE_OK != sqlite3ReadSchema(pParse)) {
    return;
  }

  if (pName1 == 0) {
    for (i = 0; i < db->nDb; i++) {
      if (i == 1)
        continue;
      analyzeDatabase(pParse, i);
    }
  } else if (pName2->n == 0 && (iDb = sqlite3FindDb(db, pName1)) >= 0) {
    analyzeDatabase(pParse, iDb);
  } else {
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pTableName);
    if (iDb >= 0) {
      zDb = pName2->n ? db->aDb[iDb].zDbSName : 0;
      z = sqlite3NameFromToken(db, pTableName);
      if (z) {
        if ((pIdx = sqlite3FindIndex(db, z, zDb)) != 0) {
          analyzeTable(pParse, pIdx->pTable, pIdx);
        } else if ((pTab = sqlite3LocateTable(pParse, 0, z, zDb)) != 0) {
          analyzeTable(pParse, pTab, 0);
        }
        sqlite3DbFree(db, z);
      }
    }
  }
  if (db->nSqlExec == 0 && (v = sqlite3GetVdbe(pParse)) != 0) {
    sqlite3VdbeAddOp0(v, 168);
  }
}

void codeAttach(Parse *pParse, int type, FuncDef const *pFunc, Expr *pAuthArg, Expr *pFilename, Expr *pDbname,
                Expr *pKey) {
  int rc;
  NameContext sName;
  Vdbe *v;
  sqlite3 *db = pParse->db;
  int regArgs;

  if (SQLITE_OK != sqlite3ReadSchema(pParse))
    goto attach_end;

  if (pParse->nErr)
    goto attach_end;
  memset(&sName, 0, sizeof(NameContext));
  sName.pParse = pParse;

  if (SQLITE_OK != resolveAttachExpr(&sName, pFilename) || SQLITE_OK != resolveAttachExpr(&sName, pDbname) ||
      SQLITE_OK != resolveAttachExpr(&sName, pKey)) {
    goto attach_end;
  }

  if ((pAuthArg)) {
    char *zAuthArg;
    if (pAuthArg->op == 118) {
      zAuthArg = pAuthArg->u.zToken;
    } else {
      zAuthArg = 0;
    }
    rc = sqlite3AuthCheck(pParse, type, zAuthArg, 0, 0);
    if (rc != SQLITE_OK) {
      goto attach_end;
    }
  }

  v = sqlite3GetVdbe(pParse);
  regArgs = sqlite3GetTempRange(pParse, 4);
  sqlite3ExprCode(pParse, pFilename, regArgs);
  sqlite3ExprCode(pParse, pDbname, regArgs + 1);
  sqlite3ExprCode(pParse, pKey, regArgs + 2);

  if (v) {
    sqlite3VdbeAddFunctionCall(pParse, 0, regArgs + 3 - pFunc->nArg, regArgs + 3, pFunc->nArg, pFunc, 0);

    sqlite3VdbeAddOp1(v, 168, (type == SQLITE_ATTACH));
  }

attach_end:
  sqlite3ExprDelete(db, pFilename);
  sqlite3ExprDelete(db, pDbname);
  sqlite3ExprDelete(db, pKey);
}

void sqlite3Detach(Parse *pParse, Expr *pDbname) {
  static const FuncDef detach_func = {1, 1, 0, 0, detachFunc, 0, 0, 0, "sqlite_detach", {0}};
  codeAttach(pParse, SQLITE_DETACH, &detach_func, pDbname, 0, 0, pDbname);
}

void sqlite3Attach(Parse *pParse, Expr *p, Expr *pDbname, Expr *pKey) {
  static const FuncDef attach_func = {3, 1, 0, 0, attachFunc, 0, 0, 0, "sqlite_attach", {0}};
  codeAttach(pParse, SQLITE_ATTACH, &attach_func, p, p, pDbname, pKey);
}

void sqliteAuthBadReturnCode(Parse *pParse) {
  sqlite3ErrorMsg(pParse, "authorizer malfunction");
  pParse->rc = SQLITE_ERROR;
}

int sqlite3AuthReadCol(Parse *pParse, const char *zTab, const char *zCol, int iDb) {
  sqlite3 *db = pParse->db;
  char *zDb = db->aDb[iDb].zDbSName;
  int rc;

  if (db->init.busy)
    return SQLITE_OK;
  rc = db->xAuth(db->pAuthArg, SQLITE_READ, zTab, zCol, zDb, pParse->zAuthContext);
  if (rc == SQLITE_DENY) {
    char *z = sqlite3_mprintf("%s.%s", zTab, zCol);
    if (db->nDb > 2 || iDb != 0)
      z = sqlite3_mprintf("%s.%z", zDb, z);
    sqlite3ErrorMsg(pParse, "access to %z is prohibited", z);
    pParse->rc = SQLITE_AUTH;
  } else if (rc != SQLITE_IGNORE && rc != SQLITE_OK) {
    sqliteAuthBadReturnCode(pParse);
  }
  return rc;
}

void sqlite3AuthRead(Parse *pParse, Expr *pExpr, Schema *pSchema, SrcList *pTabList) {
  Table *pTab = 0;
  const char *zCol;
  int iSrc;
  int iDb;
  int iCol;

  iDb = sqlite3SchemaToIndex(pParse->db, pSchema);
  if (iDb < 0) {
    return;
  }

  if (pExpr->op == 78) {
    pTab = pParse->pTriggerTab;
  } else {
    for (iSrc = 0; iSrc < pTabList->nSrc; iSrc++) {
      if (pExpr->iTable == pTabList->a[iSrc].iCursor) {
        pTab = pTabList->a[iSrc].pSTab;
        break;
      }
    }
  }
  iCol = pExpr->iColumn;
  if (pTab == 0)
    return;

  if (iCol >= 0) {
    zCol = pTab->aCol[iCol].zCnName;
  } else if (pTab->iPKey >= 0) {
    zCol = pTab->aCol[pTab->iPKey].zCnName;
  } else {
    zCol = "ROWID";
  }

  if (SQLITE_IGNORE == sqlite3AuthReadCol(pParse, pTab->zName, zCol, iDb)) {
    pExpr->op = 122;
  }
}

int sqlite3AuthCheck(Parse *pParse, int code, const char *zArg1, const char *zArg2, const char *zArg3) {
  sqlite3 *db = pParse->db;
  int rc;

  if (db->xAuth == 0 || db->init.busy || (pParse->eParseMode != 0)) {
    return SQLITE_OK;
  }

  rc = db->xAuth(db->pAuthArg, code, zArg1, zArg2, zArg3, pParse->zAuthContext);
  if (rc == SQLITE_DENY) {
    sqlite3ErrorMsg(pParse, "not authorized");
    pParse->rc = SQLITE_AUTH;
  } else if (rc != SQLITE_OK && rc != SQLITE_IGNORE) {
    rc = SQLITE_DENY;
    sqliteAuthBadReturnCode(pParse);
  }
  return rc;
}

void sqlite3AuthContextPush(Parse *pParse, AuthContext *pContext, const char *zContext) {
  pContext->pParse = pParse;
  pContext->zAuthContext = pParse->zAuthContext;
  pParse->zAuthContext = zContext;
}

__attribute__((noinline)) void lockTable(Parse *pParse, int iDb, Pgno iTab, u8 isWriteLock, const char *zName) {
  Parse *pToplevel;
  int i;
  int nBytes;
  TableLock *p;

  pToplevel = ((pParse)->pToplevel ? (pParse)->pToplevel : (pParse));
  for (i = 0; i < pToplevel->nTableLock; i++) {
    p = &pToplevel->aTableLock[i];
    if (p->iDb == iDb && p->iTab == iTab) {
      p->isWriteLock = (p->isWriteLock || isWriteLock);
      return;
    }
  }

  nBytes = sizeof(TableLock) * (pToplevel->nTableLock + 1);
  pToplevel->aTableLock = (TableLock*)(sqlite3DbReallocOrFree(pToplevel->db, pToplevel->aTableLock, nBytes));
  if (pToplevel->aTableLock) {
    p = &pToplevel->aTableLock[pToplevel->nTableLock++];
    p->iDb = iDb;
    p->iTab = iTab;
    p->isWriteLock = isWriteLock;
    p->zLockName = zName;
  } else {
    pToplevel->nTableLock = 0;
    sqlite3OomFault(pToplevel->db);
  }
}

void sqlite3TableLock(Parse *pParse, int iDb, Pgno iTab, u8 isWriteLock, const char *zName) {
  if (iDb == 1)
    return;
  if (!sqlite3BtreeSharable(pParse->db->aDb[iDb].pBt))
    return;
  lockTable(pParse, iDb, iTab, isWriteLock, zName);
}

void codeTableLocks(Parse *pParse) {
  int i;
  Vdbe *pVdbe = pParse->pVdbe;

  for (i = 0; i < pParse->nTableLock; i++) {
    TableLock *p = &pParse->aTableLock[i];
    int p1 = p->iDb;
    sqlite3VdbeAddOp4(pVdbe, 171, p1, p->iTab, p->isWriteLock, p->zLockName, (-1));
  }
}

void sqlite3FinishCoding(Parse *pParse) {
  sqlite3 *db;
  Vdbe *v;
  int iDb, i;

  db = pParse->db;

  if (pParse->nested)
    return;
  if (pParse->nErr) {
    if (db->mallocFailed)
      pParse->rc = SQLITE_NOMEM;
    return;
  }

  v = pParse->pVdbe;
  if (v == 0) {
    if (db->init.busy) {
      pParse->rc = SQLITE_DONE;
      return;
    }
    v = sqlite3GetVdbe(pParse);
    if (v == 0)
      pParse->rc = SQLITE_ERROR;
  }

  if (v) {
    if (pParse->bReturning) {
      Returning *pReturning;
      int addrRewind;
      int reg;

      pReturning = pParse->u1.d.pReturning;
      if (pReturning->nRetCol) {
        sqlite3VdbeAddOp0(v, 85);
        addrRewind = sqlite3VdbeAddOp1(v, 36, pReturning->iRetCur);
        reg = pReturning->iRetReg;
        for (i = 0; i < pReturning->nRetCol; i++) {
          sqlite3VdbeAddOp3(v, 96, pReturning->iRetCur, i, reg + i);
        }
        sqlite3VdbeAddOp2(v, 86, reg, i);
        sqlite3VdbeAddOp2(v, 40, pReturning->iRetCur, addrRewind + 1);
        sqlite3VdbeJumpHere(v, addrRewind);
      }
    }
    sqlite3VdbeAddOp0(v, 72);

    sqlite3VdbeJumpHere(v, 0);

    iDb = 0;
    do {
      Schema *pSchema;
      if ((((pParse->cookieMask) & (((yDbMask)1) << (iDb))) != 0) == 0)
        continue;
      sqlite3VdbeUsesBtree(v, iDb);
      pSchema = db->aDb[iDb].pSchema;
      sqlite3VdbeAddOp4Int(v, 2, iDb, (((pParse->writeMask) & (((yDbMask)1) << (iDb))) != 0), pSchema->schema_cookie,
                           pSchema->iGeneration);
      if (db->init.busy == 0)
        sqlite3VdbeChangeP5(v, 1);

    } while (++iDb < db->nDb);

    for (i = 0; i < pParse->nVtabLock; i++) {
      char *vtab = (char *)sqlite3GetVTable(db, pParse->apVtabLock[i]);
      sqlite3VdbeAddOp4(v, 172, 0, 0, 0, vtab, (-12));
    }
    pParse->nVtabLock = 0;

    if (pParse->nTableLock)
      codeTableLocks(pParse);

    if (pParse->pAinc)
      sqlite3AutoincrementBegin(pParse);

    if (pParse->pConstExpr) {
      ExprList *pEL = pParse->pConstExpr;
      pParse->okConstFactor = 0;
      for (i = 0; i < pEL->nExpr; i++) {
        sqlite3ExprCode(pParse, pEL->a[i].pExpr, pEL->a[i].u.iConstExprReg);
      }
    }

    if (pParse->bReturning) {
      Returning *pRet;

      pRet = pParse->u1.d.pReturning;
      if (pRet->nRetCol) {
        sqlite3VdbeAddOp2(v, 120, pRet->iRetCur, pRet->nRetCol);
      }
    }

    sqlite3VdbeGoto(v, 1);
  }

  if (pParse->nErr == 0) {
    sqlite3VdbeMakeReady(v, pParse);
    pParse->rc = SQLITE_DONE;
  } else {
    pParse->rc = SQLITE_ERROR;
  }
}

void sqlite3NestedParse(Parse *pParse, const char *zFormat, ...) {
  va_list ap;
  char *zSql;
  sqlite3 *db = pParse->db;
  u32 savedDbFlags = db->mDbFlags;
  char saveBuf[(sizeof(Parse) - offsetof(Parse, sLastToken))];

  if (pParse->nErr)
    return;
  if (pParse->eParseMode)
    return;

  va_start(ap, zFormat);
  zSql = sqlite3VMPrintf(db, zFormat, ap);

  va_end(ap);
  if (zSql == 0) {
    if (!db->mallocFailed)
      pParse->rc = SQLITE_TOOBIG;
    pParse->nErr++;
    return;
  }
  pParse->nested++;
  memcpy(saveBuf, (((char *)(pParse)) + offsetof(Parse, sLastToken)), (sizeof(Parse) - offsetof(Parse, sLastToken)));
  memset((((char *)(pParse)) + offsetof(Parse, sLastToken)), 0, (sizeof(Parse) - offsetof(Parse, sLastToken)));
  db->mDbFlags |= 0x0002;
  sqlite3RunParser(pParse, zSql);
  db->mDbFlags = savedDbFlags;
  sqlite3DbFree(db, zSql);
  memcpy((((char *)(pParse)) + offsetof(Parse, sLastToken)), saveBuf, (sizeof(Parse) - offsetof(Parse, sLastToken)));
  pParse->nested--;
}

Table *sqlite3LocateTable(Parse *pParse, u32 flags, const char *zName, const char *zDbase) {
  Table *p;
  sqlite3 *db = pParse->db;

  if ((db->mDbFlags & 0x0010) == 0 && SQLITE_OK != sqlite3ReadSchema(pParse)) {
    return 0;
  }

  p = sqlite3FindTable(db, zName, zDbase);
  if (p == 0) {
    if ((pParse->prepFlags & SQLITE_PREPARE_NO_VTAB) == 0 && db->init.busy == 0) {
      Module *pMod = (Module *)sqlite3HashFind(&db->aModule, zName);
      if (pMod == 0 && sqlite3_strnicmp(zName, "pragma_", 7) == 0) {
        pMod = sqlite3PragmaVtabRegister(db, zName);
      }

      if (pMod == 0 && sqlite3_strnicmp(zName, "json", 4) == 0) {
        pMod = sqlite3JsonVtabRegister(db, zName);
      }

      if (pMod && sqlite3VtabEponymousTableInit(pParse, pMod)) {
        return pMod->pEpoTab;
      }
    }

    if (flags & 0x02)
      return 0;
    pParse->checkSchema = 1;
  } else if (((p)->eTabType == 1) && (pParse->prepFlags & SQLITE_PREPARE_NO_VTAB) != 0) {
    p = 0;
  }

  if (p == 0) {
    const char *zMsg = flags & 0x01 ? "no such view" : "no such table";
    if (zDbase) {
      sqlite3ErrorMsg(pParse, "%s: %s.%s", zMsg, zDbase, zName);
    } else {
      sqlite3ErrorMsg(pParse, "%s: %s", zMsg, zName);
    }
  } else {
  }

  return p;
}

Table *sqlite3LocateTableItem(Parse *pParse, u32 flags, SrcItem *p) {
  const char *zDb;
  if (p->fg.fixedSchema) {
    int iDb = sqlite3SchemaToIndex(pParse->db, p->u4.pSchema);

    zDb = pParse->db->aDb[iDb].zDbSName;
  } else {
    zDb = p->u4.zDatabase;
  }
  return sqlite3LocateTable(pParse, flags, p->zName, zDb);
}

void sqlite3ColumnSetExpr(Parse *pParse, Table *pTab, Column *pCol, Expr *pExpr) {
  ExprList *pList;

  pList = pTab->u.tab.pDfltList;
  if (pCol->iDflt == 0 || (pList == 0) || (pList->nExpr < pCol->iDflt)) {
    pCol->iDflt = pList == 0 ? 1 : pList->nExpr + 1;
    pTab->u.tab.pDfltList = sqlite3ExprListAppend(pParse, pList, pExpr);
  } else {
    sqlite3ExprDelete(pParse->db, pList->a[pCol->iDflt - 1].pExpr);
    pList->a[pCol->iDflt - 1].pExpr = pExpr;
  }
}

void sqlite3OpenSchemaTable(Parse *p, int iDb) {
  Vdbe *v = sqlite3GetVdbe(p);
  sqlite3TableLock(p, iDb, 1, 1, "sqlite_master");
  sqlite3VdbeAddOp4Int(v, 116, 0, 1, iDb, 5);
  if (p->nTab == 0) {
    p->nTab = 1;
  }
}

int sqlite3TwoPartName(Parse *pParse, Token *pName1, Token *pName2, Token **pUnqual) {
  int iDb;
  sqlite3 *db = pParse->db;

  if (pName2->n > 0) {
    if (db->init.busy) {
      sqlite3ErrorMsg(pParse, "corrupt database");
      return -1;
    }
    *pUnqual = pName2;
    iDb = sqlite3FindDb(db, pName1);
    if (iDb < 0) {
      sqlite3ErrorMsg(pParse, "unknown database %T", pName1);
      return -1;
    }
  } else {
    iDb = db->init.iDb;
    *pUnqual = pName1;
  }
  return iDb;
}

int sqlite3CheckObjectName(Parse *pParse, const char *zName, const char *zType, const char *zTblName) {
  sqlite3 *db = pParse->db;
  if (sqlite3WritableSchema(db) || db->init.imposterTable || !sqlite3Config.bExtraSchemaChecks) {
    return SQLITE_OK;
  }
  if (db->init.busy) {
    if (sqlite3_stricmp(zType, db->init.azInit[0]) || sqlite3_stricmp(zName, db->init.azInit[1]) ||
        sqlite3_stricmp(zTblName, db->init.azInit[2])) {
      sqlite3ErrorMsg(pParse, "");
      return SQLITE_ERROR;
    }
  } else {
    if ((pParse->nested == 0 && 0 == sqlite3_strnicmp(zName, "sqlite_", 7)) ||
        (sqlite3ReadOnlyShadowTables(db) && sqlite3ShadowTableName(db, zName))) {
      sqlite3ErrorMsg(pParse, "object name reserved for internal use: %s", zName);
      return SQLITE_ERROR;
    }
  }
  return SQLITE_OK;
}

void sqlite3ForceNotReadOnly(Parse *pParse) {
  int iReg = ++pParse->nMem;
  Vdbe *v = sqlite3GetVdbe(pParse);
  if (v) {
    sqlite3VdbeAddOp3(v, 4, 0, iReg, (-1));
    sqlite3VdbeUsesBtree(v, 0);
  }
}

void sqlite3StartTable(Parse *pParse, Token *pName1, Token *pName2, int isTemp, int isView, int isVirtual, int noErr) {
  Table *pTable;
  char *zName = 0;
  sqlite3 *db = pParse->db;
  Vdbe *v;
  int iDb;
  Token *pName;

  if (db->init.busy && db->init.newTnum == 1) {
    iDb = db->init.iDb;
    zName = sqlite3DbStrDup(db, ((!0) && (iDb == 1) ? "sqlite_temp_master" : "sqlite_master"));
    pName = pName1;
  } else {
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pName);
    if (iDb < 0)
      return;
    if (!0 && isTemp && pName2->n > 0 && iDb != 1) {
      sqlite3ErrorMsg(pParse, "temporary table name must be unqualified");
      return;
    }
    if (!0 && isTemp)
      iDb = 1;
    zName = sqlite3NameFromToken(db, pName);
    if ((pParse->eParseMode >= 2)) {
      sqlite3RenameTokenMap(pParse, (void *)zName, pName);
    }
  }
  pParse->sNameToken = *pName;
  if (zName == 0)
    return;
  if (sqlite3CheckObjectName(pParse, zName, isView ? "view" : "table", zName)) {
    goto begin_table_error;
  }
  if (db->init.iDb == 1)
    isTemp = 1;

  {
    static const u8 aCode[] = {SQLITE_CREATE_TABLE, SQLITE_CREATE_TEMP_TABLE, SQLITE_CREATE_VIEW,
                               SQLITE_CREATE_TEMP_VIEW};
    char *zDb = db->aDb[iDb].zDbSName;
    if (sqlite3AuthCheck(pParse, SQLITE_INSERT, ((!0) && (isTemp == 1) ? "sqlite_temp_master" : "sqlite_master"), 0,
                         zDb)) {
      goto begin_table_error;
    }
    if (!isVirtual && sqlite3AuthCheck(pParse, (int)aCode[isTemp + 2 * isView], zName, 0, zDb)) {
      goto begin_table_error;
    }
  }

  if (!(pParse->eParseMode != 0)) {
    char *zDb = db->aDb[iDb].zDbSName;
    if (SQLITE_OK != sqlite3ReadSchema(pParse)) {
      goto begin_table_error;
    }
    pTable = sqlite3FindTable(db, zName, zDb);
    if (pTable) {
      if (!noErr) {
        sqlite3ErrorMsg(pParse, "%s %T already exists", (((pTable)->eTabType == 2) ? "view" : "table"), pName);
      } else {
        sqlite3CodeVerifySchema(pParse, iDb);
        sqlite3ForceNotReadOnly(pParse);
      }
      goto begin_table_error;
    }
    if (sqlite3FindIndex(db, zName, zDb) != 0) {
      sqlite3ErrorMsg(pParse, "there is already an index named %s", zName);
      goto begin_table_error;
    }
  }

  pTable = (Table*)(sqlite3DbMallocZero(db, sizeof(Table)));
  if (pTable == 0) {
    pParse->rc = 7;
    pParse->nErr++;
    goto begin_table_error;
  }
  pTable->zName = zName;
  pTable->iPKey = -1;
  pTable->pSchema = db->aDb[iDb].pSchema;
  pTable->nTabRef = 1;

  pTable->nRowLogEst = 200;

  pParse->pNewTable = pTable;

  if (!db->init.busy && (v = sqlite3GetVdbe(pParse)) != 0) {
    int addr1;
    int fileFormat;
    int reg1, reg2, reg3;

    static const char nullRow[] = {6, 0, 0, 0, 0, 0};
    sqlite3BeginWriteOperation(pParse, 1, iDb);

    if (isVirtual) {
      sqlite3VdbeAddOp0(v, 172);
    }

    reg1 = pParse->u1.cr.regRowid = ++pParse->nMem;
    reg2 = pParse->u1.cr.regRoot = ++pParse->nMem;
    reg3 = ++pParse->nMem;
    sqlite3VdbeAddOp3(v, 101, iDb, reg3, 2);
    sqlite3VdbeUsesBtree(v, iDb);
    addr1 = sqlite3VdbeAddOp1(v, 16, reg3);
    fileFormat = (db->flags & 0x00000002) != 0 ? 1 : 4;
    sqlite3VdbeAddOp3(v, 102, iDb, 2, fileFormat);
    sqlite3VdbeAddOp3(v, 102, iDb, 5, ((db)->enc));
    sqlite3VdbeJumpHere(v, addr1);

    if (isView || isVirtual) {
      sqlite3VdbeAddOp2(v, 73, 0, reg2);
    } else {
      pParse->u1.cr.addrCrTab = sqlite3VdbeAddOp3(v, 149, iDb, reg2, 1);
    }
    sqlite3OpenSchemaTable(pParse, iDb);
    sqlite3VdbeAddOp2(v, 129, 0, reg1);
    sqlite3VdbeAddOp4(v, 79, 6, reg3, 0, nullRow, (-1));
    sqlite3VdbeAddOp3(v, 130, 0, reg3, reg1);
    sqlite3VdbeChangeP5(v, 0x08);
    sqlite3VdbeAddOp0(v, 124);
  } else if (db->init.imposterTable) {
    pTable->tabFlags |= 0x00020000;
    if (db->init.imposterTable >= 2)
      pTable->tabFlags |= 0x00000001;
  }

  return;

begin_table_error:
  pParse->checkSchema = 1;
  sqlite3DbFree(db, zName);
  return;
}

void sqlite3AddReturning(Parse *pParse, ExprList *pList) {
  Returning *pRet;
  Hash *pHash;
  sqlite3 *db = pParse->db;
  if (pParse->pNewTrigger) {
    sqlite3ErrorMsg(pParse, "cannot use RETURNING in a trigger");
  } else {
  }
  pParse->bReturning = 1;
  pRet = (Returning*)(sqlite3DbMallocZero(db, sizeof(*pRet)));
  if (pRet == 0) {
    sqlite3ExprListDelete(db, pList);
    return;
  }

  pParse->u1.d.pReturning = pRet;
  pRet->pParse = pParse;
  pRet->pReturnEL = pList;
  sqlite3ParserAddCleanup(pParse, sqlite3DeleteReturning, pRet);
  if (db->mallocFailed)
    return;
  sqlite3_snprintf(sizeof(pRet->zName), pRet->zName, "sqlite_returning_%p", pParse);
  pRet->retTrig.zName = pRet->zName;
  pRet->retTrig.op = 151;
  pRet->retTrig.tr_tm = 2;
  pRet->retTrig.bReturning = 1;
  pRet->retTrig.pSchema = db->aDb[1].pSchema;
  pRet->retTrig.pTabSchema = db->aDb[1].pSchema;
  pRet->retTrig.step_list = &pRet->retTStep;
  pRet->retTStep.op = 151;
  pRet->retTStep.pTrig = &pRet->retTrig;
  pRet->retTStep.pExprList = pList;
  pHash = &(db->aDb[1].pSchema->trigHash);

  if (sqlite3HashInsert(pHash, pRet->zName, &pRet->retTrig) == &pRet->retTrig) {
    sqlite3OomFault(db);
  }
}

void sqlite3AddColumn(Parse *pParse, Token sName, Token sType) {
  Table *p;
  int i;
  char *z;
  char *zType;
  Column *pCol;
  sqlite3 *db = pParse->db;
  Column *aNew;
  u8 eType = 0;
  u8 szEst = 1;
  char affinity = 0x41;

  if ((p = pParse->pNewTable) == 0)
    return;
  if (p->nCol + 1 > db->aLimit[SQLITE_LIMIT_COLUMN]) {
    sqlite3ErrorMsg(pParse, "too many columns on %s", p->zName);
    return;
  }
  if (!(pParse->eParseMode >= 2))
    sqlite3DequoteToken(&sName);

  if (sType.n >= 16 && sqlite3_strnicmp(sType.z + (sType.n - 6), "always", 6) == 0) {
    sType.n -= 6;
    while ((sType.n > 0) && (sqlite3CtypeMap[(unsigned char)(sType.z[sType.n - 1])] & 0x01))
      sType.n--;
    if (sType.n >= 9 && sqlite3_strnicmp(sType.z + (sType.n - 9), "generated", 9) == 0) {
      sType.n -= 9;
      while (sType.n > 0 && (sqlite3CtypeMap[(unsigned char)(sType.z[sType.n - 1])] & 0x01))
        sType.n--;
    }
  }

  if (sType.n >= 3) {
    sqlite3DequoteToken(&sType);
    for (i = 0; i < 6; i++) {
      if (sType.n == sqlite3StdTypeLen[i] && sqlite3_strnicmp(sType.z, sqlite3StdType[i], sType.n) == 0) {
        sType.n = 0;
        eType = i + 1;
        affinity = sqlite3StdTypeAffinity[i];
        if (affinity <= 0x42)
          szEst = 5;
        break;
      }
    }
  }

  z = (char*)(sqlite3DbMallocRaw(db, (i64)sName.n + 1 + (i64)sType.n + (sType.n > 0)));
  if (z == 0)
    return;
  if ((pParse->eParseMode >= 2))
    sqlite3RenameTokenMap(pParse, (void *)z, &sName);
  memcpy(z, sName.z, sName.n);
  z[sName.n] = 0;
  sqlite3Dequote(z);
  if (p->nCol && sqlite3ColumnIndex(p, z) >= 0) {
    sqlite3ErrorMsg(pParse, "duplicate column name: %s", z);
    sqlite3DbFree(db, z);
    return;
  }
  aNew = (Column*)(sqlite3DbRealloc(db, p->aCol, ((i64)p->nCol + 1) * sizeof(p->aCol[0])));
  if (aNew == 0) {
    sqlite3DbFree(db, z);
    return;
  }
  p->aCol = aNew;
  pCol = &p->aCol[p->nCol];
  memset(pCol, 0, sizeof(p->aCol[0]));
  pCol->zCnName = z;
  pCol->hName = sqlite3StrIHash(z);

  if (sType.n == 0) {
    pCol->affinity = affinity;
    pCol->eCType = eType;
    pCol->szEst = szEst;

  } else {
    zType = z + sqlite3Strlen30(z) + 1;
    memcpy(zType, sType.z, sType.n);
    zType[sType.n] = 0;
    sqlite3Dequote(zType);
    pCol->affinity = sqlite3AffinityType(zType, pCol);
    pCol->colFlags |= 0x0004;
  }
  if (p->nCol <= 0xff) {
    u8 h = pCol->hName % sizeof(p->aHx);
    p->aHx[h] = p->nCol;
  }
  p->nCol++;
  p->nNVCol++;

  pParse->u1.cr.constraintName.n = 0;
}

void sqlite3AddNotNull(Parse *pParse, int onError) {
  Table *p;
  Column *pCol;
  p = pParse->pNewTable;
  if (p == 0 || (p->nCol < 1))
    return;
  pCol = &p->aCol[p->nCol - 1];
  pCol->notNull = (u8)onError;
  p->tabFlags |= 0x00000800;

  if (pCol->colFlags & 0x0008) {
    Index *pIdx;
    for (pIdx = p->pIndex; pIdx; pIdx = pIdx->pNext) {
      if (pIdx->aiColumn[0] == p->nCol - 1) {
        pIdx->uniqNotNull = 1;
      }
    }
  }
}

void sqlite3AddDefaultValue(Parse *pParse, Expr *pExpr, const char *zStart, const char *zEnd) {
  Table *p;
  Column *pCol;
  sqlite3 *db = pParse->db;
  p = pParse->pNewTable;
  if (p != 0) {
    int isInit = db->init.busy && db->init.iDb != 1;
    pCol = &(p->aCol[p->nCol - 1]);
    if (!sqlite3ExprIsConstantOrFunction(pExpr, isInit)) {
      sqlite3ErrorMsg(pParse, "default value of column [%s] is not constant", pCol->zCnName);

    } else if (pCol->colFlags & 0x0060) {
      sqlite3ErrorMsg(pParse, "cannot use DEFAULT on a generated column");

    } else {
      Expr x, *pDfltExpr;
      memset(&x, 0, sizeof(x));
      x.op = 181;
      x.u.zToken = sqlite3DbSpanDup(db, zStart, zEnd);
      x.pLeft = pExpr;
      x.flags = 0x002000;
      pDfltExpr = sqlite3ExprDup(db, &x, 0x0001);
      sqlite3DbFree(db, x.u.zToken);
      sqlite3ColumnSetExpr(pParse, p, pCol, pDfltExpr);
    }
  }
  if ((pParse->eParseMode >= 2)) {
    sqlite3RenameExprUnmap(pParse, pExpr);
  }
  sqlite3ExprDelete(db, pExpr);
}

void makeColumnPartOfPrimaryKey(Parse *pParse, Column *pCol) {
  pCol->colFlags |= 0x0001;

  if (pCol->colFlags & 0x0060) {
    sqlite3ErrorMsg(pParse, "generated columns cannot be part of the PRIMARY KEY");
  }
}

void sqlite3AddPrimaryKey(Parse *pParse, ExprList *pList, int onError, int autoInc, int sortOrder) {
  Table *pTab = pParse->pNewTable;
  Column *pCol = 0;
  int iCol = -1, i;
  int nTerm;
  if (pTab == 0)
    goto primary_key_exit;
  if (pTab->tabFlags & 0x00000004) {
    sqlite3ErrorMsg(pParse, "table \"%s\" has more than one primary key", pTab->zName);
    goto primary_key_exit;
  }
  pTab->tabFlags |= 0x00000004;
  if (pList == 0) {
    iCol = pTab->nCol - 1;
    pCol = &pTab->aCol[iCol];
    makeColumnPartOfPrimaryKey(pParse, pCol);
    nTerm = 1;
  } else {
    nTerm = pList->nExpr;
    for (i = 0; i < nTerm; i++) {
      Expr *pCExpr = sqlite3ExprSkipCollate(pList->a[i].pExpr);

      sqlite3StringToId(pCExpr);
      if (pCExpr->op == 60) {
        iCol = sqlite3ColumnIndex(pTab, pCExpr->u.zToken);
        if (iCol >= 0) {
          pCol = &pTab->aCol[iCol];
          makeColumnPartOfPrimaryKey(pParse, pCol);
        }
      }
    }
  }
  if (nTerm == 1 && pCol && pCol->eCType == 4 && sortOrder != 1) {
    if ((pParse->eParseMode >= 2) && pList) {
      Expr *pCExpr = sqlite3ExprSkipCollate(pList->a[0].pExpr);
      sqlite3RenameTokenRemap(pParse, &pTab->iPKey, pCExpr);
    }
    pTab->iPKey = iCol;
    pTab->keyConf = (u8)onError;

    pTab->tabFlags |= autoInc * 0x00000008;
    if (pList)
      pParse->iPkSortOrder = pList->a[0].fg.sortFlags;
    (void)sqlite3HasExplicitNulls(pParse, pList);
  } else if (autoInc) {
    sqlite3ErrorMsg(pParse,
                    "AUTOINCREMENT is only allowed on an "
                    "INTEGER PRIMARY KEY");

  } else {
    sqlite3CreateIndex(pParse, 0, 0, 0, pList, onError, 0, 0, sortOrder, 0, 2);
    pList = 0;
  }

primary_key_exit:
  sqlite3ExprListDelete(pParse->db, pList);
  return;
}

void sqlite3AddCheckConstraint(Parse *pParse, Expr *pCheckExpr, const char *zStart, const char *zEnd) {
  Table *pTab = pParse->pNewTable;
  sqlite3 *db = pParse->db;
  if (pTab && !(pParse->eParseMode == 1) && !sqlite3BtreeIsReadonly(db->aDb[db->init.iDb].pBt)) {
    pTab->pCheck = sqlite3ExprListAppend(pParse, pTab->pCheck, pCheckExpr);

    if (pParse->u1.cr.constraintName.n) {
      sqlite3ExprListSetName(pParse, pTab->pCheck, &pParse->u1.cr.constraintName, 1);
    } else {
      Token t;
      for (zStart++; (sqlite3CtypeMap[(unsigned char)(zStart[0])] & 0x01); zStart++) {
      }
      while ((sqlite3CtypeMap[(unsigned char)(zEnd[-1])] & 0x01)) {
        zEnd--;
      }
      t.z = zStart;
      t.n = (int)(zEnd - t.z);
      sqlite3ExprListSetName(pParse, pTab->pCheck, &t, 1);
    }
  } else {
    sqlite3ExprDelete(pParse->db, pCheckExpr);
  }
}

void sqlite3AddCollateType(Parse *pParse, Token *pToken) {
  Table *p;
  int i;
  char *zColl;
  sqlite3 *db;

  if ((p = pParse->pNewTable) == 0 || (pParse->eParseMode >= 2))
    return;
  i = p->nCol - 1;
  db = pParse->db;
  zColl = sqlite3NameFromToken(db, pToken);
  if (!zColl)
    return;

  if (sqlite3LocateCollSeq(pParse, zColl)) {
    Index *pIdx;
    sqlite3ColumnSetColl(db, &p->aCol[i], zColl);

    for (pIdx = p->pIndex; pIdx; pIdx = pIdx->pNext) {
      if (pIdx->aiColumn[0] == i) {
        pIdx->azColl[0] = sqlite3ColumnColl(&p->aCol[i]);
      }
    }
  }
  sqlite3DbFree(db, zColl);
}

void sqlite3AddGenerated(Parse *pParse, Expr *pExpr, Token *pType) {
  u8 eType = 0x0020;
  Table *pTab = pParse->pNewTable;
  Column *pCol;
  if (pTab == 0) {
    goto generated_done;
  }
  pCol = &(pTab->aCol[pTab->nCol - 1]);
  if (pParse->eParseMode == 1) {
    sqlite3ErrorMsg(pParse, "virtual tables cannot use computed columns");
    goto generated_done;
  }
  if (pCol->iDflt > 0)
    goto generated_error;
  if (pType) {
    if (pType->n == 7 && sqlite3_strnicmp("virtual", pType->z, 7) == 0) {
    } else if (pType->n == 6 && sqlite3_strnicmp("stored", pType->z, 6) == 0) {
      eType = 0x0040;
    } else {
      goto generated_error;
    }
  }
  if (eType == 0x0020)
    pTab->nNVCol--;
  pCol->colFlags |= eType;

  pTab->tabFlags |= eType;
  if (pCol->colFlags & 0x0001) {
    makeColumnPartOfPrimaryKey(pParse, pCol);
  }
  if ((pExpr) && pExpr->op == 60) {
    pExpr = sqlite3PExpr(pParse, 173, pExpr, 0);
  }
  if (pExpr && pExpr->op != 72)
    pExpr->affExpr = pCol->affinity;
  sqlite3ColumnSetExpr(pParse, pTab, pCol, pExpr);
  pExpr = 0;
  goto generated_done;

generated_error:
  sqlite3ErrorMsg(pParse, "error in generated column \"%s\"", pCol->zCnName);
generated_done:
  sqlite3ExprDelete(pParse->db, pExpr);
}

void sqlite3ChangeCookie(Parse *pParse, int iDb) {
  sqlite3 *db = pParse->db;
  Vdbe *v = pParse->pVdbe;

  sqlite3VdbeAddOp3(v, 102, iDb, 1, (int)(1 + (unsigned)db->aDb[iDb].pSchema->schema_cookie));
}

int resizeIndexObject(Parse *pParse, Index *pIdx, int N) {
  char *zExtra;
  u64 nByte;
  sqlite3 *db;
  if (pIdx->nColumn >= N)
    return SQLITE_OK;
  db = pParse->db;

  nByte = (sizeof(char *) + sizeof(LogEst) + sizeof(i16) + 1) * (u64)N;
  zExtra = (char*)(sqlite3DbMallocZero(db, nByte));
  if (zExtra == 0)
    return 7;
  memcpy(zExtra, pIdx->azColl, sizeof(char *) * pIdx->nColumn);
  pIdx->azColl = (const char **)zExtra;
  zExtra += sizeof(char *) * N;
  memcpy(zExtra, pIdx->aiRowLogEst, sizeof(LogEst) * (pIdx->nKeyCol + 1));
  pIdx->aiRowLogEst = (LogEst *)zExtra;
  zExtra += sizeof(LogEst) * N;
  memcpy(zExtra, pIdx->aiColumn, sizeof(i16) * pIdx->nColumn);
  pIdx->aiColumn = (i16 *)zExtra;
  zExtra += sizeof(i16) * N;
  memcpy(zExtra, pIdx->aSortOrder, pIdx->nColumn);
  pIdx->aSortOrder = (u8 *)zExtra;
  pIdx->nColumn = (u16)N;
  pIdx->isResized = 1;
  return SQLITE_OK;
}

void convertToWithoutRowidTable(Parse *pParse, Table *pTab) {
  Index *pIdx;
  Index *pPk;
  int nPk;
  int nExtra;
  int i, j;
  sqlite3 *db = pParse->db;
  Vdbe *v = pParse->pVdbe;

  if (!db->init.imposterTable) {
    for (i = 0; i < pTab->nCol; i++) {
      if ((pTab->aCol[i].colFlags & 0x0001) != 0 && (pTab->aCol[i].notNull == 0)) {
        pTab->aCol[i].notNull = 2;
      }
    }
    pTab->tabFlags |= 0x00000800;
  }

  if (pParse->u1.cr.addrCrTab) {
    sqlite3VdbeChangeP3(v, pParse->u1.cr.addrCrTab, 2);
  }

  if (pTab->iPKey >= 0) {
    ExprList *pList;
    Token ipkToken;
    sqlite3TokenInit(&ipkToken, pTab->aCol[pTab->iPKey].zCnName);
    pList = sqlite3ExprListAppend(pParse, 0, sqlite3ExprAlloc(db, 60, &ipkToken, 0));
    if (pList == 0) {
      pTab->tabFlags &= ~0x00000080;
      return;
    }
    if ((pParse->eParseMode >= 2)) {
      sqlite3RenameTokenRemap(pParse, pList->a[0].pExpr, &pTab->iPKey);
    }
    pList->a[0].fg.sortFlags = pParse->iPkSortOrder;

    pTab->iPKey = -1;
    sqlite3CreateIndex(pParse, 0, 0, 0, pList, pTab->keyConf, 0, 0, 0, 0, 2);
    if (pParse->nErr) {
      pTab->tabFlags &= ~0x00000080;
      return;
    }

    pPk = sqlite3PrimaryKeyIndex(pTab);

  } else {
    pPk = sqlite3PrimaryKeyIndex(pTab);

    for (i = j = 1; i < pPk->nKeyCol; i++) {
      if (isDupColumn(pPk, j, pPk, i)) {
        pPk->nColumn--;
      } else {
        pPk->azColl[j] = pPk->azColl[i];
        pPk->aSortOrder[j] = pPk->aSortOrder[i];
        pPk->aiColumn[j++] = pPk->aiColumn[i];
      }
    }
    pPk->nKeyCol = j;
  }

  pPk->isCovering = 1;
  if (!db->init.imposterTable)
    pPk->uniqNotNull = 1;
  nPk = pPk->nColumn = pPk->nKeyCol;

  if (v && pPk->tnum > 0) {
    sqlite3VdbeChangeOpcode(v, (int)pPk->tnum, 9);
  }

  pPk->tnum = pTab->tnum;

  for (pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext) {
    int n;
    if ((pIdx)->idxType == 2)
      continue;
    for (i = n = 0; i < nPk; i++) {
      if (!isDupColumn(pIdx, pIdx->nKeyCol, pPk, i)) {
        n++;
      }
    }
    if (n == 0) {
      pIdx->nColumn = pIdx->nKeyCol;
      continue;
    }
    if (resizeIndexObject(pParse, pIdx, pIdx->nKeyCol + n))
      return;
    for (i = 0, j = pIdx->nKeyCol; i < nPk; i++) {
      if (!isDupColumn(pIdx, pIdx->nKeyCol, pPk, i)) {
        pIdx->aiColumn[j] = pPk->aiColumn[i];
        pIdx->azColl[j] = pPk->azColl[i];
        if (pPk->aSortOrder[i]) {
          pIdx->bAscKeyBug = 1;
        }
        j++;
      }
    }
  }

  nExtra = 0;
  for (i = 0; i < pTab->nCol; i++) {
    if (!hasColumn(pPk->aiColumn, nPk, i) && (pTab->aCol[i].colFlags & 0x0020) == 0)
      nExtra++;
  }
  if (resizeIndexObject(pParse, pPk, nPk + nExtra))
    return;
  for (i = 0, j = nPk; i < pTab->nCol; i++) {
    if (!hasColumn(pPk->aiColumn, j, i) && (pTab->aCol[i].colFlags & 0x0020) == 0) {
      const char *zColl = sqlite3ColumnColl(&pTab->aCol[i]);

      pPk->aiColumn[j] = i;
      pPk->azColl[j] = zColl ? zColl : sqlite3StrBINARY;
      j++;
    }
  }

  recomputeColumnsNotIndexed(pPk);
}

void sqlite3EndTable(Parse *pParse, Token *pCons, Token *pEnd, u32 tabOpts, Select *pSelect) {
  Table *p;
  sqlite3 *db = pParse->db;
  int iDb;
  Index *pIdx;

  if (pEnd == 0 && pSelect == 0) {
    return;
  }
  p = pParse->pNewTable;
  if (p == 0)
    return;

  if (pSelect == 0 && sqlite3ShadowTableName(db, p->zName)) {
    p->tabFlags |= 0x00001000;
  }

  if (db->init.busy) {
    if (pSelect || (!((p)->eTabType == 0) && db->init.newTnum)) {
      sqlite3ErrorMsg(pParse, "");
      return;
    }
    p->tnum = db->init.newTnum;
    if (p->tnum == 1)
      p->tabFlags |= 0x00000001;
  }

  if (tabOpts & 0x00010000) {
    int ii;
    p->tabFlags |= 0x00010000;
    for (ii = 0; ii < p->nCol; ii++) {
      Column *pCol = &p->aCol[ii];
      if (pCol->eCType == 0) {
        if (pCol->colFlags & 0x0004) {
          sqlite3ErrorMsg(pParse, "unknown datatype for %s.%s: \"%s\"", p->zName, pCol->zCnName,
                          sqlite3ColumnType(pCol, (char*)("")));
        } else {
          sqlite3ErrorMsg(pParse, "missing datatype for %s.%s", p->zName, pCol->zCnName);
        }
        return;
      } else if (pCol->eCType == 1) {
        pCol->affinity = 0x41;
      }
      if ((pCol->colFlags & 0x0001) != 0 && p->iPKey != ii && pCol->notNull == 0) {
        pCol->notNull = 2;
        p->tabFlags |= 0x00000800;
      }
    }
  }

  if (tabOpts & 0x00000080) {
    if ((p->tabFlags & 0x00000008)) {
      sqlite3ErrorMsg(pParse, "AUTOINCREMENT not allowed on WITHOUT ROWID tables");
      return;
    }
    if ((p->tabFlags & 0x00000004) == 0) {
      sqlite3ErrorMsg(pParse, "PRIMARY KEY missing on table %s", p->zName);
      return;
    }
    p->tabFlags |= 0x00000080 | 0x00000200;
    convertToWithoutRowidTable(pParse, p);
  }
  iDb = sqlite3SchemaToIndex(db, p->pSchema);

  if (p->pCheck) {
    sqlite3ResolveSelfReference(pParse, p, 0x000004, 0, p->pCheck);
    if (pParse->nErr) {
      sqlite3ExprListDelete(db, p->pCheck);
      p->pCheck = 0;
    } else {
    }
  }

  if (p->tabFlags & 0x00000060) {
    int ii, nNG = 0;
    for (ii = 0; ii < p->nCol; ii++) {
      u32 colFlags = p->aCol[ii].colFlags;
      if ((colFlags & 0x0060) != 0) {
        Expr *pX = sqlite3ColumnExpr(p, &p->aCol[ii]);
        if (sqlite3ResolveSelfReference(pParse, p, 0x000008, pX, 0)) {
          sqlite3ColumnSetExpr(pParse, p, &p->aCol[ii], sqlite3ExprAlloc(db, 122, 0, 0));
        }
      } else {
        nNG++;
      }
    }
    if (nNG == 0) {
      sqlite3ErrorMsg(pParse, "must have at least one non-generated column");
      return;
    }
  }

  estimateTableWidth(p);
  for (pIdx = p->pIndex; pIdx; pIdx = pIdx->pNext) {
    estimateIndexWidth(pIdx);
  }

  if (!db->init.busy) {
    int n;
    Vdbe *v;
    char *zType;
    char *zType2;
    char *zStmt;

    v = sqlite3GetVdbe(pParse);
    if (v == 0)
      return;

    sqlite3VdbeAddOp1(v, 124, 0);

    if ((p)->eTabType == 0) {
      zType = (char*)("table");
      zType2 = (char*)("TABLE");

    } else {
      zType = (char*)("view");
      zType2 = (char*)("VIEW");
    }

    if (pSelect) {
      SelectDest dest;
      int regYield;
      int addrTop;
      int regRec;
      int regRowid;
      int addrInsLoop;
      Table *pSelTab;
      int iCsr;

      if ((pParse->eParseMode != 0)) {
        pParse->rc = SQLITE_ERROR;
        pParse->nErr++;
        return;
      }
      iCsr = pParse->nTab++;
      regYield = ++pParse->nMem;
      regRec = ++pParse->nMem;
      regRowid = ++pParse->nMem;
      sqlite3MayAbort(pParse);

      sqlite3VdbeAddOp3(v, 116, iCsr, pParse->u1.cr.regRoot, iDb);
      sqlite3VdbeChangeP5(v, 0x10);
      addrTop = sqlite3VdbeCurrentAddr(v) + 1;
      sqlite3VdbeAddOp3(v, 11, regYield, 0, addrTop);
      if (pParse->nErr)
        return;
      pSelTab = sqlite3ResultSetOfSelect(pParse, pSelect, 0x41);
      if (pSelTab == 0)
        return;

      p->nCol = p->nNVCol = pSelTab->nCol;
      p->aCol = pSelTab->aCol;
      pSelTab->nCol = 0;
      pSelTab->aCol = 0;
      sqlite3DeleteTable(db, pSelTab);
      sqlite3SelectDestInit(&dest, 11, regYield);
      sqlite3Select(pParse, pSelect, &dest);
      if (pParse->nErr)
        return;
      sqlite3VdbeEndCoroutine(v, regYield);
      sqlite3VdbeJumpHere(v, addrTop - 1);
      addrInsLoop = sqlite3VdbeAddOp1(v, 12, dest.iSDParm);
      sqlite3VdbeAddOp3(v, 99, dest.iSdst, dest.nSdst, regRec);
      sqlite3TableAffinity(v, p, 0);
      sqlite3VdbeAddOp2(v, 129, iCsr, regRowid);
      sqlite3VdbeAddOp3(v, 130, iCsr, regRec, regRowid);
      sqlite3VdbeGoto(v, addrInsLoop);
      sqlite3VdbeJumpHere(v, addrInsLoop);
      sqlite3VdbeAddOp1(v, 124, iCsr);
    }

    if (pSelect) {
      zStmt = createTableStmt(db, p);
    } else {
      Token *pEnd2 = tabOpts ? &pParse->sLastToken : pEnd;
      n = (int)(pEnd2->z - pParse->sNameToken.z);
      if (pEnd2->z[0] != ';')
        n += pEnd2->n;
      zStmt = sqlite3MPrintf(db, "CREATE %s %.*s", zType2, n, pParse->sNameToken.z);
    }

    sqlite3NestedParse(pParse,
                       "UPDATE %Q."
                       "sqlite_master"
                       " SET type='%s', name=%Q, tbl_name=%Q, rootpage=#%d, sql=%Q"
                       " WHERE rowid=#%d",
                       db->aDb[iDb].zDbSName, zType, p->zName, p->zName, pParse->u1.cr.regRoot, zStmt,
                       pParse->u1.cr.regRowid);
    sqlite3DbFree(db, zStmt);
    sqlite3ChangeCookie(pParse, iDb);

    if ((p->tabFlags & 0x00000008) != 0 && !(pParse->eParseMode != 0)) {
      Db *pDb = &db->aDb[iDb];

      if (pDb->pSchema->pSeqTab == 0) {
        sqlite3NestedParse(pParse, "CREATE TABLE %Q.sqlite_sequence(name,seq)", pDb->zDbSName);
      }
    }

    sqlite3VdbeAddParseSchemaOp(v, iDb, sqlite3MPrintf(db, "tbl_name='%q' AND type!='trigger'", p->zName), 0);

    if (p->tabFlags & 0x00000060) {
      sqlite3VdbeAddOp4(v, 150, 0x0001, 0, 0,
                        sqlite3MPrintf(db, "SELECT*FROM\"%w\".\"%w\"", db->aDb[iDb].zDbSName, p->zName), (-7));
    }
  }

  if (db->init.busy) {
    Table *pOld;
    Schema *pSchema = p->pSchema;

    pOld = (Table*)(sqlite3HashInsert(&pSchema->tblHash, p->zName, p));
    if (pOld) {
      sqlite3OomFault(db);
      return;
    }
    pParse->pNewTable = 0;
    db->mDbFlags |= 0x0001;

    if (strcmp(p->zName, "sqlite_sequence") == 0) {
      p->pSchema->pSeqTab = p;
    }
  }

  if (!pSelect && ((p)->eTabType == 0)) {
    if (pCons->z == 0) {
      pCons = pEnd;
    }
    p->u.tab.addColOffset = 13 + (int)(pCons->z - pParse->sNameToken.z);
  }
}

void sqlite3CreateView(Parse *pParse, Token *pBegin, Token *pName1, Token *pName2, ExprList *pCNames, Select *pSelect,
                       int isTemp, int noErr) {
  Table *p;
  int n;
  const char *z;
  Token sEnd;
  DbFixer sFix;
  Token *pName = 0;
  int iDb;
  sqlite3 *db = pParse->db;

  if (pParse->nVar > 0) {
    sqlite3ErrorMsg(pParse, "parameters are not allowed in views");
    goto create_view_fail;
  }
  sqlite3StartTable(pParse, pName1, pName2, isTemp, 1, 0, noErr);
  p = pParse->pNewTable;
  if (p == 0 || pParse->nErr)
    goto create_view_fail;

  p->tabFlags |= 0x00000200;

  sqlite3TwoPartName(pParse, pName1, pName2, &pName);
  iDb = sqlite3SchemaToIndex(db, p->pSchema);

  sqlite3FixInit(&sFix, pParse, iDb, "view", pName);
  if (sqlite3FixSelect(&sFix, pSelect))
    goto create_view_fail;

  pSelect->selFlags |= 0x0200000;
  if ((pParse->eParseMode >= 2)) {
    p->u.view.pSelect = pSelect;
    pSelect = 0;
  } else {
    p->u.view.pSelect = sqlite3SelectDup(db, pSelect, 0x0001);
  }
  p->pCheck = sqlite3ExprListDup(db, pCNames, 0x0001);
  p->eTabType = 2;
  if (db->mallocFailed)
    goto create_view_fail;

  sEnd = pParse->sLastToken;

  if (sEnd.z[0] != ';') {
    sEnd.z += sEnd.n;
  }
  sEnd.n = 0;
  n = (int)(sEnd.z - pBegin->z);

  z = pBegin->z;
  while ((sqlite3CtypeMap[(unsigned char)(z[n - 1])] & 0x01)) {
    n--;
  }
  sEnd.z = &z[n - 1];
  sEnd.n = 1;

  sqlite3EndTable(pParse, 0, &sEnd, 0, 0);

create_view_fail:
  sqlite3SelectDelete(db, pSelect);
  if ((pParse->eParseMode >= 2)) {
    sqlite3RenameExprlistUnmap(pParse, pCNames);
  }
  sqlite3ExprListDelete(db, pCNames);
  return;
}

__attribute__((noinline)) int viewGetColumnNames(Parse *pParse, Table *pTable) {
  Table *pSelTab;
  Select *pSel;
  int nErr = 0;
  sqlite3 *db = pParse->db;

  int rc;

  sqlite3_xauth xAuth;

  if ((pTable)->eTabType == 1) {
    db->nSchemaLock++;
    rc = sqlite3VtabCallConnect(pParse, pTable);
    db->nSchemaLock--;
    return rc;
  }

  if (pTable->nCol < 0) {
    sqlite3ErrorMsg(pParse, "view %s is circularly defined", pTable->zName);
    return 1;
  }

  pSel = sqlite3SelectDup(db, pTable->u.view.pSelect, 0);
  if (pSel) {
    u8 eParseMode = pParse->eParseMode;
    int nTab = pParse->nTab;
    int nSelect = pParse->nSelect;
    pParse->eParseMode = 0;
    sqlite3SrcListAssignCursors(pParse, pSel->pSrc);
    pTable->nCol = -1;
    db->lookaside.bDisable++;
    db->lookaside.sz = 0;

    xAuth = db->xAuth;
    db->xAuth = 0;
    pSelTab = sqlite3ResultSetOfSelect(pParse, pSel, 0x40);
    db->xAuth = xAuth;

    pParse->nTab = nTab;
    pParse->nSelect = nSelect;
    if (pSelTab == 0) {
      pTable->nCol = 0;
      nErr++;
    } else if (pTable->pCheck) {
      sqlite3ColumnsFromExprList(pParse, pTable->pCheck, &pTable->nCol, &pTable->aCol);
      if (pParse->nErr == 0 && pTable->nCol == pSel->pEList->nExpr) {
        sqlite3SubqueryColumnTypes(pParse, pTable, pSel, 0x40);
      }
    } else {
      pTable->nCol = pSelTab->nCol;
      pTable->aCol = pSelTab->aCol;
      pTable->tabFlags |= (pSelTab->tabFlags & 0x0062);
      pSelTab->nCol = 0;
      pSelTab->aCol = 0;
    }
    pTable->nNVCol = pTable->nCol;
    sqlite3DeleteTable(db, pSelTab);
    sqlite3SelectDelete(db, pSel);
    db->lookaside.bDisable--;
    db->lookaside.sz = db->lookaside.bDisable ? 0 : db->lookaside.szTrue;
    pParse->eParseMode = eParseMode;
  } else {
    nErr++;
  }
  pTable->pSchema->schemaFlags |= 0x0002;
  if (db->mallocFailed) {
    sqlite3DeleteColumnNames(db, pTable);
  }

  return nErr + pParse->nErr;
}

int sqlite3ViewGetColumnNames(Parse *pParse, Table *pTable) {
  if (!((pTable)->eTabType == 1) && pTable->nCol > 0)
    return 0;
  return viewGetColumnNames(pParse, pTable);
}

void destroyRootPage(Parse *pParse, int iTable, int iDb) {
  Vdbe *v = sqlite3GetVdbe(pParse);
  int r1 = sqlite3GetTempReg(pParse);
  if (iTable < 2)
    sqlite3ErrorMsg(pParse, "corrupt schema");
  sqlite3VdbeAddOp3(v, 146, iTable, r1, iDb);
  sqlite3MayAbort(pParse);

  sqlite3NestedParse(pParse,
                     "UPDATE %Q."
                     "sqlite_master"
                     " SET rootpage=%d WHERE #%d AND rootpage=#%d",
                     pParse->db->aDb[iDb].zDbSName, iTable, r1, r1);

  sqlite3ReleaseTempReg(pParse, r1);
}

void destroyTable(Parse *pParse, Table *pTab) {
  Pgno iTab = pTab->tnum;
  Pgno iDestroyed = 0;

  while (1) {
    Index *pIdx;
    Pgno iLargest = 0;

    if (iDestroyed == 0 || iTab < iDestroyed) {
      iLargest = iTab;
    }
    for (pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext) {
      Pgno iIdx = pIdx->tnum;

      if ((iDestroyed == 0 || (iIdx < iDestroyed)) && iIdx > iLargest) {
        iLargest = iIdx;
      }
    }
    if (iLargest == 0) {
      return;
    } else {
      int iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);

      destroyRootPage(pParse, iLargest, iDb);
      iDestroyed = iLargest;
    }
  }
}

void sqlite3ClearStatTables(Parse *pParse, int iDb, const char *zType, const char *zName) {
  int i;
  const char *zDbName = pParse->db->aDb[iDb].zDbSName;
  for (i = 1; i <= 4; i++) {
    char zTab[24];
    sqlite3_snprintf(sizeof(zTab), zTab, "sqlite_stat%d", i);
    if (sqlite3FindTable(pParse->db, zTab, zDbName)) {
      sqlite3NestedParse(pParse, "DELETE FROM %Q.%s WHERE %s=%Q", zDbName, zTab, zType, zName);
    }
  }
}

void sqlite3CodeDropTable(Parse *pParse, Table *pTab, int iDb, int isView) {
  Vdbe *v;
  sqlite3 *db = pParse->db;
  Trigger *pTrigger;
  Db *pDb = &db->aDb[iDb];

  v = sqlite3GetVdbe(pParse);

  sqlite3BeginWriteOperation(pParse, 1, iDb);

  if ((pTab)->eTabType == 1) {
    sqlite3VdbeAddOp0(v, 172);
  }

  pTrigger = sqlite3TriggerList(pParse, pTab);
  while (pTrigger) {
    sqlite3DropTriggerPtr(pParse, pTrigger);
    pTrigger = pTrigger->pNext;
  }

  if (pTab->tabFlags & 0x00000008) {
    sqlite3NestedParse(pParse, "DELETE FROM %Q.sqlite_sequence WHERE name=%Q", pDb->zDbSName, pTab->zName);
  }

  sqlite3NestedParse(pParse,
                     "DELETE FROM %Q."
                     "sqlite_master"
                     " WHERE tbl_name=%Q and type!='trigger'",
                     pDb->zDbSName, pTab->zName);
  if (!isView && !((pTab)->eTabType == 1)) {
    destroyTable(pParse, pTab);
  }

  if ((pTab)->eTabType == 1) {
    sqlite3VdbeAddOp4(v, 174, iDb, 0, 0, pTab->zName, 0);
    sqlite3MayAbort(pParse);
  }
  sqlite3VdbeAddOp4(v, 153, iDb, 0, 0, pTab->zName, 0);
  sqlite3ChangeCookie(pParse, iDb);
  sqliteViewResetAll(db, iDb);
}

void sqlite3DropTable(Parse *pParse, SrcList *pName, int isView, int noErr) {
  Table *pTab;
  Vdbe *v;
  sqlite3 *db = pParse->db;
  int iDb;

  if (db->mallocFailed) {
    goto exit_drop_table;
  }

  if (sqlite3ReadSchema(pParse))
    goto exit_drop_table;
  if (noErr)
    db->suppressErr++;

  pTab = sqlite3LocateTableItem(pParse, isView, &pName->a[0]);
  if (noErr)
    db->suppressErr--;

  if (pTab == 0) {
    if (noErr) {
      sqlite3CodeVerifyNamedSchema(pParse, pName->a[0].u4.zDatabase);
      sqlite3ForceNotReadOnly(pParse);
    }
    goto exit_drop_table;
  }
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);

  if (((pTab)->eTabType == 1) && sqlite3ViewGetColumnNames(pParse, pTab)) {
    goto exit_drop_table;
  }

  {
    int code;
    const char *zTab = ((!0) && (iDb == 1) ? "sqlite_temp_master" : "sqlite_master");
    const char *zDb = db->aDb[iDb].zDbSName;
    const char *zArg2 = 0;
    if (sqlite3AuthCheck(pParse, SQLITE_DELETE, zTab, 0, zDb)) {
      goto exit_drop_table;
    }
    if (isView) {
      if (!0 && iDb == 1) {
        code = SQLITE_DROP_TEMP_VIEW;
      } else {
        code = SQLITE_DROP_VIEW;
      }

    } else if ((pTab)->eTabType == 1) {
      code = SQLITE_DROP_VTABLE;
      zArg2 = sqlite3GetVTable(db, pTab)->pMod->zName;

    } else {
      if (!0 && iDb == 1) {
        code = SQLITE_DROP_TEMP_TABLE;
      } else {
        code = SQLITE_DROP_TABLE;
      }
    }
    if (sqlite3AuthCheck(pParse, code, pTab->zName, zArg2, zDb)) {
      goto exit_drop_table;
    }
    if (sqlite3AuthCheck(pParse, SQLITE_DELETE, pTab->zName, 0, zDb)) {
      goto exit_drop_table;
    }
  }

  if (tableMayNotBeDropped(db, pTab)) {
    sqlite3ErrorMsg(pParse, "table %s may not be dropped", pTab->zName);
    goto exit_drop_table;
  }

  if (isView && !((pTab)->eTabType == 2)) {
    sqlite3ErrorMsg(pParse, "use DROP TABLE to delete table %s", pTab->zName);
    goto exit_drop_table;
  }
  if (!isView && ((pTab)->eTabType == 2)) {
    sqlite3ErrorMsg(pParse, "use DROP VIEW to delete view %s", pTab->zName);
    goto exit_drop_table;
  }

  v = sqlite3GetVdbe(pParse);
  if (v) {
    sqlite3BeginWriteOperation(pParse, 1, iDb);
    if (!isView) {
      sqlite3ClearStatTables(pParse, iDb, "tbl", pTab->zName);
      sqlite3FkDropTable(pParse, pName, pTab);
    }
    sqlite3CodeDropTable(pParse, pTab, iDb, isView);
  }

exit_drop_table:
  sqlite3SrcListDelete(db, pName);
}

void sqlite3CreateForeignKey(Parse *pParse, ExprList *pFromCol, Token *pTo, ExprList *pToCol, int flags) {
  sqlite3 *db = pParse->db;

  FKey *pFKey = 0;
  FKey *pNextTo;
  Table *p = pParse->pNewTable;
  i64 nByte;
  int i;
  int nCol;
  char *z;

  if (p == 0 || (pParse->eParseMode == 1))
    goto fk_end;
  if (pFromCol == 0) {
    int iCol = p->nCol - 1;
    if ((iCol < 0))
      goto fk_end;
    if (pToCol && pToCol->nExpr != 1) {
      sqlite3ErrorMsg(pParse,
                      "foreign key on %s"
                      " should reference only one column of table %T",
                      p->aCol[iCol].zCnName, pTo);
      goto fk_end;
    }
    nCol = 1;
  } else if (pToCol && pToCol->nExpr != pFromCol->nExpr) {
    sqlite3ErrorMsg(pParse,
                    "number of columns in foreign key does not match the number of "
                    "columns in the referenced table");
    goto fk_end;
  } else {
    nCol = pFromCol->nExpr;
  }
  nByte = (offsetof(FKey, aCol) + (nCol) * sizeof(struct sColMap)) + pTo->n + 1;
  if (pToCol) {
    for (i = 0; i < pToCol->nExpr; i++) {
      nByte += sqlite3Strlen30(pToCol->a[i].zEName) + 1;
    }
  }
  pFKey = (FKey*)(sqlite3DbMallocZero(db, nByte));
  if (pFKey == 0) {
    goto fk_end;
  }
  pFKey->pFrom = p;

  pFKey->pNextFrom = p->u.tab.pFKey;
  z = (char *)&pFKey->aCol[nCol];
  pFKey->zTo = z;
  if ((pParse->eParseMode >= 2)) {
    sqlite3RenameTokenMap(pParse, (void *)z, pTo);
  }
  memcpy(z, pTo->z, pTo->n);
  z[pTo->n] = 0;
  sqlite3Dequote(z);
  z += pTo->n + 1;
  pFKey->nCol = nCol;
  if (pFromCol == 0) {
    pFKey->aCol[0].iFrom = p->nCol - 1;
  } else {
    for (i = 0; i < nCol; i++) {
      int j;
      for (j = 0; j < p->nCol; j++) {
        if (sqlite3StrICmp(p->aCol[j].zCnName, pFromCol->a[i].zEName) == 0) {
          pFKey->aCol[i].iFrom = j;
          break;
        }
      }
      if (j >= p->nCol) {
        sqlite3ErrorMsg(pParse, "unknown column \"%s\" in foreign key definition", pFromCol->a[i].zEName);
        goto fk_end;
      }
      if ((pParse->eParseMode >= 2)) {
        sqlite3RenameTokenRemap(pParse, &pFKey->aCol[i], pFromCol->a[i].zEName);
      }
    }
  }
  if (pToCol) {
    for (i = 0; i < nCol; i++) {
      int n = sqlite3Strlen30(pToCol->a[i].zEName);
      pFKey->aCol[i].zCol = z;
      if ((pParse->eParseMode >= 2)) {
        sqlite3RenameTokenRemap(pParse, z, pToCol->a[i].zEName);
      }
      memcpy(z, pToCol->a[i].zEName, n);
      z[n] = 0;
      z += n + 1;
    }
  }
  pFKey->isDeferred = 0;
  pFKey->aAction[0] = (u8)(flags & 0xff);
  pFKey->aAction[1] = (u8)((flags >> 8) & 0xff);

  pNextTo = (FKey *)sqlite3HashInsert(&p->pSchema->fkeyHash, pFKey->zTo, (void *)pFKey);
  if (pNextTo == pFKey) {
    sqlite3OomFault(db);
    goto fk_end;
  }
  if (pNextTo) {
    pFKey->pNextTo = pNextTo;
    pNextTo->pPrevTo = pFKey;
  }

  p->u.tab.pFKey = pFKey;
  pFKey = 0;

fk_end:
  sqlite3DbFree(db, pFKey);

  sqlite3ExprListDelete(db, pFromCol);
  sqlite3ExprListDelete(db, pToCol);
}

void sqlite3DeferForeignKey(Parse *pParse, int isDeferred) {
  Table *pTab;
  FKey *pFKey;
  if ((pTab = pParse->pNewTable) == 0)
    return;
  if ((!((pTab)->eTabType == 0)))
    return;
  if ((pFKey = pTab->u.tab.pFKey) == 0)
    return;

  pFKey->isDeferred = (u8)isDeferred;
}

void sqlite3RefillIndex(Parse *pParse, Index *pIndex, int memRootPage) {
  Table *pTab = pIndex->pTable;
  int iTab = pParse->nTab++;
  int iIdx = pParse->nTab++;
  int iSorter;
  int addr1;
  int addr2;
  Pgno tnum;
  int iPartIdxLabel;
  Vdbe *v;
  KeyInfo *pKey;
  int regRecord;
  sqlite3 *db = pParse->db;
  int iDb = sqlite3SchemaToIndex(db, pIndex->pSchema);

  if (sqlite3AuthCheck(pParse, SQLITE_REINDEX, pIndex->zName, 0, db->aDb[iDb].zDbSName)) {
    return;
  }

  sqlite3TableLock(pParse, iDb, pTab->tnum, 1, pTab->zName);

  v = sqlite3GetVdbe(pParse);
  if (v == 0)
    return;
  if (memRootPage >= 0) {
    tnum = (Pgno)memRootPage;
  } else {
    tnum = pIndex->tnum;
  }
  pKey = sqlite3KeyInfoOfIndex(pParse, pIndex);

  iSorter = pParse->nTab++;
  sqlite3VdbeAddOp4(v, 121, iSorter, 0, pIndex->nKeyCol, (char *)sqlite3KeyInfoRef(pKey), (-9));

  sqlite3OpenTable(pParse, iTab, iDb, pTab, 114);
  addr1 = sqlite3VdbeAddOp2(v, 36, iTab, 0);
  regRecord = sqlite3GetTempReg(pParse);
  sqlite3MultiWrite(pParse);

  sqlite3GenerateIndexKey(pParse, pIndex, iTab, regRecord, 0, &iPartIdxLabel, 0, 0);
  sqlite3VdbeAddOp2(v, 141, iSorter, regRecord);
  sqlite3ResolvePartIdxLabel(pParse, iPartIdxLabel);
  sqlite3VdbeAddOp2(v, 40, iTab, addr1 + 1);
  sqlite3VdbeJumpHere(v, addr1);
  if (memRootPage < 0)
    sqlite3VdbeAddOp2(v, 147, tnum, iDb);
  sqlite3VdbeAddOp4(v, 116, iIdx, (int)tnum, iDb, (char *)pKey, (-9));
  sqlite3VdbeChangeP5(v, 0x01 | ((memRootPage >= 0) ? 0x10 : 0));

  addr1 = sqlite3VdbeAddOp2(v, 34, iSorter, 0);
  if (((pIndex)->onError != 0)) {
    int j2 = sqlite3VdbeGoto(v, 1);
    addr2 = sqlite3VdbeCurrentAddr(v);
    sqlite3VdbeAddOp4Int(v, 134, iSorter, j2, regRecord, pIndex->nKeyCol);
    sqlite3UniqueConstraint(pParse, 2, pIndex);
    sqlite3VdbeJumpHere(v, j2);
  } else {
    sqlite3MayAbort(pParse);
    addr2 = sqlite3VdbeCurrentAddr(v);
  }
  sqlite3VdbeAddOp3(v, 135, iSorter, regRecord, iIdx);
  if (!pIndex->bAscKeyBug) {
    sqlite3VdbeAddOp1(v, 139, iIdx);
  }
  sqlite3VdbeAddOp2(v, 140, iIdx, regRecord);
  sqlite3VdbeChangeP5(v, 0x10);
  sqlite3ReleaseTempReg(pParse, regRecord);
  sqlite3VdbeAddOp2(v, 38, iSorter, addr2);
  sqlite3VdbeJumpHere(v, addr1);

  sqlite3VdbeAddOp1(v, 124, iTab);
  sqlite3VdbeAddOp1(v, 124, iIdx);
  sqlite3VdbeAddOp1(v, 124, iSorter);
}

int sqlite3HasExplicitNulls(Parse *pParse, ExprList *pList) {
  if (pList) {
    int i;
    for (i = 0; i < pList->nExpr; i++) {
      if (pList->a[i].fg.bNulls) {
        u8 sf = pList->a[i].fg.sortFlags;
        sqlite3ErrorMsg(pParse, "unsupported use of NULLS %s", (sf == 0 || sf == 3) ? "FIRST" : "LAST");
        return 1;
      }
    }
  }
  return 0;
}

void sqlite3CreateIndex(Parse *pParse, Token *pName1, Token *pName2, SrcList *pTblName, ExprList *pList, int onError,
                        Token *pStart, Expr *pPIWhere, int sortOrder, int ifNotExist, u8 idxType) {
  Table *pTab = 0;
  Index *pIndex = 0;
  char *zName = 0;
  int nName;
  int i, j;
  DbFixer sFix;
  int sortOrderMask;
  sqlite3 *db = pParse->db;
  Db *pDb;
  int iDb;
  Token *pName = 0;
  struct ExprList_item *pListItem;
  int nExtra = 0;
  int nExtraCol;
  char *zExtra = 0;
  Index *pPk = 0;

  if (pParse->nErr) {
    goto exit_create_index;
  }

  if ((pParse->eParseMode == 1) && idxType != 2) {
    goto exit_create_index;
  }
  if (SQLITE_OK != sqlite3ReadSchema(pParse)) {
    goto exit_create_index;
  }
  if (sqlite3HasExplicitNulls(pParse, pList)) {
    goto exit_create_index;
  }

  if (pTblName != 0) {
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pName);
    if (iDb < 0)
      goto exit_create_index;

    if (!db->init.busy) {
      pTab = sqlite3SrcListLookup(pParse, pTblName);
      if (pName2->n == 0 && pTab && pTab->pSchema == db->aDb[1].pSchema) {
        iDb = 1;
      }
    }

    sqlite3FixInit(&sFix, pParse, iDb, "index", pName);
    if (sqlite3FixSrcList(&sFix, pTblName)) {
    }
    pTab = sqlite3LocateTableItem(pParse, 0, &pTblName->a[0]);

    if (pTab == 0)
      goto exit_create_index;
    if (iDb == 1 && db->aDb[iDb].pSchema != pTab->pSchema) {
      sqlite3ErrorMsg(pParse, "cannot create a TEMP index on non-TEMP table \"%s\"", pTab->zName);
      goto exit_create_index;
    }
    if (!(((pTab)->tabFlags & 0x00000080) == 0))
      pPk = sqlite3PrimaryKeyIndex(pTab);
  } else {
    pTab = pParse->pNewTable;
    if (!pTab)
      goto exit_create_index;
    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  }
  pDb = &db->aDb[iDb];

  if (sqlite3_strnicmp(pTab->zName, "sqlite_", 7) == 0 && db->init.busy == 0 && pTblName != 0) {
    sqlite3ErrorMsg(pParse, "table %s may not be indexed", pTab->zName);
    goto exit_create_index;
  }

  if ((pTab)->eTabType == 2) {
    sqlite3ErrorMsg(pParse, "views may not be indexed");
    goto exit_create_index;
  }

  if ((pTab)->eTabType == 1) {
    sqlite3ErrorMsg(pParse, "virtual tables may not be indexed");
    goto exit_create_index;
  }

  if (pName) {
    zName = sqlite3NameFromToken(db, pName);
    if (zName == 0)
      goto exit_create_index;

    if (SQLITE_OK != sqlite3CheckObjectName(pParse, zName, "index", pTab->zName)) {
      goto exit_create_index;
    }
    if (!(pParse->eParseMode >= 2)) {
      if (!db->init.busy) {
        if (sqlite3FindTable(db, zName, pDb->zDbSName) != 0) {
          sqlite3ErrorMsg(pParse, "there is already a table named %s", zName);
          goto exit_create_index;
        }
      }
      if (sqlite3FindIndex(db, zName, pDb->zDbSName) != 0) {
        if (!ifNotExist) {
          sqlite3ErrorMsg(pParse, "index %s already exists", zName);
        } else {
          sqlite3CodeVerifySchema(pParse, iDb);
          sqlite3ForceNotReadOnly(pParse);
        }
        goto exit_create_index;
      }
    }
  } else {
    int n;
    Index *pLoop;
    for (pLoop = pTab->pIndex, n = 1; pLoop; pLoop = pLoop->pNext, n++) {
    }
    zName = sqlite3MPrintf(db, "sqlite_autoindex_%s_%d", pTab->zName, n);
    if (zName == 0) {
      goto exit_create_index;
    }

    if ((pParse->eParseMode != 0))
      zName[7]++;
  }

  if (!(pParse->eParseMode >= 2)) {
    const char *zDb = pDb->zDbSName;
    if (sqlite3AuthCheck(pParse, SQLITE_INSERT, ((!0) && (iDb == 1) ? "sqlite_temp_master" : "sqlite_master"), 0,
                         zDb)) {
      goto exit_create_index;
    }
    i = SQLITE_CREATE_INDEX;
    if (!0 && iDb == 1)
      i = SQLITE_CREATE_TEMP_INDEX;
    if (sqlite3AuthCheck(pParse, i, zName, pTab->zName, zDb)) {
      goto exit_create_index;
    }
  }

  if (pList == 0) {
    Token prevCol;
    Column *pCol = &pTab->aCol[pTab->nCol - 1];
    pCol->colFlags |= 0x0008;
    sqlite3TokenInit(&prevCol, pCol->zCnName);
    pList = sqlite3ExprListAppend(pParse, 0, sqlite3ExprAlloc(db, 60, &prevCol, 0));
    if (pList == 0)
      goto exit_create_index;

    sqlite3ExprListSetSortOrder(pList, sortOrder, -1);
  } else {
    sqlite3ExprListCheckLength(pParse, pList, "index");
    if (pParse->nErr)
      goto exit_create_index;
  }

  for (i = 0; i < pList->nExpr; i++) {
    Expr *pExpr = pList->a[i].pExpr;

    if (pExpr->op == 114) {
      nExtra += (1 + sqlite3Strlen30(pExpr->u.zToken));
    }
  }

  nName = sqlite3Strlen30(zName);
  nExtraCol = pPk ? pPk->nKeyCol : 1;

  pIndex = sqlite3AllocateIndexObject(db, pList->nExpr + nExtraCol, nName + nExtra + 1, &zExtra);
  if (db->mallocFailed) {
    goto exit_create_index;
  }

  pIndex->zName = zExtra;
  zExtra += nName + 1;
  memcpy(pIndex->zName, zName, nName + 1);
  pIndex->pTable = pTab;
  pIndex->onError = (u8)onError;
  pIndex->uniqNotNull = onError != 0;
  pIndex->idxType = idxType;
  pIndex->pSchema = db->aDb[iDb].pSchema;
  pIndex->nKeyCol = pList->nExpr;
  if (pPIWhere) {
    sqlite3ResolveSelfReference(pParse, pTab, 0x000002, pPIWhere, 0);
    pIndex->pPartIdxWhere = pPIWhere;
    pPIWhere = 0;
  }

  if (pDb->pSchema->file_format >= 4) {
    sortOrderMask = -1;
  } else {
    sortOrderMask = 0;
  }

  pListItem = pList->a;
  if ((pParse->eParseMode >= 2)) {
    pIndex->aColExpr = pList;
    pList = 0;
  }
  for (i = 0; i < pIndex->nKeyCol; i++, pListItem++) {
    Expr *pCExpr;
    int requestedSortOrder;
    const char *zColl;

    sqlite3StringToId(pListItem->pExpr);
    sqlite3ResolveSelfReference(pParse, pTab, 0x000020, pListItem->pExpr, 0);
    if (pParse->nErr)
      goto exit_create_index;
    pCExpr = sqlite3ExprSkipCollate(pListItem->pExpr);
    if (pCExpr->op != 168) {
      if (pTab == pParse->pNewTable) {
        sqlite3ErrorMsg(pParse,
                        "expressions prohibited in PRIMARY KEY and "
                        "UNIQUE constraints");
        goto exit_create_index;
      }
      if (pIndex->aColExpr == 0) {
        pIndex->aColExpr = pList;
        pList = 0;
      }
      j = (-2);
      pIndex->aiColumn[i] = (-2);
      pIndex->uniqNotNull = 0;
      pIndex->bHasExpr = 1;
    } else {
      j = pCExpr->iColumn;

      if (j < 0) {
        j = pTab->iPKey;
      } else {
        if (pTab->aCol[j].notNull == 0) {
          pIndex->uniqNotNull = 0;
        }
        if (pTab->aCol[j].colFlags & 0x0020) {
          pIndex->bHasVCol = 1;
          pIndex->bHasExpr = 1;
        }
      }
      pIndex->aiColumn[i] = (i16)j;
    }
    zColl = 0;
    if (pListItem->pExpr->op == 114) {
      int nColl;

      zColl = pListItem->pExpr->u.zToken;
      nColl = sqlite3Strlen30(zColl) + 1;

      memcpy(zExtra, zColl, nColl);
      zColl = zExtra;
      zExtra += nColl;
      nExtra -= nColl;
    } else if (j >= 0) {
      zColl = sqlite3ColumnColl(&pTab->aCol[j]);
    }
    if (!zColl)
      zColl = sqlite3StrBINARY;
    if (!db->init.busy && !sqlite3LocateCollSeq(pParse, zColl)) {
      goto exit_create_index;
    }
    pIndex->azColl[i] = zColl;
    requestedSortOrder = pListItem->fg.sortFlags & sortOrderMask;
    pIndex->aSortOrder[i] = (u8)requestedSortOrder;
  }

  if (pPk) {
    for (j = 0; j < pPk->nKeyCol; j++) {
      int x = pPk->aiColumn[j];

      if (isDupColumn(pIndex, pIndex->nKeyCol, pPk, j)) {
        pIndex->nColumn--;
      } else {
        pIndex->aiColumn[i] = x;
        pIndex->azColl[i] = pPk->azColl[j];
        pIndex->aSortOrder[i] = pPk->aSortOrder[j];
        i++;
      }
    }

  } else {
    pIndex->aiColumn[i] = (-1);
    pIndex->azColl[i] = sqlite3StrBINARY;
  }
  sqlite3DefaultRowEst(pIndex);
  if (pParse->pNewTable == 0)
    estimateIndexWidth(pIndex);

  recomputeColumnsNotIndexed(pIndex);
  if (pTblName != 0 && pIndex->nColumn >= pTab->nCol) {
    pIndex->isCovering = 1;
    for (j = 0; j < pTab->nCol; j++) {
      if (j == pTab->iPKey)
        continue;
      if (sqlite3TableColumnToIndex(pIndex, j) >= 0)
        continue;
      pIndex->isCovering = 0;
      break;
    }
  }

  if (pTab == pParse->pNewTable) {
    Index *pIdx;
    for (pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext) {
      int k;

      if (pIdx->nKeyCol != pIndex->nKeyCol)
        continue;
      for (k = 0; k < pIdx->nKeyCol; k++) {
        const char *z1;
        const char *z2;

        if (pIdx->aiColumn[k] != pIndex->aiColumn[k])
          break;
        z1 = pIdx->azColl[k];
        z2 = pIndex->azColl[k];
        if (sqlite3StrICmp(z1, z2))
          break;
      }
      if (k == pIdx->nKeyCol) {
        if (pIdx->onError != pIndex->onError) {
          if (!(pIdx->onError == 11 || pIndex->onError == 11)) {
            sqlite3ErrorMsg(pParse, "conflicting ON CONFLICT clauses specified", 0);
          }
          if (pIdx->onError == 11) {
            pIdx->onError = pIndex->onError;
          }
        }
        if (idxType == 2)
          pIdx->idxType = idxType;
        if ((pParse->eParseMode >= 2)) {
          pIndex->pNext = pParse->pNewIndex;
          pParse->pNewIndex = pIndex;
          pIndex = 0;
        }
        goto exit_create_index;
      }
    }
  }

  if (!(pParse->eParseMode >= 2)) {
    if (db->init.busy) {
      Index *p;

      if (pTblName != 0) {
        pIndex->tnum = db->init.newTnum;
        if (sqlite3IndexHasDuplicateRootPage(pIndex)) {
          sqlite3ErrorMsg(pParse, "invalid rootpage");
          pParse->rc = sqlite3CorruptError(130938);
          goto exit_create_index;
        }
      }
      p = (Index*)(sqlite3HashInsert(&pIndex->pSchema->idxHash, pIndex->zName, pIndex));
      if (p) {
        sqlite3OomFault(db);
        goto exit_create_index;
      }
      db->mDbFlags |= 0x0001;
    }

    else if ((((pTab)->tabFlags & 0x00000080) == 0) || pTblName != 0) {
      Vdbe *v;
      char *zStmt;
      int iMem = ++pParse->nMem;

      v = sqlite3GetVdbe(pParse);
      if (v == 0)
        goto exit_create_index;

      sqlite3BeginWriteOperation(pParse, 1, iDb);

      pIndex->tnum = (Pgno)sqlite3VdbeAddOp0(v, 189);
      sqlite3VdbeAddOp3(v, 149, iDb, iMem, 2);

      if (pStart) {
        int n = (int)(pParse->sLastToken.z - pName->z) + pParse->sLastToken.n;
        if (pName->z[n - 1] == ';')
          n--;

        zStmt = sqlite3MPrintf(db, "CREATE%s INDEX %.*s", onError == 0 ? "" : " UNIQUE", n, pName->z);
      } else {
        zStmt = 0;
      }

      sqlite3NestedParse(pParse,
                         "INSERT INTO %Q."
                         "sqlite_master"
                         " VALUES('index',%Q,%Q,#%d,%Q);",
                         db->aDb[iDb].zDbSName, pIndex->zName, pTab->zName, iMem, zStmt);
      sqlite3DbFree(db, zStmt);

      if (pTblName) {
        sqlite3RefillIndex(pParse, pIndex, iMem);
        sqlite3ChangeCookie(pParse, iDb);
        sqlite3VdbeAddParseSchemaOp(v, iDb, sqlite3MPrintf(db, "name='%q' AND type='index'", pIndex->zName), 0);
        sqlite3VdbeAddOp2(v, 168, 0, 1);
      }

      sqlite3VdbeJumpHere(v, (int)pIndex->tnum);
    }
  }
  if (db->init.busy || pTblName == 0) {
    pIndex->pNext = pTab->pIndex;
    pTab->pIndex = pIndex;
    pIndex = 0;
  } else if ((pParse->eParseMode >= 2)) {
    pParse->pNewIndex = pIndex;
    pIndex = 0;
  }

exit_create_index:
  if (pIndex)
    sqlite3FreeIndex(db, pIndex);
  if (pTab) {
    Index **ppFrom;
    Index *pThis;
    for (ppFrom = &pTab->pIndex; (pThis = *ppFrom) != 0; ppFrom = &pThis->pNext) {
      Index *pNext;
      if (pThis->onError != 5)
        continue;
      while ((pNext = pThis->pNext) != 0 && pNext->onError != 5) {
        *ppFrom = pNext;
        pThis->pNext = pNext->pNext;
        pNext->pNext = pThis;
        ppFrom = &pNext->pNext;
      }
      break;
    }
  }
  sqlite3ExprDelete(db, pPIWhere);
  sqlite3ExprListDelete(db, pList);
  sqlite3SrcListDelete(db, pTblName);
  sqlite3DbFree(db, zName);
}

void sqlite3DropIndex(Parse *pParse, SrcList *pName, int ifExists) {
  Index *pIndex;
  Vdbe *v;
  sqlite3 *db = pParse->db;
  int iDb;

  if (db->mallocFailed) {
    goto exit_drop_index;
  }

  if (SQLITE_OK != sqlite3ReadSchema(pParse)) {
    goto exit_drop_index;
  }
  pIndex = sqlite3FindIndex(db, pName->a[0].zName, pName->a[0].u4.zDatabase);
  if (pIndex == 0) {
    if (!ifExists) {
      sqlite3ErrorMsg(pParse, "no such index: %S", pName->a);
    } else {
      sqlite3CodeVerifyNamedSchema(pParse, pName->a[0].u4.zDatabase);
      sqlite3ForceNotReadOnly(pParse);
    }
    pParse->checkSchema = 1;
    goto exit_drop_index;
  }
  if (pIndex->idxType != 0) {
    sqlite3ErrorMsg(pParse,
                    "index associated with UNIQUE "
                    "or PRIMARY KEY constraint cannot be dropped",
                    0);
    goto exit_drop_index;
  }
  iDb = sqlite3SchemaToIndex(db, pIndex->pSchema);

  {
    int code = SQLITE_DROP_INDEX;
    Table *pTab = pIndex->pTable;
    const char *zDb = db->aDb[iDb].zDbSName;
    const char *zTab = ((!0) && (iDb == 1) ? "sqlite_temp_master" : "sqlite_master");
    if (sqlite3AuthCheck(pParse, SQLITE_DELETE, zTab, 0, zDb)) {
      goto exit_drop_index;
    }
    if (!0 && iDb == 1)
      code = SQLITE_DROP_TEMP_INDEX;
    if (sqlite3AuthCheck(pParse, code, pIndex->zName, pTab->zName, zDb)) {
      goto exit_drop_index;
    }
  }

  v = sqlite3GetVdbe(pParse);
  if (v) {
    sqlite3BeginWriteOperation(pParse, 1, iDb);
    sqlite3NestedParse(pParse,
                       "DELETE FROM %Q."
                       "sqlite_master"
                       " WHERE name=%Q AND type='index'",
                       db->aDb[iDb].zDbSName, pIndex->zName);
    sqlite3ClearStatTables(pParse, iDb, "idx", pIndex->zName);
    sqlite3ChangeCookie(pParse, iDb);
    destroyRootPage(pParse, pIndex->tnum, iDb);
    sqlite3VdbeAddOp4(v, 155, iDb, 0, 0, pIndex->zName, 0);
  }

exit_drop_index:
  sqlite3SrcListDelete(db, pName);
}

IdList *sqlite3IdListAppend(Parse *pParse, IdList *pList, Token *pToken) {
  sqlite3 *db = pParse->db;
  int i;
  if (pList == 0) {
    pList = (IdList*)(sqlite3DbMallocZero(db, (offsetof(IdList, a) + (1) * sizeof(struct IdList_item))));
    if (pList == 0)
      return 0;
  } else {
    IdList *pNew;
    pNew = (IdList*)(sqlite3DbRealloc(db, pList, (offsetof(IdList, a) + (pList->nId + 1) * sizeof(struct IdList_item))));
    if (pNew == 0) {
      sqlite3IdListDelete(db, pList);
      return 0;
    }
    pList = pNew;
  }
  i = pList->nId++;
  pList->a[i].zName = sqlite3NameFromToken(db, pToken);
  if ((pParse->eParseMode >= 2) && pList->a[i].zName) {
    sqlite3RenameTokenMap(pParse, (void *)pList->a[i].zName, pToken);
  }
  return pList;
}

SrcList *sqlite3SrcListEnlarge(Parse *pParse, SrcList *pSrc, int nExtra, int iStart) {
  int i;

  if ((u32)pSrc->nSrc + nExtra > pSrc->nAlloc) {
    SrcList *pNew;
    sqlite3_int64 nAlloc = 2 * (sqlite3_int64)pSrc->nSrc + nExtra;
    sqlite3 *db = pParse->db;

    if (pSrc->nSrc + nExtra >= 200) {
      sqlite3ErrorMsg(pParse, "too many FROM clause terms, max: %d", 200);
      return 0;
    }
    if (nAlloc > 200)
      nAlloc = 200;
    pNew = (SrcList*)(sqlite3DbRealloc(db, pSrc, (offsetof(SrcList, a) + (nAlloc) * sizeof(SrcItem))));
    if (pNew == 0) {
      return 0;
    }
    pSrc = pNew;
    pSrc->nAlloc = nAlloc;
  }

  for (i = pSrc->nSrc - 1; i >= iStart; i--) {
    pSrc->a[i + nExtra] = pSrc->a[i];
  }
  pSrc->nSrc += nExtra;

  memset(&pSrc->a[iStart], 0, sizeof(pSrc->a[0]) * nExtra);
  for (i = iStart; i < iStart + nExtra; i++) {
    pSrc->a[i].iCursor = -1;
  }

  return pSrc;
}

SrcList *sqlite3SrcListAppend(Parse *pParse, SrcList *pList, Token *pTable, Token *pDatabase) {
  SrcItem *pItem;
  sqlite3 *db;

  db = pParse->db;
  if (pList == 0) {
    pList = (SrcList*)(sqlite3DbMallocRawNN(pParse->db, (offsetof(SrcList, a) + (1) * sizeof(SrcItem))));
    if (pList == 0)
      return 0;
    pList->nAlloc = 1;
    pList->nSrc = 1;
    memset(&pList->a[0], 0, sizeof(pList->a[0]));
    pList->a[0].iCursor = -1;
  } else {
    SrcList *pNew = sqlite3SrcListEnlarge(pParse, pList, 1, pList->nSrc);
    if (pNew == 0) {
      sqlite3SrcListDelete(db, pList);
      return 0;
    } else {
      pList = pNew;
    }
  }
  pItem = &pList->a[pList->nSrc - 1];
  if (pDatabase && pDatabase->z == 0) {
    pDatabase = 0;
  }

  if (pDatabase) {
    pItem->zName = sqlite3NameFromToken(db, pDatabase);
    pItem->u4.zDatabase = sqlite3NameFromToken(db, pTable);
  } else {
    pItem->zName = sqlite3NameFromToken(db, pTable);
    pItem->u4.zDatabase = 0;
  }
  return pList;
}

void sqlite3SrcListAssignCursors(Parse *pParse, SrcList *pList) {
  int i;
  SrcItem *pItem;

  if ((pList)) {
    for (i = 0, pItem = pList->a; i < pList->nSrc; i++, pItem++) {
      if (pItem->iCursor >= 0)
        continue;
      pItem->iCursor = pParse->nTab++;
      if (pItem->fg.isSubquery) {
        sqlite3SrcListAssignCursors(pParse, pItem->u4.pSubq->pSelect->pSrc);
      }
    }
  }
}

int sqlite3SrcItemAttachSubquery(Parse *pParse, SrcItem *pItem, Select *pSelect, int dupSelect) {
  Subquery *p;

  if (pItem->fg.fixedSchema) {
    pItem->u4.pSchema = 0;
    pItem->fg.fixedSchema = 0;
  } else if (pItem->u4.zDatabase != 0) {
    sqlite3DbFree(pParse->db, pItem->u4.zDatabase);
    pItem->u4.zDatabase = 0;
  }
  if (dupSelect) {
    pSelect = sqlite3SelectDup(pParse->db, pSelect, 0);
    if (pSelect == 0)
      return 0;
  }
  p = pItem->u4.pSubq = (Subquery*)(sqlite3DbMallocRawNN(pParse->db, sizeof(Subquery)));
  if (p == 0) {
    sqlite3SelectDelete(pParse->db, pSelect);
    return 0;
  }
  pItem->fg.isSubquery = 1;
  p->pSelect = pSelect;

  memset(((char *)p) + sizeof(p->pSelect), 0, sizeof(*p) - sizeof(p->pSelect));
  return 1;
}

SrcList *sqlite3SrcListAppendFromTerm(Parse *pParse, SrcList *p, Token *pTable, Token *pDatabase, Token *pAlias,
                                      Select *pSubquery, OnOrUsing *pOnUsing) {
  SrcItem *pItem;
  sqlite3 *db = pParse->db;
  if (!p && pOnUsing != 0 && (pOnUsing->pOn || pOnUsing->pUsing)) {
    sqlite3ErrorMsg(pParse, "a JOIN clause is required before %s", (pOnUsing->pOn ? "ON" : "USING"));
    goto append_from_error;
  }
  p = sqlite3SrcListAppend(pParse, p, pTable, pDatabase);
  if (p == 0) {
    goto append_from_error;
  }

  pItem = &p->a[p->nSrc - 1];

  if ((pParse->eParseMode >= 2) && pItem->zName) {
    Token *pToken = ((pDatabase) && pDatabase->z) ? pDatabase : pTable;
    sqlite3RenameTokenMap(pParse, pItem->zName, pToken);
  }

  if (pAlias->n) {
    pItem->zAlias = sqlite3NameFromToken(db, pAlias);
  }

  if (pSubquery) {
    if (sqlite3SrcItemAttachSubquery(pParse, pItem, pSubquery, 0)) {
      if (pSubquery->selFlags & 0x0000800) {
        pItem->fg.isNestedFrom = 1;
      }
    }
  }

  if (pOnUsing == 0) {
    pItem->u3.pOn = 0;
  } else if (pOnUsing->pUsing) {
    pItem->fg.isUsing = 1;
    pItem->u3.pUsing = pOnUsing->pUsing;
  } else {
    pItem->u3.pOn = pOnUsing->pOn;
  }
  return p;

append_from_error:
  sqlite3ClearOnOrUsing(db, pOnUsing);
  sqlite3SelectDelete(db, pSubquery);
  return 0;
}

void sqlite3SrcListIndexedBy(Parse *pParse, SrcList *p, Token *pIndexedBy) {
  if (p && pIndexedBy->n > 0) {
    SrcItem *pItem;

    pItem = &p->a[p->nSrc - 1];

    if (pIndexedBy->n == 1 && !pIndexedBy->z) {
      pItem->fg.notIndexed = 1;
    } else {
      pItem->u1.zIndexedBy = sqlite3NameFromToken(pParse->db, pIndexedBy);
      pItem->fg.isIndexedBy = 1;
    }
  }
}

SrcList *sqlite3SrcListAppendList(Parse *pParse, SrcList *p1, SrcList *p2) {
  if (p2) {
    int nOld = p1->nSrc;
    SrcList *pNew = sqlite3SrcListEnlarge(pParse, p1, p2->nSrc, nOld);
    if (pNew == 0) {
      sqlite3SrcListDelete(pParse->db, p2);
    } else {
      p1 = pNew;
      memcpy(&p1->a[nOld], p2->a, p2->nSrc * sizeof(SrcItem));

      p1->a[0].fg.jointype |= (0x40 & p2->a[0].fg.jointype);
      sqlite3DbFree(pParse->db, p2);
    }
  }
  return p1;
}

void sqlite3SrcListFuncArgs(Parse *pParse, SrcList *p, ExprList *pList) {
  if (p) {
    SrcItem *pItem = &p->a[p->nSrc - 1];

    pItem->u1.pFuncArg = pList;
    pItem->fg.isTabFunc = 1;
  } else {
    sqlite3ExprListDelete(pParse->db, pList);
  }
}

void sqlite3SrcListShiftJoinType(Parse *pParse, SrcList *p) {
  (void)pParse;
  if (p && p->nSrc > 1) {
    int i = p->nSrc - 1;
    u8 allFlags = 0;
    do {
      allFlags |= p->a[i].fg.jointype = p->a[i - 1].fg.jointype;
    } while ((--i) > 0);
    p->a[0].fg.jointype = 0;

    if (allFlags & 0x10) {
      for (i = p->nSrc - 1; (i > 0) && (p->a[i].fg.jointype & 0x10) == 0; i--) {
      }
      i--;

      do {
        p->a[i].fg.jointype |= 0x40;
      } while ((--i) >= 0);
    }
  }
}

void sqlite3BeginTransaction(Parse *pParse, int type) {
  sqlite3 *db;
  Vdbe *v;
  int i;

  db = pParse->db;

  if (sqlite3AuthCheck(pParse, SQLITE_TRANSACTION, "BEGIN", 0, 0)) {
    return;
  }
  v = sqlite3GetVdbe(pParse);
  if (!v)
    return;
  if (type != 7) {
    for (i = 0; i < db->nDb; i++) {
      int eTxnType;
      Btree *pBt = db->aDb[i].pBt;
      if (pBt && sqlite3BtreeIsReadonly(pBt)) {
        eTxnType = 0;
      } else if (type == 9) {
        eTxnType = 2;
      } else {
        eTxnType = 1;
      }
      sqlite3VdbeAddOp2(v, 2, i, eTxnType);
      sqlite3VdbeUsesBtree(v, i);
    }
  }
  sqlite3VdbeAddOp0(v, 1);
}

void sqlite3EndTransaction(Parse *pParse, int eType) {
  Vdbe *v;
  int isRollback;

  isRollback = eType == 12;
  if (sqlite3AuthCheck(pParse, SQLITE_TRANSACTION, isRollback ? "ROLLBACK" : "COMMIT", 0, 0)) {
    return;
  }
  v = sqlite3GetVdbe(pParse);
  if (v) {
    sqlite3VdbeAddOp2(v, 1, 1, isRollback);
  }
}

void sqlite3Savepoint(Parse *pParse, int op, Token *pName) {
  char *zName = sqlite3NameFromToken(pParse->db, pName);
  if (zName) {
    Vdbe *v = sqlite3GetVdbe(pParse);

    static const char *const az[] = {"BEGIN", "RELEASE", "ROLLBACK"};

    if (!v || sqlite3AuthCheck(pParse, SQLITE_SAVEPOINT, az[op], zName, 0)) {
      sqlite3DbFree(pParse->db, zName);
      return;
    }
    sqlite3VdbeAddOp4(v, 0, op, 0, 0, zName, (-7));
  }
}

int sqlite3OpenTempDatabase(Parse *pParse) {
  sqlite3 *db = pParse->db;
  if (db->aDb[1].pBt == 0 && !pParse->explain) {
    int rc;
    Btree *pBt;
    static const int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXCLUSIVE |
                             SQLITE_OPEN_DELETEONCLOSE | SQLITE_OPEN_TEMP_DB;

    rc = sqlite3BtreeOpen(db->pVfs, 0, db, &pBt, 0, flags);
    if (rc != SQLITE_OK) {
      sqlite3ErrorMsg(pParse,
                      "unable to open a temporary database "
                      "file for storing temporary tables");
      pParse->rc = rc;
      return 1;
    }
    db->aDb[1].pBt = pBt;

    if (SQLITE_NOMEM == sqlite3BtreeSetPageSize(pBt, db->nextPagesize, 0, 0)) {
      sqlite3OomFault(db);
      return 1;
    }
  }
  return 0;
}

void sqlite3CodeVerifySchemaAtToplevel(Parse *pToplevel, int iDb) {
  if ((((pToplevel->cookieMask) & (((yDbMask)1) << (iDb))) != 0) == 0) {
    ((pToplevel->cookieMask) |= (((yDbMask)1) << (iDb)));
    if (!0 && iDb == 1) {
      sqlite3OpenTempDatabase(pToplevel);
    }
  }
}

void sqlite3CodeVerifySchema(Parse *pParse, int iDb) {
  sqlite3CodeVerifySchemaAtToplevel(((pParse)->pToplevel ? (pParse)->pToplevel : (pParse)), iDb);
}

void sqlite3CodeVerifyNamedSchema(Parse *pParse, const char *zDb) {
  sqlite3 *db = pParse->db;
  int i;
  for (i = 0; i < db->nDb; i++) {
    Db *pDb = &db->aDb[i];
    if (pDb->pBt && (!zDb || 0 == sqlite3StrICmp(zDb, pDb->zDbSName))) {
      sqlite3CodeVerifySchema(pParse, i);
    }
  }
}

void sqlite3BeginWriteOperation(Parse *pParse, int setStatement, int iDb) {
  Parse *pToplevel = ((pParse)->pToplevel ? (pParse)->pToplevel : (pParse));
  sqlite3CodeVerifySchemaAtToplevel(pToplevel, iDb);
  ((pToplevel->writeMask) |= (((yDbMask)1) << (iDb)));
  pToplevel->isMultiWrite |= setStatement;
}

void sqlite3MultiWrite(Parse *pParse) {
  Parse *pToplevel = ((pParse)->pToplevel ? (pParse)->pToplevel : (pParse));
  pToplevel->isMultiWrite = 1;
}

void sqlite3MayAbort(Parse *pParse) {
  Parse *pToplevel = ((pParse)->pToplevel ? (pParse)->pToplevel : (pParse));
  pToplevel->mayAbort = 1;
}

void sqlite3HaltConstraint(Parse *pParse, int errCode, int onError, char *p4, i8 p4type, u8 p5Errmsg) {
  Vdbe *v;

  v = sqlite3GetVdbe(pParse);

  if (onError == 2) {
    sqlite3MayAbort(pParse);
  }
  sqlite3VdbeAddOp4(v, 72, errCode, onError, 0, p4, p4type);
  sqlite3VdbeChangeP5(v, p5Errmsg);
}

void sqlite3UniqueConstraint(Parse *pParse, int onError, Index *pIdx) {
  char *zErr;
  int j;
  StrAccum errMsg;
  Table *pTab = pIdx->pTable;

  sqlite3StrAccumInit(&errMsg, pParse->db, 0, 0, pParse->db->aLimit[SQLITE_LIMIT_LENGTH]);
  if (pIdx->aColExpr) {
    sqlite3_str_appendf(&errMsg, "index '%q'", pIdx->zName);
  } else {
    for (j = 0; j < pIdx->nKeyCol; j++) {
      char *zCol;

      zCol = pTab->aCol[pIdx->aiColumn[j]].zCnName;
      if (j)
        sqlite3_str_append(&errMsg, ", ", 2);
      sqlite3_str_appendall(&errMsg, pTab->zName);
      sqlite3_str_append(&errMsg, ".", 1);
      sqlite3_str_appendall(&errMsg, zCol);
    }
  }
  zErr = sqlite3StrAccumFinish(&errMsg);
  sqlite3HaltConstraint(pParse, ((pIdx)->idxType == 2) ? (19 | (6 << 8)) : (19 | (8 << 8)), onError, zErr, (-7), 2);
}

void sqlite3RowidConstraint(Parse *pParse, int onError, Table *pTab) {
  char *zMsg;
  int rc;
  if (pTab->iPKey >= 0) {
    zMsg = sqlite3MPrintf(pParse->db, "%s.%s", pTab->zName, pTab->aCol[pTab->iPKey].zCnName);
    rc = (19 | (6 << 8));
  } else {
    zMsg = sqlite3MPrintf(pParse->db, "%s.rowid", pTab->zName);
    rc = (19 | (10 << 8));
  }
  sqlite3HaltConstraint(pParse, rc, onError, zMsg, (-7), 2);
}

void sqlite3Reindex(Parse *pParse, Token *pName1, Token *pName2) {
  char *z = 0;
  const char *zDb = 0;
  int iReDb = -1;
  sqlite3 *db = pParse->db;
  Token *pObjName;
  int bMatch = 0;
  const char *zColl = 0;
  Table *pReTab = 0;
  Index *pReIndex = 0;
  int isExprIdx = 0;
  int bAll = 0;

  if (SQLITE_OK != sqlite3ReadSchema(pParse)) {
    return;
  }

  if (pName1 == 0) {
    bMatch = 1;
    bAll = 1;
  } else if ((pName2 == 0) || pName2->z == 0) {
    z = sqlite3NameFromToken(pParse->db, pName1);
    if (z == 0)
      return;
  } else {
    iReDb = sqlite3TwoPartName(pParse, pName1, pName2, &pObjName);
    if (iReDb < 0)
      return;
    z = sqlite3NameFromToken(db, pObjName);
    if (z == 0)
      return;
    zDb = db->aDb[iReDb].zDbSName;
  }
  if (!bAll) {
    if (zDb == 0 && sqlite3StrICmp(z, "expressions") == 0) {
      isExprIdx = 1;
      bMatch = 1;
    }
    if (zDb == 0 && sqlite3FindCollSeq(db, ((db)->enc), z, 0) != 0) {
      zColl = z;
      bMatch = 1;
    }
    if (zColl == 0 && (pReTab = sqlite3FindTable(db, z, zDb)) != 0) {
      bMatch = 1;
    }
    if (zColl == 0 && (pReIndex = sqlite3FindIndex(db, z, zDb)) != 0) {
      bMatch = 1;
    }
  }
  if (bMatch) {
    int iDb;
    HashElem *k;
    Table *pTab;
    Index *pIdx;
    Db *pDb;
    for (iDb = 0, pDb = db->aDb; iDb < db->nDb; iDb++, pDb++) {
      if (iReDb >= 0 && iReDb != iDb)
        continue;
      for (k = ((&pDb->pSchema->tblHash)->first); k; k = ((k)->next)) {
        pTab = (Table *)((k)->data);
        if ((pTab)->eTabType == 1)
          continue;
        for (pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext) {
          if (bAll || pTab == pReTab || pIdx == pReIndex || (isExprIdx && pIdx->bHasExpr) ||
              (zColl != 0 && collationMatch(zColl, pIdx))) {
            sqlite3BeginWriteOperation(pParse, 0, iDb);
            sqlite3RefillIndex(pParse, pIdx, -1);
          }
        }
      }
    }
  } else {
    sqlite3ErrorMsg(pParse, "unable to identify the object to be reindexed");
  }
  sqlite3DbFree(db, z);
  return;
}

KeyInfo *sqlite3KeyInfoOfIndex(Parse *pParse, Index *pIdx) {
  int i;
  int nCol = pIdx->nColumn;
  int nKey = pIdx->nKeyCol;
  KeyInfo *pKey;
  if (pParse->nErr)
    return 0;
  if (pIdx->uniqNotNull) {
    pKey = sqlite3KeyInfoAlloc(pParse->db, nKey, nCol - nKey);
  } else {
    pKey = sqlite3KeyInfoAlloc(pParse->db, nCol, 0);
  }
  if (pKey) {
    for (i = 0; i < nCol; i++) {
      const char *zColl = pIdx->azColl[i];
      pKey->aColl[i] = zColl == sqlite3StrBINARY ? 0 : sqlite3LocateCollSeq(pParse, zColl);
      pKey->aSortFlags[i] = pIdx->aSortOrder[i];
    }
    if (pParse->nErr) {
      if (pIdx->bNoQuery == 0 && sqlite3HashFind(&pIdx->pSchema->idxHash, pIdx->zName)) {
        pIdx->bNoQuery = 1;
        pParse->rc = (1 | (2 << 8));
      }
      sqlite3KeyInfoUnref(pKey);
      pKey = 0;
    }
  }
  return pKey;
}

Cte *sqlite3CteNew(Parse *pParse, Token *pName, ExprList *pArglist, Select *pQuery, u8 eM10d) {
  Cte *pNew;
  sqlite3 *db = pParse->db;

  pNew = (Cte*)(sqlite3DbMallocZero(db, sizeof(*pNew)));

  if (db->mallocFailed) {
    sqlite3ExprListDelete(db, pArglist);
    sqlite3SelectDelete(db, pQuery);
  } else {
    pNew->pSelect = pQuery;
    pNew->pCols = pArglist;
    pNew->zName = sqlite3NameFromToken(pParse->db, pName);
    pNew->eM10d = eM10d;
  }
  return pNew;
}

With *sqlite3WithAdd(Parse *pParse, With *pWith, Cte *pCte) {
  sqlite3 *db = pParse->db;
  With *pNew;
  char *zName;

  if (pCte == 0) {
    return pWith;
  }

  zName = pCte->zName;
  if (zName && pWith) {
    int i;
    for (i = 0; i < pWith->nCte; i++) {
      if (sqlite3StrICmp(zName, pWith->a[i].zName) == 0) {
        sqlite3ErrorMsg(pParse, "duplicate WITH table name: %s", zName);
      }
    }
  }

  if (pWith) {
    pNew = (With*)(sqlite3DbRealloc(db, pWith, (offsetof(With, a) + (pWith->nCte + 1) * sizeof(Cte))));
  } else {
    pNew = (With*)(sqlite3DbMallocZero(db, (offsetof(With, a) + (1) * sizeof(Cte))));
  }

  if (db->mallocFailed) {
    sqlite3CteDelete(db, pCte);
    pNew = pWith;
  } else {
    pNew->a[pNew->nCte++] = *pCte;
    sqlite3DbFree(db, pCte);
  }

  return pNew;
}

int sqlite3CheckCollSeq(Parse *pParse, CollSeq *pColl) {
  if (pColl && pColl->xCmp == 0) {
    const char *zName = pColl->zName;
    sqlite3 *db = pParse->db;
    CollSeq *p = sqlite3GetCollSeq(pParse, ((db)->enc), pColl, zName);
    if (!p) {
      return SQLITE_ERROR;
    }
  }
  return 0;
}

CollSeq *sqlite3GetCollSeq(Parse *pParse, u8 enc, CollSeq *pColl, const char *zName) {
  CollSeq *p;
  sqlite3 *db = pParse->db;

  p = pColl;
  if (!p) {
    p = sqlite3FindCollSeq(db, enc, zName, 0);
  }
  if (!p || !p->xCmp) {
    callCollNeeded(db, enc, zName);
    p = sqlite3FindCollSeq(db, enc, zName, 0);
  }
  if (p && !p->xCmp && synthCollSeq(db, p)) {
    p = 0;
  }

  if (p == 0) {
    sqlite3ErrorMsg(pParse, "no such collation sequence: %s", zName);
    pParse->rc = (1 | (1 << 8));
  }
  return p;
}

CollSeq *sqlite3LocateCollSeq(Parse *pParse, const char *zName) {
  sqlite3 *db = pParse->db;
  u8 enc = ((db)->enc);
  u8 initbusy = db->init.busy;
  CollSeq *pColl;

  pColl = sqlite3FindCollSeq(db, enc, zName, initbusy);
  if (!initbusy && (!pColl || !pColl->xCmp)) {
    pColl = sqlite3GetCollSeq(pParse, enc, pColl, zName);
  }

  return pColl;
}

Table *sqlite3SrcListLookup(Parse *pParse, SrcList *pSrc) {
  SrcItem *pItem = pSrc->a;
  Table *pTab;

  pTab = sqlite3LocateTableItem(pParse, 0, pItem);
  if (pItem->pSTab)
    sqlite3DeleteTable(pParse->db, pItem->pSTab);
  pItem->pSTab = pTab;
  pItem->fg.notCte = 1;
  if (pTab) {
    pTab->nTabRef++;
    if (pItem->fg.isIndexedBy && sqlite3IndexedByLookup(pParse, pItem)) {
      pTab = 0;
    }
  }
  return pTab;
}

int vtabIsReadOnly(Parse *pParse, Table *pTab) {
  if (sqlite3GetVTable(pParse->db, pTab)->pMod->pModule->xUpdate == 0) {
    return 1;
  }

  if ((pParse->pToplevel != 0 || (pParse->prepFlags & SQLITE_PREPARE_FROM_DDL)) &&
      pTab->u.vtab.p->eVtabRisk > ((pParse->db->flags & 0x00000080) != 0)) {
    sqlite3ErrorMsg(pParse, "unsafe use of virtual table \"%s\"", pTab->zName);
  }
  return 0;
}

int tabIsReadOnly(Parse *pParse, Table *pTab) {
  sqlite3 *db;
  if ((pTab)->eTabType == 1) {
    return vtabIsReadOnly(pParse, pTab);
  }
  if ((pTab->tabFlags & (0x00000001 | 0x00001000)) == 0)
    return 0;
  db = pParse->db;
  if ((pTab->tabFlags & 0x00000001) != 0) {
    return sqlite3WritableSchema(db) == 0 && pParse->nested == 0;
  }

  return sqlite3ReadOnlyShadowTables(db);
}

int sqlite3IsReadOnly(Parse *pParse, Table *pTab, Trigger *pTrigger) {
  if (tabIsReadOnly(pParse, pTab)) {
    sqlite3ErrorMsg(pParse, "table %s may not be modified", pTab->zName);
    return 1;
  }

  if (((pTab)->eTabType == 2) && (pTrigger == 0 || (pTrigger->bReturning && pTrigger->pNext == 0))) {
    sqlite3ErrorMsg(pParse, "cannot modify %s because it is a view", pTab->zName);
    return 1;
  }

  return 0;
}

void sqlite3MaterializeView(Parse *pParse, Table *pView, Expr *pWhere, ExprList *pOrderBy, Expr *pLimit, int iCur) {
  SelectDest dest;
  Select *pSel;
  SrcList *pFrom;
  sqlite3 *db = pParse->db;
  int iDb = sqlite3SchemaToIndex(db, pView->pSchema);
  pWhere = sqlite3ExprDup(db, pWhere, 0);
  pFrom = sqlite3SrcListAppend(pParse, 0, 0, 0);
  if (pFrom) {
    pFrom->a[0].zName = sqlite3DbStrDup(db, pView->zName);

    pFrom->a[0].u4.zDatabase = sqlite3DbStrDup(db, db->aDb[iDb].zDbSName);
  }
  pSel = sqlite3SelectNew(pParse, 0, pFrom, pWhere, 0, 0, pOrderBy, 0x0020000, pLimit);
  sqlite3SelectDestInit(&dest, 10, iCur);
  sqlite3Select(pParse, pSel, &dest);
  sqlite3SelectDelete(db, pSel);
}

void sqlite3DeleteFrom(Parse *pParse, SrcList *pTabList, Expr *pWhere, ExprList *pOrderBy, Expr *pLimit) {
  Vdbe *v;
  Table *pTab;
  int i;
  WhereInfo *pWInfo;
  Index *pIdx;
  int iTabCur;
  int iDataCur = 0;
  int iIdxCur = 0;
  int nIdx;
  sqlite3 *db;
  AuthContext sContext;
  NameContext sNC;
  int iDb;
  int memCnt = 0;
  int rcauth;
  int eOnePass;
  int aiCurOnePass[2];
  u8 *aToOpen = 0;
  Index *pPk;
  int iPk = 0;
  i16 nPk = 1;
  int iKey;
  i16 nKey;
  int iEphCur = 0;
  int iRowSet = 0;
  int addrBypass = 0;
  int addrLoop = 0;
  int addrEphOpen = 0;
  int bComplex;

  int isView;
  Trigger *pTrigger;

  memset(&sContext, 0, sizeof(sContext));
  db = pParse->db;

  if (pParse->nErr) {
    goto delete_from_cleanup;
  }

  pTab = sqlite3SrcListLookup(pParse, pTabList);
  if (pTab == 0)
    goto delete_from_cleanup;

  pTrigger = sqlite3TriggersExist(pParse, pTab, 129, 0, 0);
  isView = ((pTab)->eTabType == 2);

  bComplex = pTrigger || sqlite3FkRequired(pParse, pTab, 0, 0);

  if (sqlite3ViewGetColumnNames(pParse, pTab)) {
    goto delete_from_cleanup;
  }

  if (sqlite3IsReadOnly(pParse, pTab, pTrigger)) {
    goto delete_from_cleanup;
  }
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);

  rcauth = sqlite3AuthCheck(pParse, SQLITE_DELETE, pTab->zName, 0, db->aDb[iDb].zDbSName);

  if (rcauth == SQLITE_DENY) {
    goto delete_from_cleanup;
  }

  iTabCur = pTabList->a[0].iCursor = pParse->nTab++;
  for (nIdx = 0, pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext, nIdx++) {
    pParse->nTab++;
  }

  if (isView) {
    sqlite3AuthContextPush(pParse, &sContext, pTab->zName);
  }

  v = sqlite3GetVdbe(pParse);
  if (v == 0) {
    goto delete_from_cleanup;
  }
  if (pParse->nested == 0)
    sqlite3VdbeCountChanges(v);
  sqlite3BeginWriteOperation(pParse, bComplex, iDb);

  if (isView) {
    sqlite3MaterializeView(pParse, pTab, pWhere, pOrderBy, pLimit, iTabCur);
    iDataCur = iIdxCur = iTabCur;
    pOrderBy = 0;
    pLimit = 0;
  }

  memset(&sNC, 0, sizeof(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  if (sqlite3ResolveExprNames(&sNC, pWhere)) {
    goto delete_from_cleanup;
  }

  if ((db->flags & ((u64)(0x00001) << 32)) != 0 && !pParse->nested && !pParse->pTriggerTab && !pParse->bReturning) {
    memCnt = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, 73, 0, memCnt);
  }

  if (rcauth == SQLITE_OK && pWhere == 0 && !bComplex && !((pTab)->eTabType == 1)) {
    sqlite3TableLock(pParse, iDb, pTab->tnum, 1, pTab->zName);
    if ((((pTab)->tabFlags & 0x00000080) == 0)) {
      sqlite3VdbeAddOp4(v, 147, pTab->tnum, iDb, memCnt ? memCnt : -1, pTab->zName, (-1));
    }
    for (pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext) {
      if (((pIdx)->idxType == 2) && !(((pTab)->tabFlags & 0x00000080) == 0)) {
        sqlite3VdbeAddOp3(v, 147, pIdx->tnum, iDb, memCnt ? memCnt : -1);
      } else {
        sqlite3VdbeAddOp2(v, 147, pIdx->tnum, iDb);
      }
    }
  } else {
    u16 wcf = 0x0004 | 0x0010;
    if (sNC.ncFlags & 0x000040)
      bComplex = 1;
    wcf |= (bComplex ? 0 : 0x0008);
    if ((((pTab)->tabFlags & 0x00000080) == 0)) {
      pPk = 0;

      iRowSet = ++pParse->nMem;
      sqlite3VdbeAddOp2(v, 77, 0, iRowSet);
    } else {
      pPk = sqlite3PrimaryKeyIndex(pTab);

      nPk = pPk->nKeyCol;
      iPk = pParse->nMem + 1;
      pParse->nMem += nPk;
      iEphCur = pParse->nTab++;
      addrEphOpen = sqlite3VdbeAddOp2(v, 120, iEphCur, nPk);
      sqlite3VdbeSetP4KeyInfo(pParse, pPk);
    }

    pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, 0, 0, 0, wcf, iTabCur + 1);
    if (pWInfo == 0)
      goto delete_from_cleanup;
    eOnePass = sqlite3WhereOkOnePass(pWInfo, aiCurOnePass);

    if (eOnePass != 1)
      sqlite3MultiWrite(pParse);
    if (sqlite3WhereUsesDeferredSeek(pWInfo)) {
      sqlite3VdbeAddOp1(v, 145, iTabCur);
    }

    if (memCnt) {
      sqlite3VdbeAddOp2(v, 88, memCnt, 1);
    }

    if (pPk) {
      for (i = 0; i < nPk; i++) {
        sqlite3ExprCodeGetColumnOfTable(v, pTab, iTabCur, pPk->aiColumn[i], iPk + i);
      }
      iKey = iPk;
    } else {
      iKey = ++pParse->nMem;
      sqlite3ExprCodeGetColumnOfTable(v, pTab, iTabCur, -1, iKey);
    }

    if (eOnePass != 0) {
      nKey = nPk;
      aToOpen = (u8*)(sqlite3DbMallocRawNN(db, nIdx + 2));
      if (aToOpen == 0) {
        sqlite3WhereEnd(pWInfo);
        goto delete_from_cleanup;
      }
      memset(aToOpen, 1, nIdx + 1);
      aToOpen[nIdx + 1] = 0;
      if (aiCurOnePass[0] >= 0)
        aToOpen[aiCurOnePass[0] - iTabCur] = 0;
      if (aiCurOnePass[1] >= 0)
        aToOpen[aiCurOnePass[1] - iTabCur] = 0;
      if (addrEphOpen)
        sqlite3VdbeChangeToNoop(v, addrEphOpen);
      addrBypass = sqlite3VdbeMakeLabel(pParse);
    } else {
      if (pPk) {
        iKey = ++pParse->nMem;
        nKey = 0;
        sqlite3VdbeAddOp4(v, 99, iPk, nPk, iKey, sqlite3IndexAffinityStr(pParse->db, pPk), nPk);
        sqlite3VdbeAddOp4Int(v, 140, iEphCur, iKey, iPk, nPk);
      } else {
        nKey = 1;
        sqlite3VdbeAddOp2(v, 158, iRowSet, iKey);
      }
      sqlite3WhereEnd(pWInfo);
    }

    if (!isView) {
      int iAddrOnce = 0;
      if (eOnePass == 2) {
        iAddrOnce = sqlite3VdbeAddOp0(v, 15);
      };
      sqlite3OpenTableAndIndices(pParse, pTab, 116, 0x08, iTabCur, aToOpen, &iDataCur, &iIdxCur);

      if (eOnePass == 2) {
        sqlite3VdbeJumpHereOrPopInst(v, iAddrOnce);
      }
    }

    if (eOnePass != 0) {
      if (!((pTab)->eTabType == 1) && aToOpen[iDataCur - iTabCur]) {
        sqlite3VdbeAddOp4Int(v, 28, iDataCur, addrBypass, iKey, nKey);
      }
    } else if (pPk) {
      addrLoop = sqlite3VdbeAddOp1(v, 36, iEphCur);
      if ((pTab)->eTabType == 1) {
        sqlite3VdbeAddOp3(v, 96, iEphCur, 0, iKey);
      } else {
        sqlite3VdbeAddOp2(v, 136, iEphCur, iKey);
      }

    } else {
      addrLoop = sqlite3VdbeAddOp3(v, 48, iRowSet, 0, iKey);
    }

    if ((pTab)->eTabType == 1) {
      const char *pVTab = (const char *)sqlite3GetVTable(db, pTab);
      sqlite3VtabMakeWritable(pParse, pTab);

      sqlite3MayAbort(pParse);
      if (eOnePass == 1) {
        sqlite3VdbeAddOp1(v, 124, iTabCur);
        if ((pParse)->pToplevel == 0) {
          pParse->isMultiWrite = 0;
        }
      }
      sqlite3VdbeAddOp4(v, 7, 0, 1, iKey, pVTab, (-12));
      sqlite3VdbeChangeP5(v, 2);
    } else {
      int count = (pParse->nested == 0);
      sqlite3GenerateRowDelete(pParse, pTab, pTrigger, iDataCur, iIdxCur, iKey, nKey, count, 11, eOnePass,
                               aiCurOnePass[1]);
    }

    if (eOnePass != 0) {
      sqlite3VdbeResolveLabel(v, addrBypass);
      sqlite3WhereEnd(pWInfo);
    } else if (pPk) {
      sqlite3VdbeAddOp2(v, 40, iEphCur, addrLoop + 1);
      sqlite3VdbeJumpHere(v, addrLoop);
    } else {
      sqlite3VdbeGoto(v, addrLoop);
      sqlite3VdbeJumpHere(v, addrLoop);
    }
  }

  if (pParse->nested == 0 && pParse->pTriggerTab == 0) {
    sqlite3AutoincrementEnd(pParse);
  }

  if (memCnt) {
    sqlite3CodeChangeCount(v, memCnt, "rows deleted");
  }

delete_from_cleanup:
  sqlite3AuthContextPop(&sContext);
  sqlite3SrcListDelete(db, pTabList);
  sqlite3ExprDelete(db, pWhere);

  if (aToOpen)
    sqlite3DbNNFreeNN(db, aToOpen);
  return;
}

void sqlite3GenerateRowDelete(Parse *pParse, Table *pTab, Trigger *pTrigger, int iDataCur, int iIdxCur, int iPk,
                              i16 nPk, u8 count, u8 onconf, u8 eMode, int iIdxNoSeek) {
  Vdbe *v = pParse->pVdbe;
  int iOld = 0;
  int iLabel;
  u8 opSeek;

  iLabel = sqlite3VdbeMakeLabel(pParse);
  opSeek = (((pTab)->tabFlags & 0x00000080) == 0) ? 31 : 28;
  if (eMode == 0) {
    sqlite3VdbeAddOp4Int(v, opSeek, iDataCur, iLabel, iPk, nPk);
  }

  if (sqlite3FkRequired(pParse, pTab, 0, 0) || pTrigger) {
    u32 mask;
    int iCol;
    int addrStart;

    mask = sqlite3TriggerColmask(pParse, pTrigger, 0, 0, 1 | 2, pTab, onconf);
    mask |= sqlite3FkOldmask(pParse, pTab);
    iOld = pParse->nMem + 1;
    pParse->nMem += (1 + pTab->nCol);

    sqlite3VdbeAddOp2(v, 82, iPk, iOld);
    for (iCol = 0; iCol < pTab->nCol; iCol++) {
      if (mask == 0xffffffff || (iCol <= 31 && (mask & (((unsigned int)1) << (iCol))) != 0)) {
        int kk = sqlite3TableColumnToStorage(pTab, iCol);
        sqlite3ExprCodeGetColumnOfTable(v, pTab, iDataCur, iCol, iOld + kk + 1);
      }
    }

    addrStart = sqlite3VdbeCurrentAddr(v);
    sqlite3CodeRowTrigger(pParse, pTrigger, 129, 0, 1, pTab, iOld, onconf, iLabel);

    if (addrStart < sqlite3VdbeCurrentAddr(v)) {
      sqlite3VdbeAddOp4Int(v, opSeek, iDataCur, iLabel, iPk, nPk);
      iIdxNoSeek = -1;
    }

    sqlite3FkCheck(pParse, pTab, iOld, 0, 0, 0);
  }

  if (!((pTab)->eTabType == 2)) {
    u8 p5 = 0;
    sqlite3GenerateRowIndexDelete(pParse, pTab, iDataCur, iIdxCur, 0, iIdxNoSeek);
    sqlite3VdbeAddOp2(v, 132, iDataCur, (count ? 0x01 : 0));
    if (pParse->nested == 0 || 0 == sqlite3_stricmp(pTab->zName, "sqlite_stat1")) {
      sqlite3VdbeAppendP4(v, (char *)pTab, (-5));
    }
    if (eMode != 0) {
      sqlite3VdbeChangeP5(v, 0x04);
    }
    if (iIdxNoSeek >= 0 && iIdxNoSeek != iDataCur) {
      sqlite3VdbeAddOp1(v, 132, iIdxNoSeek);
    }
    if (eMode == 2)
      p5 |= 0x02;
    sqlite3VdbeChangeP5(v, p5);
  }

  sqlite3FkActions(pParse, pTab, 0, iOld, 0, 0);

  if (pTrigger) {
    sqlite3CodeRowTrigger(pParse, pTrigger, 129, 0, 2, pTab, iOld, onconf, iLabel);
  }

  sqlite3VdbeResolveLabel(v, iLabel);
}

void sqlite3GenerateRowIndexDelete(Parse *pParse, Table *pTab, int iDataCur, int iIdxCur, int *aRegIdx,
                                   int iIdxNoSeek) {
  int i;
  int r1 = -1;
  int iPartIdxLabel;
  Index *pIdx;
  Index *pPrior = 0;
  Vdbe *v;
  Index *pPk;

  v = pParse->pVdbe;
  pPk = (((pTab)->tabFlags & 0x00000080) == 0) ? 0 : sqlite3PrimaryKeyIndex(pTab);
  for (i = 0, pIdx = pTab->pIndex; pIdx; i++, pIdx = pIdx->pNext) {
    if (aRegIdx != 0 && aRegIdx[i] == 0)
      continue;
    if (pIdx == pPk)
      continue;
    if (iIdxCur + i == iIdxNoSeek)
      continue;
    r1 = sqlite3GenerateIndexKey(pParse, pIdx, iDataCur, 0, 1, &iPartIdxLabel, pPrior, r1);
    sqlite3VdbeAddOp3(v, 142, iIdxCur + i, r1, pIdx->uniqNotNull ? pIdx->nKeyCol : pIdx->nColumn);
    sqlite3VdbeChangeP4(v, -1, (const char *)pIdx, (-6));
    sqlite3ResolvePartIdxLabel(pParse, iPartIdxLabel);
    pPrior = pIdx;
  }
}

int sqlite3GenerateIndexKey(Parse *pParse, Index *pIdx, int iDataCur, int regOut, int prefixOnly, int *piPartIdxLabel,
                            Index *pPrior, int regPrior) {
  Vdbe *v = pParse->pVdbe;
  int j;
  int regBase;
  int nCol;

  if (piPartIdxLabel) {
    if (pIdx->pPartIdxWhere) {
      *piPartIdxLabel = sqlite3VdbeMakeLabel(pParse);
      pParse->iSelfTab = iDataCur + 1;
      sqlite3ExprIfFalseDup(pParse, pIdx->pPartIdxWhere, *piPartIdxLabel, 0x10);
      pParse->iSelfTab = 0;
      pPrior = 0;

    } else {
      *piPartIdxLabel = 0;
    }
  }
  nCol = (prefixOnly && pIdx->uniqNotNull) ? pIdx->nKeyCol : pIdx->nColumn;
  regBase = sqlite3GetTempRange(pParse, nCol);
  if (pPrior && (regBase != regPrior || pPrior->pPartIdxWhere))
    pPrior = 0;
  for (j = 0; j < nCol; j++) {
    if (pPrior && pPrior->aiColumn[j] == pIdx->aiColumn[j] && pPrior->aiColumn[j] != (-2)) {
      continue;
    }
    sqlite3ExprCodeLoadIndexColumn(pParse, pIdx, iDataCur, j, regBase + j);
    if (pIdx->aiColumn[j] >= 0) {
      sqlite3VdbeDeletePriorOpcode(v, 89);
    }
  }
  if (regOut) {
    sqlite3VdbeAddOp3(v, 99, regBase, nCol, regOut);
  }
  sqlite3ReleaseTempRange(pParse, regBase, nCol);
  return regBase;
}

void sqlite3ResolvePartIdxLabel(Parse *pParse, int iLabel) {
  if (iLabel) {
    sqlite3VdbeResolveLabel(pParse->pVdbe, iLabel);
  }
}

int sqlite3FkLocateIndex(Parse *pParse, Table *pParent, FKey *pFKey, Index **ppIdx, int **paiCol) {
  Index *pIdx = 0;
  int *aiCol = 0;
  int nCol = pFKey->nCol;
  char *zKey = pFKey->aCol[0].zCol;

  if (nCol == 1) {
    if (pParent->iPKey >= 0) {
      if (!zKey)
        return 0;
      if (!sqlite3StrICmp(pParent->aCol[pParent->iPKey].zCnName, zKey)) {
        return 0;
      }
    }
  } else if (paiCol) {
    aiCol = (int *)sqlite3DbMallocRawNN(pParse->db, nCol * sizeof(int));
    if (!aiCol)
      return 1;
    *paiCol = aiCol;
  }

  for (pIdx = pParent->pIndex; pIdx; pIdx = pIdx->pNext) {
    if (pIdx->nKeyCol == nCol && ((pIdx)->onError != 0) && pIdx->pPartIdxWhere == 0) {
      if (zKey == 0) {
        if ((pIdx)->idxType == 2) {
          if (aiCol) {
            int i;
            for (i = 0; i < nCol; i++)
              aiCol[i] = pFKey->aCol[i].iFrom;
          }
          break;
        }
      } else {
        int i, j;
        for (i = 0; i < nCol; i++) {
          i16 iCol = pIdx->aiColumn[i];
          const char *zDfltColl;
          char *zIdxCol;

          if (iCol < 0)
            break;

          zDfltColl = sqlite3ColumnColl(&pParent->aCol[iCol]);
          if (!zDfltColl)
            zDfltColl = sqlite3StrBINARY;
          if (sqlite3StrICmp(pIdx->azColl[i], zDfltColl))
            break;

          zIdxCol = pParent->aCol[iCol].zCnName;
          for (j = 0; j < nCol; j++) {
            if (sqlite3StrICmp(pFKey->aCol[j].zCol, zIdxCol) == 0) {
              if (aiCol)
                aiCol[i] = pFKey->aCol[j].iFrom;
              break;
            }
          }
          if (j == nCol)
            break;
        }
        if (i == nCol)
          break;
      }
    }
  }

  if (!pIdx) {
    if (!pParse->disableTriggers) {
      sqlite3ErrorMsg(pParse, "foreign key mismatch - \"%w\" referencing \"%w\"", pFKey->pFrom->zName, pFKey->zTo);
    }
    sqlite3DbFree(pParse->db, aiCol);
    return 1;
  }

  *ppIdx = pIdx;
  return 0;
}

void fkLookupParent(Parse *pParse, int iDb, Table *pTab, Index *pIdx, FKey *pFKey, int *aiCol, int regData, int nIncr,
                    int isIgnore) {
  int i;
  Vdbe *v = sqlite3GetVdbe(pParse);
  int iCur = pParse->nTab - 1;
  int iOk = sqlite3VdbeMakeLabel(pParse);

  if (nIncr < 0) {
    sqlite3VdbeAddOp2(v, 60, pFKey->isDeferred, iOk);
  }
  for (i = 0; i < pFKey->nCol; i++) {
    int iReg = sqlite3TableColumnToStorage(pFKey->pFrom, aiCol[i]) + regData + 1;
    sqlite3VdbeAddOp2(v, 51, iReg, iOk);
  }

  if (isIgnore == 0) {
    if (pIdx == 0) {
      int iMustBeInt;
      int regTemp = sqlite3GetTempReg(pParse);

      sqlite3VdbeAddOp2(v, 83, sqlite3TableColumnToStorage(pFKey->pFrom, aiCol[0]) + 1 + regData, regTemp);
      iMustBeInt = sqlite3VdbeAddOp2(v, 13, regTemp, 0);

      if (pTab == pFKey->pFrom && nIncr == 1) {
        sqlite3VdbeAddOp3(v, 54, regData, iOk, regTemp);
        sqlite3VdbeChangeP5(v, 0x90);
      }

      sqlite3OpenTable(pParse, iCur, iDb, pTab, 114);
      sqlite3VdbeAddOp3(v, 31, iCur, 0, regTemp);
      sqlite3VdbeGoto(v, iOk);
      sqlite3VdbeJumpHere(v, sqlite3VdbeCurrentAddr(v) - 2);
      sqlite3VdbeJumpHere(v, iMustBeInt);
      sqlite3ReleaseTempReg(pParse, regTemp);
    } else {
      int nCol = pFKey->nCol;
      int regTemp = sqlite3GetTempRange(pParse, nCol);

      sqlite3VdbeAddOp3(v, 114, iCur, pIdx->tnum, iDb);
      sqlite3VdbeSetP4KeyInfo(pParse, pIdx);
      for (i = 0; i < nCol; i++) {
        sqlite3VdbeAddOp2(v, 82, sqlite3TableColumnToStorage(pFKey->pFrom, aiCol[i]) + 1 + regData, regTemp + i);
      }

      if (pTab == pFKey->pFrom && nIncr == 1) {
        int iJump = sqlite3VdbeCurrentAddr(v) + nCol + 1;
        for (i = 0; i < nCol; i++) {
          int iChild = sqlite3TableColumnToStorage(pFKey->pFrom, aiCol[i]) + 1 + regData;
          int iParent = 1 + regData;
          iParent += sqlite3TableColumnToStorage(pIdx->pTable, pIdx->aiColumn[i]);

          if (pIdx->aiColumn[i] == pTab->iPKey) {
            iParent = regData;
          }
          sqlite3VdbeAddOp3(v, 53, iChild, iJump, iParent);
          sqlite3VdbeChangeP5(v, 0x10);
        }
        sqlite3VdbeGoto(v, iOk);
      }

      sqlite3VdbeAddOp4(v, 98, regTemp, nCol, 0, sqlite3IndexAffinityStr(pParse->db, pIdx), nCol);
      sqlite3VdbeAddOp4Int(v, 29, iCur, iOk, regTemp, nCol);
      sqlite3ReleaseTempRange(pParse, regTemp, nCol);
    }
  }

  if (!pFKey->isDeferred && !(pParse->db->flags & 0x00080000) && !pParse->pToplevel && !pParse->isMultiWrite) {
    sqlite3HaltConstraint(pParse, (19 | (3 << 8)), 2, 0, (-1), 4);
  } else {
    if (nIncr > 0 && pFKey->isDeferred == 0) {
      sqlite3MayAbort(pParse);
    }
    sqlite3VdbeAddOp2(v, 160, pFKey->isDeferred, nIncr);
  }

  sqlite3VdbeResolveLabel(v, iOk);
  sqlite3VdbeAddOp1(v, 124, iCur);
}

Expr *exprTableRegister(Parse *pParse, Table *pTab, int regBase, i16 iCol) {
  Expr *pExpr;
  Column *pCol;
  const char *zColl;
  sqlite3 *db = pParse->db;

  pExpr = sqlite3Expr(db, 176, 0);
  if (pExpr) {
    if (iCol >= 0 && iCol != pTab->iPKey) {
      pCol = &pTab->aCol[iCol];
      pExpr->iTable = regBase + sqlite3TableColumnToStorage(pTab, iCol) + 1;
      pExpr->affExpr = pCol->affinity;
      zColl = sqlite3ColumnColl(pCol);
      if (zColl == 0)
        zColl = db->pDfltColl->zName;
      pExpr = sqlite3ExprAddCollateString(pParse, pExpr, zColl);
    } else {
      pExpr->iTable = regBase;
      pExpr->affExpr = 0x44;
    }
  }
  return pExpr;
}

void fkScanChildren(Parse *pParse, SrcList *pSrc, Table *pTab, Index *pIdx, FKey *pFKey, int *aiCol, int regData,
                    int nIncr) {
  sqlite3 *db = pParse->db;
  int i;
  Expr *pWhere = 0;
  NameContext sNameContext;
  WhereInfo *pWInfo;
  int iFkIfZero = 0;
  Vdbe *v = sqlite3GetVdbe(pParse);

  if (nIncr < 0) {
    iFkIfZero = sqlite3VdbeAddOp2(v, 60, pFKey->isDeferred, 0);
  }

  for (i = 0; i < pFKey->nCol; i++) {
    Expr *pLeft;
    Expr *pRight;
    Expr *pEq;
    i16 iCol;
    const char *zCol;

    iCol = pIdx ? pIdx->aiColumn[i] : -1;
    pLeft = exprTableRegister(pParse, pTab, regData, iCol);
    iCol = aiCol ? aiCol[i] : pFKey->aCol[0].iFrom;

    zCol = pFKey->pFrom->aCol[iCol].zCnName;
    pRight = sqlite3Expr(db, 60, zCol);
    pEq = sqlite3PExpr(pParse, 54, pLeft, pRight);
    pWhere = sqlite3ExprAnd(pParse, pWhere, pEq);
  }

  if (pTab == pFKey->pFrom && nIncr > 0) {
    Expr *pNe;
    Expr *pLeft;
    Expr *pRight;
    if ((((pTab)->tabFlags & 0x00000080) == 0)) {
      pLeft = exprTableRegister(pParse, pTab, regData, -1);
      pRight = exprTableColumn(db, pTab, pSrc->a[0].iCursor, -1);
      pNe = sqlite3PExpr(pParse, 53, pLeft, pRight);
    } else {
      Expr *pEq, *pAll = 0;

      for (i = 0; i < pIdx->nKeyCol; i++) {
        i16 iCol = pIdx->aiColumn[i];

        pLeft = exprTableRegister(pParse, pTab, regData, iCol);
        pRight = sqlite3Expr(db, 60, pTab->aCol[iCol].zCnName);
        pEq = sqlite3PExpr(pParse, 45, pLeft, pRight);
        pAll = sqlite3ExprAnd(pParse, pAll, pEq);
      }
      pNe = sqlite3PExpr(pParse, 19, pAll, 0);
    }
    pWhere = sqlite3ExprAnd(pParse, pWhere, pNe);
  }

  memset(&sNameContext, 0, sizeof(NameContext));
  sNameContext.pSrcList = pSrc;
  sNameContext.pParse = pParse;
  sqlite3ResolveExprNames(&sNameContext, pWhere);

  if (pParse->nErr == 0) {
    pWInfo = sqlite3WhereBegin(pParse, pSrc, pWhere, 0, 0, 0, 0, 0);
    sqlite3VdbeAddOp2(v, 160, pFKey->isDeferred, nIncr);
    if (pWInfo) {
      sqlite3WhereEnd(pWInfo);
    }
  }

  sqlite3ExprDelete(db, pWhere);
  if (iFkIfZero) {
    sqlite3VdbeJumpHereOrPopInst(v, iFkIfZero);
  }
}

void sqlite3FkDropTable(Parse *pParse, SrcList *pName, Table *pTab) {
  sqlite3 *db = pParse->db;
  if ((db->flags & 0x00004000) && ((pTab)->eTabType == 0)) {
    int iSkip = 0;
    Vdbe *v = sqlite3GetVdbe(pParse);

    if (sqlite3FkReferences(pTab) == 0) {
      FKey *p;
      for (p = pTab->u.tab.pFKey; p; p = p->pNextFrom) {
        if (p->isDeferred || (db->flags & 0x00080000))
          break;
      }
      if (!p)
        return;
      iSkip = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeAddOp2(v, 60, 1, iSkip);
    }

    pParse->disableTriggers = 1;
    sqlite3DeleteFrom(pParse, sqlite3SrcListDup(db, pName, 0), 0, 0, 0);
    pParse->disableTriggers = 0;

    if ((db->flags & 0x00080000) == 0) {
      sqlite3VdbeAddOp2(v, 60, 0, sqlite3VdbeCurrentAddr(v) + 2);
      sqlite3HaltConstraint(pParse, (19 | (3 << 8)), 2, 0, (-1), 4);
    }

    if (iSkip) {
      sqlite3VdbeResolveLabel(v, iSkip);
    }
  }
}

int isSetNullAction(Parse *pParse, FKey *pFKey) {
  Parse *pTop = ((pParse)->pToplevel ? (pParse)->pToplevel : (pParse));
  if (pTop->pTriggerPrg) {
    Trigger *p = pTop->pTriggerPrg->pTrigger;
    if ((p == pFKey->apTrigger[0] && pFKey->aAction[0] == 8) || (p == pFKey->apTrigger[1] && pFKey->aAction[1] == 8)) {
      return 1;
    }
  }
  return 0;
}

void sqlite3FkCheck(Parse *pParse, Table *pTab, int regOld, int regNew, int *aChange, int bChngRowid) {
  sqlite3 *db = pParse->db;
  FKey *pFKey;
  int iDb;
  const char *zDb;
  int isIgnoreErrors = pParse->disableTriggers;

  if ((db->flags & 0x00004000) == 0)
    return;
  if (!((pTab)->eTabType == 0))
    return;

  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);

  zDb = db->aDb[iDb].zDbSName;

  for (pFKey = pTab->u.tab.pFKey; pFKey; pFKey = pFKey->pNextFrom) {
    Table *pTo;
    Index *pIdx = 0;
    int *aiFree = 0;
    int *aiCol;
    int iCol;
    int i;
    int bIgnore = 0;

    if (aChange && sqlite3_stricmp(pTab->zName, pFKey->zTo) != 0 &&
        fkChildIsModified(pTab, pFKey, aChange, bChngRowid) == 0) {
      continue;
    }

    if (pParse->disableTriggers) {
      pTo = sqlite3FindTable(db, pFKey->zTo, zDb);
    } else {
      pTo = sqlite3LocateTable(pParse, 0, pFKey->zTo, zDb);
    }
    if (!pTo || sqlite3FkLocateIndex(pParse, pTo, pFKey, &pIdx, &aiFree)) {
      if (!isIgnoreErrors || db->mallocFailed)
        return;
      if (pTo == 0) {
        Vdbe *v = sqlite3GetVdbe(pParse);
        int iJump = sqlite3VdbeCurrentAddr(v) + pFKey->nCol + 1;
        for (i = 0; i < pFKey->nCol; i++) {
          int iFromCol, iReg;
          iFromCol = pFKey->aCol[i].iFrom;
          iReg = sqlite3TableColumnToStorage(pFKey->pFrom, iFromCol) + regOld + 1;
          sqlite3VdbeAddOp2(v, 51, iReg, iJump);
        }
        sqlite3VdbeAddOp2(v, 160, pFKey->isDeferred, -1);
      }
      continue;
    }

    if (aiFree) {
      aiCol = aiFree;
    } else {
      iCol = pFKey->aCol[0].iFrom;
      aiCol = &iCol;
    }
    for (i = 0; i < pFKey->nCol; i++) {
      if (aiCol[i] == pTab->iPKey) {
        aiCol[i] = -1;
      }

      if (db->xAuth) {
        int rcauth;
        char *zCol = pTo->aCol[pIdx ? pIdx->aiColumn[i] : pTo->iPKey].zCnName;
        rcauth = sqlite3AuthReadCol(pParse, pTo->zName, zCol, iDb);
        bIgnore = (rcauth == SQLITE_IGNORE);
      }
    }

    sqlite3TableLock(pParse, iDb, pTo->tnum, 0, pTo->zName);
    pParse->nTab++;

    if (regOld != 0) {
      fkLookupParent(pParse, iDb, pTo, pIdx, pFKey, aiCol, regOld, -1, bIgnore);
    }
    if (regNew != 0 && !isSetNullAction(pParse, pFKey)) {
      fkLookupParent(pParse, iDb, pTo, pIdx, pFKey, aiCol, regNew, +1, bIgnore);
    }

    sqlite3DbFree(db, aiFree);
  }

  for (pFKey = sqlite3FkReferences(pTab); pFKey; pFKey = pFKey->pNextTo) {
    Index *pIdx = 0;
    SrcList *pSrc;
    int *aiCol = 0;

    if (aChange && fkParentIsModified(pTab, pFKey, aChange, bChngRowid) == 0) {
      continue;
    }

    if (!pFKey->isDeferred && !(db->flags & 0x00080000) && !pParse->pToplevel && !pParse->isMultiWrite) {
      continue;
    }

    if (sqlite3FkLocateIndex(pParse, pTab, pFKey, &pIdx, &aiCol)) {
      if (!isIgnoreErrors || db->mallocFailed)
        return;
      continue;
    }

    pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
    if (pSrc) {
      SrcItem *pItem = pSrc->a;
      pItem->pSTab = pFKey->pFrom;
      pItem->zName = pFKey->pFrom->zName;
      pItem->pSTab->nTabRef++;
      pItem->iCursor = pParse->nTab++;

      if (regNew != 0) {
        fkScanChildren(pParse, pSrc, pTab, pIdx, pFKey, aiCol, regNew, -1);
      }
      if (regOld != 0) {
        int eAction = pFKey->aAction[aChange != 0];
        if ((db->flags & ((u64)(0x00008) << 32)))
          eAction = 0;

        fkScanChildren(pParse, pSrc, pTab, pIdx, pFKey, aiCol, regOld, 1);

        if (!pFKey->isDeferred && eAction != 10 && eAction != 8) {
          sqlite3MayAbort(pParse);
        }
      }
      pItem->zName = 0;
      sqlite3SrcListDelete(db, pSrc);
    }
    sqlite3DbFree(db, aiCol);
  }
}

u32 sqlite3FkOldmask(Parse *pParse, Table *pTab) {
  u32 mask = 0;
  if (pParse->db->flags & 0x00004000 && ((pTab)->eTabType == 0)) {
    FKey *p;
    int i;
    for (p = pTab->u.tab.pFKey; p; p = p->pNextFrom) {
      for (i = 0; i < p->nCol; i++)
        mask |= (((p->aCol[i].iFrom) > 31) ? 0xffffffff : ((u32)1 << (p->aCol[i].iFrom)));
    }
    for (p = sqlite3FkReferences(pTab); p; p = p->pNextTo) {
      Index *pIdx = 0;
      sqlite3FkLocateIndex(pParse, pTab, p, &pIdx, 0);
      if (pIdx) {
        for (i = 0; i < pIdx->nKeyCol; i++) {
          mask |= (((pIdx->aiColumn[i]) > 31) ? 0xffffffff : ((u32)1 << (pIdx->aiColumn[i])));
        }
      }
    }
  }
  return mask;
}

int sqlite3FkRequired(Parse *pParse, Table *pTab, int *aChange, int chngRowid) {
  int eRet = 1;
  int bHaveFK = 0;
  if (pParse->db->flags & 0x00004000 && ((pTab)->eTabType == 0)) {
    if (!aChange) {
      bHaveFK = (sqlite3FkReferences(pTab) || pTab->u.tab.pFKey);
    } else {
      FKey *p;

      for (p = pTab->u.tab.pFKey; p; p = p->pNextFrom) {
        if (fkChildIsModified(pTab, p, aChange, chngRowid)) {
          if (0 == sqlite3_stricmp(pTab->zName, p->zTo))
            eRet = 2;
          bHaveFK = 1;
        }
      }

      for (p = sqlite3FkReferences(pTab); p; p = p->pNextTo) {
        if (fkParentIsModified(pTab, p, aChange, chngRowid)) {
          if ((pParse->db->flags & ((u64)(0x00008) << 32)) == 0 && p->aAction[1] != 0) {
            return 2;
          }
          bHaveFK = 1;
        }
      }
    }
  }
  return bHaveFK ? eRet : 0;
}

Trigger *fkActionTrigger(Parse *pParse, Table *pTab, FKey *pFKey, ExprList *pChanges) {
  sqlite3 *db = pParse->db;
  int action;
  Trigger *pTrigger;
  int iAction = (pChanges != 0);

  action = pFKey->aAction[iAction];
  if ((db->flags & ((u64)(0x00008) << 32)))
    action = 0;
  if (action == 7 && (db->flags & 0x00080000)) {
    return 0;
  }
  pTrigger = pFKey->apTrigger[iAction];

  if (action != 0 && !pTrigger) {
    char const *zFrom;
    int nFrom;
    Index *pIdx = 0;
    int *aiCol = 0;
    TriggerStep *pStep = 0;
    Expr *pWhere = 0;
    ExprList *pList = 0;
    Select *pSelect = 0;
    int i;
    Expr *pWhen = 0;

    if (sqlite3FkLocateIndex(pParse, pTab, pFKey, &pIdx, &aiCol))
      return 0;

    for (i = 0; i < pFKey->nCol; i++) {
      Token tOld = {"old", 3};
      Token tNew = {"new", 3};
      Token tFromCol;
      Token tToCol;
      int iFromCol;
      Expr *pEq;

      iFromCol = aiCol ? aiCol[i] : pFKey->aCol[0].iFrom;

      sqlite3TokenInit(&tToCol, pTab->aCol[pIdx ? pIdx->aiColumn[i] : pTab->iPKey].zCnName);
      sqlite3TokenInit(&tFromCol, pFKey->pFrom->aCol[iFromCol].zCnName);

      pEq = sqlite3PExpr(
          pParse, 54,
          sqlite3PExpr(pParse, 142, sqlite3ExprAlloc(db, 60, &tOld, 0), sqlite3ExprAlloc(db, 60, &tToCol, 0)),
          sqlite3ExprAlloc(db, 60, &tFromCol, 0));
      pWhere = sqlite3ExprAnd(pParse, pWhere, pEq);

      if (pChanges) {
        pEq = sqlite3PExpr(
            pParse, 45,
            sqlite3PExpr(pParse, 142, sqlite3ExprAlloc(db, 60, &tOld, 0), sqlite3ExprAlloc(db, 60, &tToCol, 0)),
            sqlite3PExpr(pParse, 142, sqlite3ExprAlloc(db, 60, &tNew, 0), sqlite3ExprAlloc(db, 60, &tToCol, 0)));
        pWhen = sqlite3ExprAnd(pParse, pWhen, pEq);
      }

      if (action != 7 && (action != 10 || pChanges)) {
        Expr *pNew;
        if (action == 10) {
          pNew = sqlite3PExpr(pParse, 142, sqlite3ExprAlloc(db, 60, &tNew, 0), sqlite3ExprAlloc(db, 60, &tToCol, 0));
        } else if (action == 9) {
          Column *pCol = pFKey->pFrom->aCol + iFromCol;
          Expr *pDflt;
          if (pCol->colFlags & 0x0060) {
            pDflt = 0;
          } else {
            pDflt = sqlite3ColumnExpr(pFKey->pFrom, pCol);
          }
          if (pDflt) {
            pNew = sqlite3ExprDup(db, pDflt, 0);
          } else {
            pNew = sqlite3ExprAlloc(db, 122, 0, 0);
          }
        } else {
          pNew = sqlite3ExprAlloc(db, 122, 0, 0);
        }
        pList = sqlite3ExprListAppend(pParse, pList, pNew);
        sqlite3ExprListSetName(pParse, pList, &tFromCol, 0);
      }
    }
    sqlite3DbFree(db, aiCol);

    zFrom = pFKey->pFrom->zName;
    nFrom = sqlite3Strlen30(zFrom);

    if (action == 7) {
      SrcList *pSrc;
      Expr *pRaise;

      pRaise = sqlite3Expr(db, 118, "FOREIGN KEY constraint failed"), pRaise = sqlite3PExpr(pParse, 72, pRaise, 0);
      if (pRaise) {
        pRaise->affExpr = 2;
      }
      pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
      if (pSrc) {
        SrcItem *pItem = &pSrc->a[0];
        pItem->zName = sqlite3DbStrDup(db, zFrom);
        pItem->fg.fixedSchema = 1;
        pItem->u4.pSchema = pTab->pSchema;
      }
      pSelect = sqlite3SelectNew(pParse, sqlite3ExprListAppend(pParse, 0, pRaise), pSrc, pWhere, 0, 0, 0, 0, 0);
      pWhere = 0;
    }

    db->lookaside.bDisable++;
    db->lookaside.sz = 0;

    pTrigger = (Trigger *)sqlite3DbMallocZero(db, sizeof(Trigger) + sizeof(TriggerStep));
    if (pTrigger) {
      pStep = pTrigger->step_list = (TriggerStep *)&pTrigger[1];
      pStep->pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
      if (pStep->pSrc) {
        SrcItem *pItem = &pStep->pSrc->a[0];
        pItem->zName = sqlite3DbStrNDup(db, zFrom, nFrom);
        pItem->u4.pSchema = pTab->pSchema;
        pItem->fg.fixedSchema = 1;
      }
      pStep->pWhere = sqlite3ExprDup(db, pWhere, 0x0001);
      pStep->pExprList = sqlite3ExprListDup(db, pList, 0x0001);
      pStep->pSelect = sqlite3SelectDup(db, pSelect, 0x0001);
      if (pWhen) {
        pWhen = sqlite3PExpr(pParse, 19, pWhen, 0);
        pTrigger->pWhen = sqlite3ExprDup(db, pWhen, 0x0001);
      }
    }

    db->lookaside.bDisable--;
    db->lookaside.sz = db->lookaside.bDisable ? 0 : db->lookaside.szTrue;

    sqlite3ExprDelete(db, pWhere);
    sqlite3ExprDelete(db, pWhen);
    sqlite3ExprListDelete(db, pList);
    sqlite3SelectDelete(db, pSelect);
    if (db->mallocFailed == 1) {
      fkTriggerDelete(db, pTrigger);
      return 0;
    }

    switch (action) {
      case 7:
        pStep->op = 139;
        break;
      case 10:
        if (!pChanges) {
          pStep->op = 129;
          break;
        }
        __attribute__((fallthrough));
      default:
        pStep->op = 130;
    }
    pStep->pTrig = pTrigger;
    pTrigger->pSchema = pTab->pSchema;
    pTrigger->pTabSchema = pTab->pSchema;
    pFKey->apTrigger[iAction] = pTrigger;
    pTrigger->op = (pChanges ? 130 : 129);
  }

  return pTrigger;
}

void sqlite3FkActions(Parse *pParse, Table *pTab, ExprList *pChanges, int regOld, int *aChange, int bChngRowid) {
  if (pParse->db->flags & 0x00004000) {
    FKey *pFKey;
    for (pFKey = sqlite3FkReferences(pTab); pFKey; pFKey = pFKey->pNextTo) {
      if (aChange == 0 || fkParentIsModified(pTab, pFKey, aChange, bChngRowid)) {
        Trigger *pAct = fkActionTrigger(pParse, pTab, pFKey, pChanges);
        if (pAct) {
          sqlite3CodeRowTriggerDirect(pParse, pAct, pTab, regOld, 2, 0);
        }
      }
    }
  }
}

void sqlite3OpenTable(Parse *pParse, int iCur, int iDb, Table *pTab, int opcode) {
  Vdbe *v;

  v = pParse->pVdbe;

  if (!pParse->db->noSharedCache) {
    sqlite3TableLock(pParse, iDb, pTab->tnum, (opcode == 116) ? 1 : 0, pTab->zName);
  }
  if ((((pTab)->tabFlags & 0x00000080) == 0)) {
    sqlite3VdbeAddOp4Int(v, opcode, iCur, pTab->tnum, iDb, pTab->nNVCol);
  } else {
    Index *pPk = sqlite3PrimaryKeyIndex(pTab);

    sqlite3VdbeAddOp3(v, opcode, iCur, pPk->tnum, iDb);
    sqlite3VdbeSetP4KeyInfo(pParse, pPk);
  }
}

int readsTable(Parse *p, int iDb, Table *pTab) {
  Vdbe *v = sqlite3GetVdbe(p);
  int i;
  int iEnd = sqlite3VdbeCurrentAddr(v);

  VTable *pVTab = ((pTab)->eTabType == 1) ? sqlite3GetVTable(p->db, pTab) : 0;

  for (i = 1; i < iEnd; i++) {
    VdbeOp *pOp = sqlite3VdbeGetOp(v, i);

    if (pOp->opcode == 114 && pOp->p3 == iDb) {
      Index *pIndex;
      Pgno tnum = pOp->p2;
      if (tnum == pTab->tnum) {
        return 1;
      }
      for (pIndex = pTab->pIndex; pIndex; pIndex = pIndex->pNext) {
        if (tnum == pIndex->tnum) {
          return 1;
        }
      }
    }

    if (pOp->opcode == 175 && pOp->p4.pVtab == pVTab) {
      return 1;
    }
  }
  return 0;
}

void sqlite3ComputeGeneratedColumns(Parse *pParse, int iRegStore, Table *pTab) {
  int i;
  Walker w;
  Column *pRedo;
  int eProgress;
  VdbeOp *pOp;

  sqlite3TableAffinity(pParse->pVdbe, pTab, iRegStore);
  if ((pTab->tabFlags & 0x00000040) != 0) {
    pOp = sqlite3VdbeGetLastOp(pParse->pVdbe);
    if (pOp->opcode == 98) {
      int ii, jj;
      char *zP4 = pOp->p4.z;

      for (ii = jj = 0; zP4[jj]; ii++) {
        if (pTab->aCol[ii].colFlags & 0x0020) {
          continue;
        }
        if (pTab->aCol[ii].colFlags & 0x0040) {
          zP4[jj] = 0x40;
        }
        jj++;
      }
    } else if (pOp->opcode == 97) {
      pOp->p3 = 1;
    }
  }

  for (i = 0; i < pTab->nCol; i++) {
    if (pTab->aCol[i].colFlags & 0x0060) {
      pTab->aCol[i].colFlags |= 0x0080;
    }
  }

  w.u.pTab = pTab;
  w.xExprCallback = exprColumnFlagUnion;
  w.xSelectCallback = 0;
  w.xSelectCallback2 = 0;

  pParse->iSelfTab = -iRegStore;
  do {
    eProgress = 0;
    pRedo = 0;
    for (i = 0; i < pTab->nCol; i++) {
      Column *pCol = pTab->aCol + i;
      if ((pCol->colFlags & 0x0080) != 0) {
        int x;
        pCol->colFlags |= 0x0100;
        w.eCode = 0;
        sqlite3WalkExpr(&w, sqlite3ColumnExpr(pTab, pCol));
        pCol->colFlags &= ~0x0100;
        if (w.eCode & 0x0080) {
          pRedo = pCol;
          continue;
        }
        eProgress = 1;

        x = sqlite3TableColumnToStorage(pTab, i) + iRegStore;
        sqlite3ExprCodeGeneratedColumn(pParse, pTab, pCol, x);
        pCol->colFlags &= ~0x0080;
      }
    }
  } while (pRedo && eProgress);
  if (pRedo) {
    sqlite3ErrorMsg(pParse, "generated column loop on \"%s\"", pRedo->zCnName);
  }
  pParse->iSelfTab = 0;
}

int autoIncBegin(Parse *pParse, int iDb, Table *pTab) {
  int memId = 0;

  if ((pTab->tabFlags & 0x00000008) != 0 && (pParse->db->mDbFlags & 0x0004) == 0) {
    Parse *pToplevel = ((pParse)->pToplevel ? (pParse)->pToplevel : (pParse));
    AutoincInfo *pInfo;
    Table *pSeqTab = pParse->db->aDb[iDb].pSchema->pSeqTab;

    if (pSeqTab == 0 || !(((pSeqTab)->tabFlags & 0x00000080) == 0) || (((pSeqTab)->eTabType == 1)) ||
        pSeqTab->nCol != 2) {
      pParse->nErr++;
      pParse->rc = (11 | (2 << 8));
      return 0;
    }

    pInfo = pToplevel->pAinc;
    while (pInfo && pInfo->pTab != pTab) {
      pInfo = pInfo->pNext;
    }
    if (pInfo == 0) {
      pInfo = (AutoincInfo*)(sqlite3DbMallocRawNN(pParse->db, sizeof(*pInfo)));
      sqlite3ParserAddCleanup(pToplevel, sqlite3DbFree, pInfo);
      if (pParse->db->mallocFailed)
        return 0;
      pInfo->pNext = pToplevel->pAinc;
      pToplevel->pAinc = pInfo;
      pInfo->pTab = pTab;
      pInfo->iDb = iDb;
      pToplevel->nMem++;
      pInfo->regCtr = ++pToplevel->nMem;
      pToplevel->nMem += 2;
    }
    memId = pInfo->regCtr;
  }
  return memId;
}

void sqlite3AutoincrementBegin(Parse *pParse) {
  AutoincInfo *p;
  sqlite3 *db = pParse->db;
  Db *pDb;
  int memId;
  Vdbe *v = pParse->pVdbe;

  for (p = pParse->pAinc; p; p = p->pNext) {
    static const int iLn = 0;
    static const VdbeOpList autoInc[] = {{77, 0, 0, 0},  {36, 0, 10, 0}, {96, 0, 0, 0}, {53, 0, 9, 0},
                                         {137, 0, 0, 0}, {96, 0, 1, 0},  {88, 0, 0, 0}, {82, 0, 0, 0},
                                         {9, 0, 11, 0},  {40, 0, 2, 0},  {73, 0, 0, 0}, {124, 0, 0, 0}};
    VdbeOp *aOp;
    pDb = &db->aDb[p->iDb];
    memId = p->regCtr;

    sqlite3OpenTable(pParse, 0, p->iDb, pDb->pSchema->pSeqTab, 114);
    sqlite3VdbeLoadString(v, memId - 1, p->pTab->zName);
    aOp = sqlite3VdbeAddOpList(v, ((int)(sizeof(autoInc) / sizeof(autoInc[0]))), autoInc, iLn);
    if (aOp == 0)
      break;
    aOp[0].p2 = memId;
    aOp[0].p3 = memId + 2;
    aOp[2].p3 = memId;
    aOp[3].p1 = memId - 1;
    aOp[3].p3 = memId;
    aOp[3].p5 = 0x10;
    aOp[4].p2 = memId + 1;
    aOp[5].p3 = memId;
    aOp[6].p1 = memId;
    aOp[7].p2 = memId + 2;
    aOp[7].p1 = memId;
    aOp[10].p2 = memId;
    if (pParse->nTab == 0)
      pParse->nTab = 1;
  }
}

void autoIncStep(Parse *pParse, int memId, int regRowid) {
  if (memId > 0) {
    sqlite3VdbeAddOp2(pParse->pVdbe, 161, memId, regRowid);
  }
}

__attribute__((noinline)) void autoIncrementEnd(Parse *pParse) {
  AutoincInfo *p;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  for (p = pParse->pAinc; p; p = p->pNext) {
    static const int iLn = 0;
    static const VdbeOpList autoIncEnd[] = {
        {52, 0, 2, 0}, {129, 0, 0, 0}, {99, 0, 2, 0}, {130, 0, 0, 0}, {124, 0, 0, 0}};
    VdbeOp *aOp;
    Db *pDb = &db->aDb[p->iDb];
    int iRec;
    int memId = p->regCtr;

    iRec = sqlite3GetTempReg(pParse);

    sqlite3VdbeAddOp3(v, 56, memId + 2, sqlite3VdbeCurrentAddr(v) + 7, memId);
    sqlite3OpenTable(pParse, 0, p->iDb, pDb->pSchema->pSeqTab, 116);
    aOp = sqlite3VdbeAddOpList(v, ((int)(sizeof(autoIncEnd) / sizeof(autoIncEnd[0]))), autoIncEnd, iLn);
    if (aOp == 0)
      break;
    aOp[0].p1 = memId + 1;
    aOp[1].p2 = memId + 1;
    aOp[2].p1 = memId - 1;
    aOp[2].p3 = iRec;
    aOp[3].p2 = iRec;
    aOp[3].p3 = memId + 1;
    aOp[3].p5 = 0x08;
    sqlite3ReleaseTempReg(pParse, iRec);
  }
}

void sqlite3AutoincrementEnd(Parse *pParse) {
  if (pParse->pAinc)
    autoIncrementEnd(pParse);
}

void sqlite3MultiValuesEnd(Parse *pParse, Select *pVal) {
  if ((pVal) && pVal->pSrc->nSrc > 0) {
    SrcItem *pItem = &pVal->pSrc->a[0];

    if (pItem->fg.isSubquery) {
      sqlite3VdbeEndCoroutine(pParse->pVdbe, pItem->u4.pSubq->regReturn);
      sqlite3VdbeJumpHere(pParse->pVdbe, pItem->u4.pSubq->addrFillSub - 1);
    }
  }
}

int exprListIsConstant(Parse *pParse, ExprList *pRow) {
  int ii;
  for (ii = 0; ii < pRow->nExpr; ii++) {
    if (0 == sqlite3ExprIsConstant(pParse, pRow->a[ii].pExpr))
      return 0;
  }
  return 1;
}

int exprListIsNoAffinity(Parse *pParse, ExprList *pRow) {
  int ii;
  if (exprListIsConstant(pParse, pRow) == 0)
    return 0;
  for (ii = 0; ii < pRow->nExpr; ii++) {
    Expr *pExpr = pRow->a[ii].pExpr;

    if (0 != sqlite3ExprAffinity(pExpr))
      return 0;
  }
  return 1;
}

Select *sqlite3MultiValues(Parse *pParse, Select *pLeft, ExprList *pRow) {
  if (pParse->bHasWith || pParse->db->init.busy || exprListIsConstant(pParse, pRow) == 0 ||
      (pLeft->pSrc->nSrc == 0 && exprListIsNoAffinity(pParse, pLeft->pEList) == 0) || (pParse->eParseMode != 0)) {
    Select *pSelect = 0;
    int f = 0x0000200 | 0x0000400;
    if (pLeft->pSrc->nSrc) {
      sqlite3MultiValuesEnd(pParse, pLeft);
      f = 0x0000200;
    } else if (pLeft->pPrior) {
      f = (f & pLeft->selFlags);
    }
    pSelect = sqlite3SelectNew(pParse, pRow, 0, 0, 0, 0, 0, f, 0);
    pLeft->selFlags &= ~(u32)0x0000400;
    if (pSelect) {
      pSelect->op = 136;
      pSelect->pPrior = pLeft;
      pLeft = pSelect;
    }
  } else {
    SrcItem *p = 0;

    if (pLeft->pSrc->nSrc == 0) {
      Vdbe *v = sqlite3GetVdbe(pParse);
      Select *pRet = sqlite3SelectNew(pParse, 0, 0, 0, 0, 0, 0, 0, 0);

      if ((pParse->db->mDbFlags & 0x0010) == 0) {
        sqlite3ReadSchema(pParse);
      }

      if (pRet) {
        SelectDest dest;
        Subquery *pSubq;
        pRet->pSrc->nSrc = 1;
        pRet->pPrior = pLeft->pPrior;
        pRet->op = pLeft->op;
        if (pRet->pPrior)
          pRet->selFlags |= 0x0000200;
        pLeft->pPrior = 0;
        pLeft->op = 139;

        p = &pRet->pSrc->a[0];
        p->fg.viaCoroutine = 1;
        p->iCursor = -1;

        p->u1.nRow = 2;
        if (sqlite3SrcItemAttachSubquery(pParse, p, pLeft, 0)) {
          pSubq = p->u4.pSubq;
          pSubq->addrFillSub = sqlite3VdbeCurrentAddr(v) + 1;
          pSubq->regReturn = ++pParse->nMem;
          sqlite3VdbeAddOp3(v, 11, pSubq->regReturn, 0, pSubq->addrFillSub);
          sqlite3SelectDestInit(&dest, 11, pSubq->regReturn);

          dest.iSdst = pParse->nMem + 3;
          dest.nSdst = pLeft->pEList->nExpr;
          pParse->nMem += 2 + dest.nSdst;

          pLeft->selFlags |= 0x0000400;
          sqlite3Select(pParse, pLeft, &dest);
          pSubq->regResult = dest.iSdst;
        }
        pLeft = pRet;
      }
    } else {
      p = &pLeft->pSrc->a[0];

      p->u1.nRow++;
    }

    if (pParse->nErr == 0) {
      Subquery *pSubq;

      pSubq = p->u4.pSubq;

      if (pSubq->pSelect->pEList->nExpr != pRow->nExpr) {
        sqlite3SelectWrongNumTermsError(pParse, pSubq->pSelect);
      } else {
        sqlite3ExprCodeExprList(pParse, pRow, pSubq->regResult, 0, 0);
        sqlite3VdbeAddOp1(pParse->pVdbe, 12, pSubq->regReturn);
      }
    }
    sqlite3ExprListDelete(pParse->db, pRow);
  }

  return pLeft;
}

void sqlite3Insert(Parse *pParse, SrcList *pTabList, Select *pSelect, IdList *pColumn, int onError, Upsert *pUpsert) {
  sqlite3 *db;
  Table *pTab;
  int i, j;
  Vdbe *v;
  Index *pIdx;
  int nColumn;
  int nHidden = 0;
  int iDataCur = 0;
  int iIdxCur = 0;
  int ipkColumn = -1;
  int endOfLoop;
  int srcTab = 0;
  int addrInsTop = 0;
  int addrCont = 0;
  SelectDest dest;
  int iDb;
  u8 useTempTable = 0;
  u8 appendFlag = 0;
  u8 withoutRowid;
  u8 bIdListInOrder;
  ExprList *pList = 0;
  int iRegStore;

  int regFromSelect = 0;
  int regAutoinc = 0;
  int regRowCount = 0;
  int regIns;
  int regRowid;
  int regData;
  int *aRegIdx = 0;
  int *aTabColMap = 0;

  int isView;
  Trigger *pTrigger;
  int tmask;

  db = pParse->db;

  if (pParse->nErr) {
    goto insert_cleanup;
  }

  dest.iSDParm = 0;

  if (pSelect && (pSelect->selFlags & 0x0000200) != 0 && pSelect->pPrior == 0) {
    pList = pSelect->pEList;
    pSelect->pEList = 0;
    sqlite3SelectDelete(db, pSelect);
    pSelect = 0;
  }

  pTab = sqlite3SrcListLookup(pParse, pTabList);
  if (pTab == 0) {
    goto insert_cleanup;
  }
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);

  if (sqlite3AuthCheck(pParse, SQLITE_INSERT, pTab->zName, 0, db->aDb[iDb].zDbSName)) {
    goto insert_cleanup;
  }
  withoutRowid = !(((pTab)->tabFlags & 0x00000080) == 0);

  pTrigger = sqlite3TriggersExist(pParse, pTab, 128, 0, &tmask);
  isView = ((pTab)->eTabType == 2);

  if (sqlite3ViewGetColumnNames(pParse, pTab)) {
    goto insert_cleanup;
  }

  if (sqlite3IsReadOnly(pParse, pTab, pTrigger)) {
    goto insert_cleanup;
  }

  v = sqlite3GetVdbe(pParse);
  if (v == 0)
    goto insert_cleanup;
  if (pParse->nested == 0)
    sqlite3VdbeCountChanges(v);
  sqlite3BeginWriteOperation(pParse, pSelect || pTrigger, iDb);

  if (pColumn == 0 && pSelect != 0 && pTrigger == 0 && xferOptimization(pParse, pTab, pSelect, onError, iDb)) {
    goto insert_end;
  }

  regAutoinc = autoIncBegin(pParse, iDb, pTab);

  regRowid = regIns = pParse->nMem + 1;
  pParse->nMem += pTab->nCol + 1;
  if ((pTab)->eTabType == 1) {
    regRowid++;
    pParse->nMem++;
  }
  regData = regRowid + 1;

  bIdListInOrder = (pTab->tabFlags & (0x00000400 | 0x00000040)) == 0;
  if (pColumn) {
    aTabColMap = (int*)(sqlite3DbMallocZero(db, pTab->nCol * sizeof(int)));
    if (aTabColMap == 0)
      goto insert_cleanup;
    for (i = 0; i < pColumn->nId; i++) {
      j = sqlite3ColumnIndex(pTab, pColumn->a[i].zName);
      if (j >= 0) {
        if (aTabColMap[j] == 0)
          aTabColMap[j] = i + 1;
        if (i != j)
          bIdListInOrder = 0;
        if (j == pTab->iPKey) {
          ipkColumn = i;
        }

        if (pTab->aCol[j].colFlags & (0x0040 | 0x0020)) {
          sqlite3ErrorMsg(pParse, "cannot INSERT into generated column \"%s\"", pTab->aCol[j].zCnName);
          goto insert_cleanup;
        }

      } else {
        if (sqlite3IsRowid(pColumn->a[i].zName) && !withoutRowid) {
          ipkColumn = i;
          bIdListInOrder = 0;
        } else {
          sqlite3ErrorMsg(pParse, "table %S has no column named %s", pTabList->a, pColumn->a[i].zName);
          pParse->checkSchema = 1;
          goto insert_cleanup;
        }
      }
    }
  }

  if (pSelect) {
    int rc;

    if (pSelect->pSrc->nSrc == 1 && pSelect->pSrc->a[0].fg.viaCoroutine && pSelect->pPrior == 0) {
      SrcItem *pItem = &pSelect->pSrc->a[0];
      Subquery *pSubq;

      pSubq = pItem->u4.pSubq;
      dest.iSDParm = pSubq->regReturn;
      regFromSelect = pSubq->regResult;

      nColumn = pSubq->pSelect->pEList->nExpr;
      sqlite3VdbeExplain(pParse, 0, "SCAN %S", pItem);
      if (bIdListInOrder && nColumn == pTab->nCol) {
        regData = regFromSelect;
        regRowid = regData - 1;
        regIns = regRowid - (((pTab)->eTabType == 1) ? 1 : 0);
      }
    } else {
      int addrTop;
      int regYield = ++pParse->nMem;
      addrTop = sqlite3VdbeCurrentAddr(v) + 1;
      sqlite3VdbeAddOp3(v, 11, regYield, 0, addrTop);
      sqlite3SelectDestInit(&dest, 11, regYield);
      dest.iSdst = bIdListInOrder ? regData : 0;
      dest.nSdst = pTab->nCol;
      rc = sqlite3Select(pParse, pSelect, &dest);
      regFromSelect = dest.iSdst;

      if (rc || pParse->nErr)
        goto insert_cleanup;

      sqlite3VdbeEndCoroutine(v, regYield);
      sqlite3VdbeJumpHere(v, addrTop - 1);

      nColumn = pSelect->pEList->nExpr;
    }

    if (pTrigger || readsTable(pParse, iDb, pTab)) {
      useTempTable = 1;
    }

    if (useTempTable) {
      int regRec;
      int regTempRowid;
      int addrL;

      srcTab = pParse->nTab++;
      regRec = sqlite3GetTempReg(pParse);
      regTempRowid = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp2(v, 120, srcTab, nColumn);
      addrL = sqlite3VdbeAddOp1(v, 12, dest.iSDParm);
      sqlite3VdbeAddOp3(v, 99, regFromSelect, nColumn, regRec);
      sqlite3VdbeAddOp2(v, 129, srcTab, regTempRowid);
      sqlite3VdbeAddOp3(v, 130, srcTab, regRec, regTempRowid);
      sqlite3VdbeGoto(v, addrL);
      sqlite3VdbeJumpHere(v, addrL);
      sqlite3ReleaseTempReg(pParse, regRec);
      sqlite3ReleaseTempReg(pParse, regTempRowid);
    }
  } else {
    NameContext sNC;
    memset(&sNC, 0, sizeof(sNC));
    sNC.pParse = pParse;
    srcTab = -1;

    if (pList) {
      nColumn = pList->nExpr;
      if (sqlite3ResolveExprListNames(&sNC, pList)) {
        goto insert_cleanup;
      }
    } else {
      nColumn = 0;
    }
  }

  if (pColumn == 0 && nColumn > 0) {
    ipkColumn = pTab->iPKey;

    if (ipkColumn >= 0 && (pTab->tabFlags & 0x00000060) != 0) {
      for (i = ipkColumn - 1; i >= 0; i--) {
        if (pTab->aCol[i].colFlags & 0x0060) {
          ipkColumn--;
        }
      }
    }

    if ((pTab->tabFlags & (0x00000060 | 0x00000002)) != 0) {
      for (i = 0; i < pTab->nCol; i++) {
        if (pTab->aCol[i].colFlags & 0x0062)
          nHidden++;
      }
    }
    if (nColumn != (pTab->nCol - nHidden)) {
      sqlite3ErrorMsg(pParse, "table %S has %d columns but %d values were supplied", pTabList->a, pTab->nCol - nHidden,
                      nColumn);
      goto insert_cleanup;
    }
  }
  if (pColumn != 0 && nColumn != pColumn->nId) {
    sqlite3ErrorMsg(pParse, "%d values for %d columns", nColumn, pColumn->nId);
    goto insert_cleanup;
  }

  if ((db->flags & ((u64)(0x00001) << 32)) != 0 && !pParse->nested && !pParse->pTriggerTab && !pParse->bReturning) {
    regRowCount = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, 73, 0, regRowCount);
  }

  if (!isView) {
    int nIdx;
    nIdx = sqlite3OpenTableAndIndices(pParse, pTab, 116, 0, -1, 0, &iDataCur, &iIdxCur);
    aRegIdx = (int*)(sqlite3DbMallocRawNN(db, sizeof(int) * (nIdx + 2)));
    if (aRegIdx == 0) {
      goto insert_cleanup;
    }
    for (i = 0, pIdx = pTab->pIndex; i < nIdx; pIdx = pIdx->pNext, i++) {
      aRegIdx[i] = ++pParse->nMem;
      pParse->nMem += pIdx->nColumn;
    }
    aRegIdx[i] = ++pParse->nMem;
  }

  if (pUpsert) {
    Upsert *pNx;
    if ((pTab)->eTabType == 1) {
      sqlite3ErrorMsg(pParse, "UPSERT not implemented for virtual table \"%s\"", pTab->zName);
      goto insert_cleanup;
    }
    if ((pTab)->eTabType == 2) {
      sqlite3ErrorMsg(pParse, "cannot UPSERT a view");
      goto insert_cleanup;
    }
    if (sqlite3HasExplicitNulls(pParse, pUpsert->pUpsertTarget)) {
      goto insert_cleanup;
    }
    pTabList->a[0].iCursor = iDataCur;
    pNx = pUpsert;
    do {
      pNx->pUpsertSrc = pTabList;
      pNx->regData = regData;
      pNx->iDataCur = iDataCur;
      pNx->iIdxCur = iIdxCur;
      if (pNx->pUpsertTarget) {
        if (sqlite3UpsertAnalyzeTarget(pParse, pTabList, pNx, pUpsert)) {
          goto insert_cleanup;
        }
      }
      pNx = pNx->pNextUpsert;
    } while (pNx != 0);
  }

  if (useTempTable) {
    addrInsTop = sqlite3VdbeAddOp1(v, 36, srcTab);
    addrCont = sqlite3VdbeCurrentAddr(v);
  } else if (pSelect) {
    addrInsTop = addrCont = sqlite3VdbeAddOp1(v, 12, dest.iSDParm);
    if (ipkColumn >= 0) {
      sqlite3VdbeAddOp2(v, 82, regFromSelect + ipkColumn, regRowid);
    }
  }

  nHidden = 0;
  iRegStore = regData;

  for (i = 0; i < pTab->nCol; i++, iRegStore++) {
    int k;
    u32 colFlags;

    if (i == pTab->iPKey) {
      sqlite3VdbeAddOp1(v, 78, iRegStore);
      continue;
    }
    if (((colFlags = pTab->aCol[i].colFlags) & 0x0062) != 0) {
      nHidden++;
      if ((colFlags & 0x0020) != 0) {
        iRegStore--;
        continue;
      } else if ((colFlags & 0x0040) != 0) {
        if (tmask & 1) {
          sqlite3VdbeAddOp1(v, 78, iRegStore);
        }
        continue;
      } else if (pColumn == 0) {
        sqlite3ExprCodeFactorable(pParse, sqlite3ColumnExpr(pTab, &pTab->aCol[i]), iRegStore);
        continue;
      }
    }
    if (pColumn) {
      j = aTabColMap[i];

      if (j == 0) {
        sqlite3ExprCodeFactorable(pParse, sqlite3ColumnExpr(pTab, &pTab->aCol[i]), iRegStore);
        continue;
      }
      k = j - 1;
    } else if (nColumn == 0) {
      sqlite3ExprCodeFactorable(pParse, sqlite3ColumnExpr(pTab, &pTab->aCol[i]), iRegStore);
      continue;
    } else {
      k = i - nHidden;
    }

    if (useTempTable) {
      sqlite3VdbeAddOp3(v, 96, srcTab, k, iRegStore);
    } else if (pSelect) {
      if (regFromSelect != regData) {
        sqlite3VdbeAddOp2(v, 83, regFromSelect + k, iRegStore);
      }
    } else {
      Expr *pX = pList->a[k].pExpr;
      int y = sqlite3ExprCodeTarget(pParse, pX, iRegStore);
      if (y != iRegStore) {
        sqlite3VdbeAddOp2(v, (((pX)->flags & (u32)(0x400000)) != 0) ? 82 : 83, y, iRegStore);
      }
    }
  }

  endOfLoop = sqlite3VdbeMakeLabel(pParse);
  if (tmask & 1) {
    int regCols = sqlite3GetTempRange(pParse, pTab->nCol + 1);

    if (ipkColumn < 0) {
      sqlite3VdbeAddOp2(v, 73, -1, regCols);
    } else {
      int addr1;

      if (useTempTable) {
        sqlite3VdbeAddOp3(v, 96, srcTab, ipkColumn, regCols);
      } else {
        sqlite3ExprCode(pParse, pList->a[ipkColumn].pExpr, regCols);
      }
      addr1 = sqlite3VdbeAddOp1(v, 52, regCols);
      sqlite3VdbeAddOp2(v, 73, -1, regCols);
      sqlite3VdbeJumpHere(v, addr1);
      sqlite3VdbeAddOp1(v, 13, regCols);
    }

    sqlite3VdbeAddOp3(v, 82, regRowid + 1, regCols + 1, pTab->nNVCol - 1);

    if (pTab->tabFlags & 0x00000060) {
      sqlite3ComputeGeneratedColumns(pParse, regCols + 1, pTab);
    }

    if (!isView) {
      sqlite3TableAffinity(v, pTab, regCols + 1);
    }

    sqlite3CodeRowTrigger(pParse, pTrigger, 128, 0, 1, pTab, regCols - pTab->nCol - 1, onError, endOfLoop);

    sqlite3ReleaseTempRange(pParse, regCols, pTab->nCol + 1);
  }

  if (!isView) {
    if ((pTab)->eTabType == 1) {
      sqlite3VdbeAddOp2(v, 77, 0, regIns);
    }
    if (ipkColumn >= 0) {
      if (useTempTable) {
        sqlite3VdbeAddOp3(v, 96, srcTab, ipkColumn, regRowid);
      } else if (pSelect) {
      } else {
        Expr *pIpk = pList->a[ipkColumn].pExpr;
        if (pIpk->op == 122 && !((pTab)->eTabType == 1)) {
          sqlite3VdbeAddOp3(v, 129, iDataCur, regRowid, regAutoinc);
          appendFlag = 1;
        } else {
          sqlite3ExprCode(pParse, pList->a[ipkColumn].pExpr, regRowid);
        }
      }

      if (!appendFlag) {
        int addr1;
        if (!((pTab)->eTabType == 1)) {
          addr1 = sqlite3VdbeAddOp1(v, 52, regRowid);
          sqlite3VdbeAddOp3(v, 129, iDataCur, regRowid, regAutoinc);
          sqlite3VdbeJumpHere(v, addr1);
        } else {
          addr1 = sqlite3VdbeCurrentAddr(v);
          sqlite3VdbeAddOp2(v, 51, regRowid, addr1 + 2);
        }
        sqlite3VdbeAddOp1(v, 13, regRowid);
      }
    } else if (((pTab)->eTabType == 1) || withoutRowid) {
      sqlite3VdbeAddOp2(v, 77, 0, regRowid);
    } else {
      sqlite3VdbeAddOp3(v, 129, iDataCur, regRowid, regAutoinc);
      appendFlag = 1;
    }
    autoIncStep(pParse, regAutoinc, regRowid);

    if (pTab->tabFlags & 0x00000060) {
      sqlite3ComputeGeneratedColumns(pParse, regRowid + 1, pTab);
    }

    if ((pTab)->eTabType == 1) {
      const char *pVTab = (const char *)sqlite3GetVTable(db, pTab);
      sqlite3VtabMakeWritable(pParse, pTab);
      sqlite3VdbeAddOp4(v, 7, 1, pTab->nCol + 2, regIns, pVTab, (-12));
      sqlite3VdbeChangeP5(v, onError == 11 ? 2 : onError);
      sqlite3MayAbort(pParse);
    } else {
      int isReplace = 0;
      int bUseSeek;
      sqlite3GenerateConstraintChecks(pParse, pTab, aRegIdx, iDataCur, iIdxCur, regIns, 0, ipkColumn >= 0, onError,
                                      endOfLoop, &isReplace, 0, pUpsert);
      if (db->flags & 0x00004000) {
        sqlite3FkCheck(pParse, pTab, 0, regIns, 0, 0);
      }

      bUseSeek = (isReplace == 0 || !sqlite3VdbeHasSubProgram(v));
      sqlite3CompleteInsertion(pParse, pTab, iDataCur, iIdxCur, regIns, aRegIdx, 0, appendFlag, bUseSeek);
    }
  }

  if (regRowCount) {
    sqlite3VdbeAddOp2(v, 88, regRowCount, 1);
  }

  if (pTrigger) {
    sqlite3CodeRowTrigger(pParse, pTrigger, 128, 0, 2, pTab, regData - 2 - pTab->nCol, onError, endOfLoop);
  }

  sqlite3VdbeResolveLabel(v, endOfLoop);
  if (useTempTable) {
    sqlite3VdbeAddOp2(v, 40, srcTab, addrCont);
    sqlite3VdbeJumpHere(v, addrInsTop);
    sqlite3VdbeAddOp1(v, 124, srcTab);
  } else if (pSelect) {
    sqlite3VdbeGoto(v, addrCont);

    sqlite3VdbeJumpHere(v, addrInsTop);
  }

insert_end:
  if (pParse->nested == 0 && pParse->pTriggerTab == 0) {
    sqlite3AutoincrementEnd(pParse);
  }

  if (regRowCount) {
    sqlite3CodeChangeCount(v, regRowCount, "rows inserted");
  }

insert_cleanup:
  sqlite3SrcListDelete(db, pTabList);
  sqlite3ExprListDelete(db, pList);
  sqlite3UpsertDelete(db, pUpsert);
  sqlite3SelectDelete(db, pSelect);
  if (pColumn) {
    sqlite3IdListDelete(db, pColumn);
    sqlite3DbFree(db, aTabColMap);
  }
  if (aRegIdx)
    sqlite3DbNNFreeNN(db, aRegIdx);
}

void sqlite3GenerateConstraintChecks(Parse *pParse, Table *pTab, int *aRegIdx, int iDataCur, int iIdxCur,
                                     int regNewData, int regOldData, u8 pkChng, u8 overrideError, int ignoreDest,
                                     int *pbMayReplace, int *aiChng, Upsert *pUpsert) {
  Vdbe *v;
  Index *pIdx;
  Index *pPk = 0;
  sqlite3 *db;
  int i;
  int ix;
  int nCol;
  int onError;
  int seenReplace = 0;
  int nPkField;
  Upsert *pUpsertClause = 0;
  u8 isUpdate;
  u8 bAffinityDone = 0;
  int upsertIpkReturn = 0;
  int upsertIpkDelay = 0;
  int ipkTop = 0;
  int ipkBottom = 0;

  int regTrigCnt;
  int addrRecheck = 0;
  int lblRecheckOk = 0;
  Trigger *pTrigger;
  int nReplaceTrig = 0;
  IndexIterator sIdxIter;

  isUpdate = regOldData != 0;
  db = pParse->db;
  v = pParse->pVdbe;

  nCol = pTab->nCol;

  if ((((pTab)->tabFlags & 0x00000080) == 0)) {
    pPk = 0;
    nPkField = 1;
  } else {
    pPk = sqlite3PrimaryKeyIndex(pTab);
    nPkField = pPk->nKeyCol;
  }

  if (pTab->tabFlags & 0x00000800) {
    int b2ndPass = 0;
    int nSeenReplace = 0;
    int nGenerated = 0;
    while (1) {
      for (i = 0; i < nCol; i++) {
        int iReg;
        Column *pCol = &pTab->aCol[i];
        int isGenerated;
        onError = pCol->notNull;
        if (onError == 0)
          continue;
        if (i == pTab->iPKey) {
          continue;
        }
        isGenerated = pCol->colFlags & 0x0060;
        if (isGenerated && !b2ndPass) {
          nGenerated++;
          continue;
        }
        if (aiChng && aiChng[i] < 0 && !isGenerated) {
          continue;
        }
        if (overrideError != 11) {
          onError = overrideError;
        } else if (onError == 11) {
          onError = 2;
        }
        if (onError == 5) {
          if (b2ndPass || pCol->iDflt == 0) {
            onError = 2;
          } else {
          }
        } else if (b2ndPass && !isGenerated) {
          continue;
        }

        iReg = sqlite3TableColumnToStorage(pTab, i) + regNewData + 1;
        switch (onError) {
          case 5: {
            int addr1 = sqlite3VdbeAddOp1(v, 52, iReg);

            nSeenReplace++;
            sqlite3ExprCodeCopy(pParse, sqlite3ColumnExpr(pTab, pCol), iReg);
            sqlite3VdbeJumpHere(v, addr1);
            break;
          }
          case 2:
            sqlite3MayAbort(pParse);
            __attribute__((fallthrough));
          case 1:
          case 3: {
            char *zMsg = sqlite3MPrintf(db, "%s.%s", pTab->zName, pCol->zCnName);
            sqlite3VdbeAddOp3(v, 71, (19 | (5 << 8)), onError, iReg);
            sqlite3VdbeAppendP4(v, zMsg, (-7));
            sqlite3VdbeChangeP5(v, 1);
            break;
          }
          default: {
            sqlite3VdbeAddOp2(v, 51, iReg, ignoreDest);
            break;
          }
        }
      }
      if (nGenerated == 0 && nSeenReplace == 0) {
        break;
      }
      if (b2ndPass)
        break;
      b2ndPass = 1;

      if (nSeenReplace > 0 && (pTab->tabFlags & 0x00000060) != 0) {
        sqlite3ComputeGeneratedColumns(pParse, regNewData + 1, pTab);
      }
    }
  }

  if (pTab->pCheck && (db->flags & 0x00000200) == 0) {
    ExprList *pCheck = pTab->pCheck;
    pParse->iSelfTab = -(regNewData + 1);
    onError = overrideError != 11 ? overrideError : 2;
    for (i = 0; i < pCheck->nExpr; i++) {
      int allOk;
      Expr *pCopy;
      Expr *pExpr = pCheck->a[i].pExpr;
      if (aiChng && !sqlite3ExprReferencesUpdatedColumn(pExpr, aiChng, pkChng)) {
        continue;
      }
      if (bAffinityDone == 0) {
        sqlite3TableAffinity(v, pTab, regNewData + 1);
        bAffinityDone = 1;
      }
      allOk = sqlite3VdbeMakeLabel(pParse);
      pCopy = sqlite3ExprDup(db, pExpr, 0);
      if (!db->mallocFailed) {
        sqlite3ExprIfTrue(pParse, pCopy, allOk, 0x10);
      }
      sqlite3ExprDelete(db, pCopy);
      if (onError == 4) {
        sqlite3VdbeGoto(v, ignoreDest);
      } else {
        char *zName = pCheck->a[i].zEName;

        if (onError == 5)
          onError = 2;
        sqlite3HaltConstraint(pParse, (19 | (1 << 8)), onError, zName, 0, 3);
      }
      sqlite3VdbeResolveLabel(v, allOk);
    }
    pParse->iSelfTab = 0;
  }

  sIdxIter.eType = 0;
  sIdxIter.i = 0;
  sIdxIter.u.ax.aIdx = 0;
  sIdxIter.u.lx.pIdx = pTab->pIndex;
  if (pUpsert) {
    if (pUpsert->pUpsertTarget == 0) {
      if (pUpsert->isDoUpdate == 0) {
        overrideError = 4;
        pUpsert = 0;
      } else {
        overrideError = 6;
      }
    } else if (pTab->pIndex != 0) {
      int nIdx, jj;
      u64 nByte;
      Upsert *pTerm;
      u8 *bUsed;
      for (nIdx = 0, pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext, nIdx++) {
      }
      sIdxIter.eType = 1;
      sIdxIter.u.ax.nIdx = nIdx;
      nByte = (sizeof(IndexListTerm) + 1) * nIdx + nIdx;
      sIdxIter.u.ax.aIdx = (IndexListTerm*)(sqlite3DbMallocZero(db, nByte));
      if (sIdxIter.u.ax.aIdx == 0)
        return;
      bUsed = (u8 *)&sIdxIter.u.ax.aIdx[nIdx];
      pUpsert->pToFree = sIdxIter.u.ax.aIdx;
      for (i = 0, pTerm = pUpsert; pTerm; pTerm = pTerm->pNextUpsert) {
        if (pTerm->pUpsertTarget == 0)
          break;
        if (pTerm->pUpsertIdx == 0)
          continue;
        jj = 0;
        pIdx = pTab->pIndex;
        while ((pIdx != 0) && pIdx != pTerm->pUpsertIdx) {
          pIdx = pIdx->pNext;
          jj++;
        }
        if (bUsed[jj])
          continue;
        bUsed[jj] = 1;
        sIdxIter.u.ax.aIdx[i].p = pIdx;
        sIdxIter.u.ax.aIdx[i].ix = jj;
        i++;
      }
      for (jj = 0, pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext, jj++) {
        if (bUsed[jj])
          continue;
        sIdxIter.u.ax.aIdx[i].p = pIdx;
        sIdxIter.u.ax.aIdx[i].ix = jj;
        i++;
      }
    }
  }

  if ((db->flags & (0x00002000 | 0x00004000)) == 0) {
    pTrigger = 0;
    regTrigCnt = 0;
  } else {
    if (db->flags & 0x00002000) {
      pTrigger = sqlite3TriggersExist(pParse, pTab, 129, 0, 0);
      regTrigCnt = pTrigger != 0 || sqlite3FkRequired(pParse, pTab, 0, 0);
    } else {
      pTrigger = 0;
      regTrigCnt = sqlite3FkRequired(pParse, pTab, 0, 0);
    }
    if (regTrigCnt) {
      regTrigCnt = ++pParse->nMem;
      sqlite3VdbeAddOp2(v, 73, 0, regTrigCnt);
      lblRecheckOk = sqlite3VdbeMakeLabel(pParse);
      addrRecheck = lblRecheckOk;
    }
  }

  if (pkChng && pPk == 0) {
    int addrRowidOk = sqlite3VdbeMakeLabel(pParse);

    onError = pTab->keyConf;
    if (overrideError != 11) {
      onError = overrideError;
    } else if (onError == 11) {
      onError = 2;
    }

    if (pUpsert) {
      pUpsertClause = sqlite3UpsertOfIndex(pUpsert, 0);
      if (pUpsertClause != 0) {
        if (pUpsertClause->isDoUpdate == 0) {
          onError = 4;
        } else {
          onError = 6;
        }
      }
      if (pUpsertClause != pUpsert) {
        upsertIpkDelay = sqlite3VdbeAddOp0(v, 9);
      }
    }

    if (onError == 5 && onError != overrideError && pTab->pIndex && !upsertIpkDelay) {
      ipkTop = sqlite3VdbeAddOp0(v, 9) + 1;
    }

    if (isUpdate) {
      sqlite3VdbeAddOp3(v, 54, regNewData, addrRowidOk, regOldData);
      sqlite3VdbeChangeP5(v, 0x90);
    }

    sqlite3VdbeAddOp3(v, 31, iDataCur, addrRowidOk, regNewData);

    switch (onError) {
      default: {
        onError = 2;
        __attribute__((fallthrough));
      }
      case 1:
      case 2:
      case 3: {
        sqlite3RowidConstraint(pParse, onError, pTab);
        break;
      }
      case 5: {
        if (regTrigCnt) {
          sqlite3MultiWrite(pParse);
          sqlite3GenerateRowDelete(pParse, pTab, pTrigger, iDataCur, iIdxCur, regNewData, 1, 0, 5, 1, -1);
          sqlite3VdbeAddOp2(v, 88, regTrigCnt, 1);
          nReplaceTrig++;
        } else {
          if (pTab->pIndex) {
            sqlite3MultiWrite(pParse);
            sqlite3GenerateRowIndexDelete(pParse, pTab, iDataCur, iIdxCur, 0, -1);
          }
        }
        seenReplace = 1;
        break;
      }

      case 6: {
        sqlite3UpsertDoUpdate(pParse, pUpsert, pTab, 0, iDataCur);
        __attribute__((fallthrough));
      }

      case 4: {
        sqlite3VdbeGoto(v, ignoreDest);
        break;
      }
    }
    sqlite3VdbeResolveLabel(v, addrRowidOk);
    if (pUpsert && pUpsertClause != pUpsert) {
      upsertIpkReturn = sqlite3VdbeAddOp0(v, 9);
    } else if (ipkTop) {
      ipkBottom = sqlite3VdbeAddOp0(v, 9);
      sqlite3VdbeJumpHere(v, ipkTop - 1);
    }
  }

  for (pIdx = indexIteratorFirst(&sIdxIter, &ix); pIdx; pIdx = indexIteratorNext(&sIdxIter, &ix)) {
    int regIdx;
    int regR;
    int iThisCur;
    int addrUniqueOk;
    int addrConflictCk;

    if (aRegIdx[ix] == 0)
      continue;
    if (pUpsert) {
      pUpsertClause = sqlite3UpsertOfIndex(pUpsert, pIdx);
      if (upsertIpkDelay && pUpsertClause == pUpsert) {
        sqlite3VdbeJumpHere(v, upsertIpkDelay);
      }
    }
    addrUniqueOk = sqlite3VdbeMakeLabel(pParse);
    if (bAffinityDone == 0) {
      sqlite3TableAffinity(v, pTab, regNewData + 1);
      bAffinityDone = 1;
    };
    iThisCur = iIdxCur + ix;

    if (pIdx->pPartIdxWhere) {
      sqlite3VdbeAddOp2(v, 77, 0, aRegIdx[ix]);
      pParse->iSelfTab = -(regNewData + 1);
      sqlite3ExprIfFalseDup(pParse, pIdx->pPartIdxWhere, addrUniqueOk, 0x10);
      pParse->iSelfTab = 0;
    }

    regIdx = aRegIdx[ix] + 1;
    for (i = 0; i < pIdx->nColumn; i++) {
      int iField = pIdx->aiColumn[i];
      int x;
      if (iField == (-2)) {
        pParse->iSelfTab = -(regNewData + 1);
        sqlite3ExprCodeCopy(pParse, pIdx->aColExpr->a[i].pExpr, regIdx + i);
        pParse->iSelfTab = 0;
      } else if (iField == (-1) || iField == pTab->iPKey) {
        x = regNewData;
        sqlite3VdbeAddOp2(v, 84, x, regIdx + i);
      } else {
        x = sqlite3TableColumnToStorage(pTab, iField) + regNewData + 1;
        sqlite3VdbeAddOp2(v, 83, x, regIdx + i);
      }
    }
    sqlite3VdbeAddOp3(v, 99, regIdx, pIdx->nColumn, aRegIdx[ix]);

    if (isUpdate && pPk == pIdx && pkChng == 0) {
      sqlite3VdbeResolveLabel(v, addrUniqueOk);
      continue;
    }

    onError = pIdx->onError;
    if (onError == 0) {
      sqlite3VdbeResolveLabel(v, addrUniqueOk);
      continue;
    }
    if (overrideError != 11) {
      onError = overrideError;
    } else if (onError == 11) {
      onError = 2;
    }

    if (pUpsertClause) {
      if (pUpsertClause->isDoUpdate == 0) {
        onError = 4;
      } else {
        onError = 6;
      }
    }

    if ((ix == 0 && pIdx->pNext == 0) && pPk == pIdx && onError == 5 &&
        (0 == (db->flags & 0x00002000) || 0 == sqlite3TriggersExist(pParse, pTab, 129, 0, 0)) &&
        (0 == (db->flags & 0x00004000) || (0 == pTab->u.tab.pFKey && 0 == sqlite3FkReferences(pTab)))) {
      sqlite3VdbeResolveLabel(v, addrUniqueOk);
      continue;
    }

    addrConflictCk = sqlite3VdbeAddOp4Int(v, 27, iThisCur, addrUniqueOk, regIdx, pIdx->nKeyCol);

    regR = pIdx == pPk ? regIdx : sqlite3GetTempRange(pParse, nPkField);
    if (isUpdate || onError == 5) {
      if ((((pTab)->tabFlags & 0x00000080) == 0)) {
        sqlite3VdbeAddOp2(v, 144, iThisCur, regR);

        if (isUpdate) {
          sqlite3VdbeAddOp3(v, 54, regR, addrUniqueOk, regOldData);
          sqlite3VdbeChangeP5(v, 0x90);
        }
      } else {
        int x;

        if (pIdx != pPk) {
          for (i = 0; i < pPk->nKeyCol; i++) {
            x = sqlite3TableColumnToIndex(pIdx, pPk->aiColumn[i]);
            sqlite3VdbeAddOp3(v, 96, iThisCur, x, regR + i);
          }
        }
        if (isUpdate) {
          int addrJump = sqlite3VdbeCurrentAddr(v) + pPk->nKeyCol;
          int op = 53;
          int regCmp = (((pIdx)->idxType == 2) ? regIdx : regR);

          for (i = 0; i < pPk->nKeyCol; i++) {
            char *p4 = (char *)sqlite3LocateCollSeq(pParse, pPk->azColl[i]);
            x = pPk->aiColumn[i];

            if (i == (pPk->nKeyCol - 1)) {
              addrJump = addrUniqueOk;
              op = 54;
            }
            x = sqlite3TableColumnToStorage(pTab, x);
            sqlite3VdbeAddOp4(v, op, regOldData + 1 + x, addrJump, regCmp + i, p4, (-2));
            sqlite3VdbeChangeP5(v, 0x90);
          }
        }
      }
    }

    switch (onError) {
      case 1:
      case 2:
      case 3: {
        sqlite3UniqueConstraint(pParse, onError, pIdx);
        break;
      }

      case 6: {
        sqlite3UpsertDoUpdate(pParse, pUpsert, pTab, pIdx, iIdxCur + ix);
        __attribute__((fallthrough));
      }

      case 4: {
        sqlite3VdbeGoto(v, ignoreDest);
        break;
      }
      default: {
        int nConflictCk;

        nConflictCk = sqlite3VdbeCurrentAddr(v) - addrConflictCk;

        if (regTrigCnt) {
          sqlite3MultiWrite(pParse);
          nReplaceTrig++;
        }
        if (pTrigger && isUpdate) {
          sqlite3VdbeAddOp1(v, 169, iDataCur);
        }
        sqlite3GenerateRowDelete(pParse, pTab, pTrigger, iDataCur, iIdxCur, regR, nPkField, 0, 5, (pIdx == pPk ? 1 : 0),
                                 iThisCur);
        if (pTrigger && isUpdate) {
          sqlite3VdbeAddOp1(v, 170, iDataCur);
        }
        if (regTrigCnt) {
          int addrBypass;

          sqlite3VdbeAddOp2(v, 88, regTrigCnt, 1);
          addrBypass = sqlite3VdbeAddOp0(v, 9);

          sqlite3VdbeResolveLabel(v, lblRecheckOk);
          lblRecheckOk = sqlite3VdbeMakeLabel(pParse);
          if (pIdx->pPartIdxWhere) {
            sqlite3VdbeAddOp2(v, 51, regIdx - 1, lblRecheckOk);
          }

          while (nConflictCk > 0) {
            VdbeOp x;

            x = *sqlite3VdbeGetOp(v, addrConflictCk);
            if (x.opcode != 144) {
              int p2;
              const char *zP4;
              if (sqlite3OpcodeProperty[x.opcode] & 0x01) {
                p2 = lblRecheckOk;
              } else {
                p2 = x.p2;
              }
              zP4 = (const char*)(x.p4type == (-3) ? ((void *)(intptr_t)(x.p4.i)) : x.p4.z);
              sqlite3VdbeAddOp4(v, x.opcode, x.p1, p2, x.p3, zP4, x.p4type);
              sqlite3VdbeChangeP5(v, x.p5);
            }
            nConflictCk--;
            addrConflictCk++;
          }

          sqlite3UniqueConstraint(pParse, 2, pIdx);

          sqlite3VdbeJumpHere(v, addrBypass);
        }
        seenReplace = 1;
        break;
      }
    }
    sqlite3VdbeResolveLabel(v, addrUniqueOk);
    if (regR != regIdx)
      sqlite3ReleaseTempRange(pParse, regR, nPkField);
    if (pUpsertClause && upsertIpkReturn && sqlite3UpsertNextIsIPK(pUpsertClause)) {
      sqlite3VdbeGoto(v, upsertIpkDelay + 1);
      sqlite3VdbeJumpHere(v, upsertIpkReturn);
      upsertIpkReturn = 0;
    }
  }

  if (ipkTop) {
    sqlite3VdbeGoto(v, ipkTop);

    sqlite3VdbeJumpHere(v, ipkBottom);
  }

  if (nReplaceTrig) {
    sqlite3VdbeAddOp2(v, 17, regTrigCnt, lblRecheckOk);
    if (!pPk) {
      if (isUpdate) {
        sqlite3VdbeAddOp3(v, 54, regNewData, addrRecheck, regOldData);
        sqlite3VdbeChangeP5(v, 0x90);
      }
      sqlite3VdbeAddOp3(v, 31, iDataCur, addrRecheck, regNewData);
      sqlite3RowidConstraint(pParse, 2, pTab);
    } else {
      sqlite3VdbeGoto(v, addrRecheck);
    }
    sqlite3VdbeResolveLabel(v, lblRecheckOk);
  }

  if ((((pTab)->tabFlags & 0x00000080) == 0)) {
    int regRec = aRegIdx[ix];
    sqlite3VdbeAddOp3(v, 99, regNewData + 1, pTab->nNVCol, regRec);
    if (!bAffinityDone) {
      sqlite3TableAffinity(v, pTab, 0);
    }
  }

  *pbMayReplace = seenReplace;
}

void sqlite3CompleteInsertion(Parse *pParse, Table *pTab, int iDataCur, int iIdxCur, int regNewData, int *aRegIdx,
                              int update_flags, int appendBias, int useSeekResult) {
  Vdbe *v;
  Index *pIdx;
  u8 pik_flags;
  int i;

  v = pParse->pVdbe;

  for (i = 0, pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext, i++) {
    if (aRegIdx[i] == 0)
      continue;
    if (pIdx->pPartIdxWhere) {
      sqlite3VdbeAddOp2(v, 51, aRegIdx[i], sqlite3VdbeCurrentAddr(v) + 2);
    }
    pik_flags = (useSeekResult ? 0x10 : 0);
    if (((pIdx)->idxType == 2) && !(((pTab)->tabFlags & 0x00000080) == 0)) {
      pik_flags |= 0x01;
      pik_flags |= (update_flags & 0x02);
      if (update_flags == 0) {
      }
    }
    sqlite3VdbeAddOp4Int(v, 140, iIdxCur + i, aRegIdx[i], aRegIdx[i] + 1,
                         pIdx->uniqNotNull ? pIdx->nKeyCol : pIdx->nColumn);
    sqlite3VdbeChangeP5(v, pik_flags);
  }
  if (!(((pTab)->tabFlags & 0x00000080) == 0))
    return;
  if (pParse->nested) {
    pik_flags = 0;
  } else {
    pik_flags = 0x01;
    pik_flags |= (update_flags ? update_flags : 0x20);
  }
  if (appendBias) {
    pik_flags |= 0x08;
  }
  if (useSeekResult) {
    pik_flags |= 0x10;
  }
  sqlite3VdbeAddOp3(v, 130, iDataCur, aRegIdx[i], regNewData);
  if (!pParse->nested) {
    sqlite3VdbeAppendP4(v, pTab, (-5));
  }
  sqlite3VdbeChangeP5(v, pik_flags);
}

int sqlite3OpenTableAndIndices(Parse *pParse, Table *pTab, int op, u8 p5, int iBase, u8 *aToOpen, int *piDataCur,
                               int *piIdxCur) {
  int i;
  int iDb;
  int iDataCur;
  Index *pIdx;
  Vdbe *v;

  if ((pTab)->eTabType == 1) {
    *piDataCur = *piIdxCur = -999;
    return 0;
  }
  iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
  v = pParse->pVdbe;

  if (iBase < 0)
    iBase = pParse->nTab;
  iDataCur = iBase++;
  *piDataCur = iDataCur;
  if ((((pTab)->tabFlags & 0x00000080) == 0) && (aToOpen == 0 || aToOpen[0])) {
    sqlite3OpenTable(pParse, iDataCur, iDb, pTab, op);
  } else if (pParse->db->noSharedCache == 0) {
    sqlite3TableLock(pParse, iDb, pTab->tnum, op == 116, pTab->zName);
  }
  *piIdxCur = iBase;
  for (i = 0, pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext, i++) {
    int iIdxCur = iBase++;

    if (((pIdx)->idxType == 2) && !(((pTab)->tabFlags & 0x00000080) == 0)) {
      *piDataCur = iIdxCur;
      p5 = 0;
    }
    if (aToOpen == 0 || aToOpen[i + 1]) {
      sqlite3VdbeAddOp3(v, op, iIdxCur, pIdx->tnum, iDb);
      sqlite3VdbeSetP4KeyInfo(pParse, pIdx);
      sqlite3VdbeChangeP5(v, p5);
    }
  }
  if (iBase > pParse->nTab)
    pParse->nTab = iBase;
  return i;
}

int xferOptimization(Parse *pParse, Table *pDest, Select *pSelect, int onError, int iDbDest) {
  sqlite3 *db = pParse->db;
  ExprList *pEList;
  Table *pSrc;
  Index *pSrcIdx, *pDestIdx;
  SrcItem *pItem;
  int i;
  int iDbSrc;
  int iSrc, iDest;
  int addr1, addr2;
  int emptyDestTest = 0;
  int emptySrcTest = 0;
  Vdbe *v;
  int regAutoinc;
  int destHasUniqueIdx = 0;
  int regData, regRowid;

  if (pParse->pWith || pSelect->pWith) {
    return 0;
  }

  if ((pDest)->eTabType == 1) {
    return 0;
  }

  if (onError == 11) {
    if (pDest->iPKey >= 0)
      onError = pDest->keyConf;
    if (onError == 11)
      onError = 2;
  }

  if (pSelect->pSrc->nSrc != 1) {
    return 0;
  }
  if (pSelect->pSrc->a[0].fg.isSubquery) {
    return 0;
  }
  if (pSelect->pWhere) {
    return 0;
  }
  if (pSelect->pOrderBy) {
    return 0;
  }

  if (pSelect->pGroupBy) {
    return 0;
  }
  if (pSelect->pLimit) {
    return 0;
  }
  if (pSelect->pPrior) {
    return 0;
  }
  if (pSelect->selFlags & 0x0000001) {
    return 0;
  }
  pEList = pSelect->pEList;

  if (pEList->nExpr != 1) {
    return 0;
  }

  if (pEList->a[0].pExpr->op != 180) {
    return 0;
  }

  pItem = pSelect->pSrc->a;
  pSrc = sqlite3LocateTableItem(pParse, 0, pItem);
  if (pSrc == 0) {
    return 0;
  }
  if (pSrc->tnum == pDest->tnum && pSrc->pSchema == pDest->pSchema) {
    return 0;
  }
  if ((((pDest)->tabFlags & 0x00000080) == 0) != (((pSrc)->tabFlags & 0x00000080) == 0)) {
    return 0;
  }
  if (!((pSrc)->eTabType == 0)) {
    return 0;
  }
  if (pDest->nCol != pSrc->nCol) {
    return 0;
  }
  if (pDest->iPKey != pSrc->iPKey) {
    return 0;
  }
  if ((pDest->tabFlags & 0x00010000) != 0 && (pSrc->tabFlags & 0x00010000) == 0) {
    return 0;
  }
  for (i = 0; i < pDest->nCol; i++) {
    Column *pDestCol = &pDest->aCol[i];
    Column *pSrcCol = &pSrc->aCol[i];

    if ((pDestCol->colFlags & 0x0060) != (pSrcCol->colFlags & 0x0060)) {
      return 0;
    }

    if ((pDestCol->colFlags & 0x0060) != 0) {
      if (sqlite3ExprCompare(0, sqlite3ColumnExpr(pSrc, pSrcCol), sqlite3ColumnExpr(pDest, pDestCol), -1) != 0) {
        return 0;
      }
    }

    if (pDestCol->affinity != pSrcCol->affinity) {
      return 0;
    }
    if (sqlite3_stricmp(sqlite3ColumnColl(pDestCol), sqlite3ColumnColl(pSrcCol)) != 0) {
      return 0;
    }
    if (pDestCol->notNull && !pSrcCol->notNull) {
      return 0;
    }

    if ((pDestCol->colFlags & 0x0060) == 0 && i > 0) {
      Expr *pDestExpr = sqlite3ColumnExpr(pDest, pDestCol);
      Expr *pSrcExpr = sqlite3ColumnExpr(pSrc, pSrcCol);

      if ((pDestExpr == 0) != (pSrcExpr == 0) ||
          (pDestExpr != 0 && strcmp(pDestExpr->u.zToken, pSrcExpr->u.zToken) != 0)) {
        return 0;
      }
    }
  }
  for (pDestIdx = pDest->pIndex; pDestIdx; pDestIdx = pDestIdx->pNext) {
    if (((pDestIdx)->onError != 0)) {
      destHasUniqueIdx = 1;
    }
    for (pSrcIdx = pSrc->pIndex; pSrcIdx; pSrcIdx = pSrcIdx->pNext) {
      if (xferCompatibleIndex(pDestIdx, pSrcIdx))
        break;
    }
    if (pSrcIdx == 0) {
      return 0;
    }
    if (pSrcIdx->tnum == pDestIdx->tnum && pSrc->pSchema == pDest->pSchema && sqlite3FaultSim(411) == SQLITE_OK) {
      return 0;
    }
  }

  if (pDest->pCheck && (db->mDbFlags & 0x0004) == 0 && sqlite3ExprListCompare(pSrc->pCheck, pDest->pCheck, -1)) {
    return 0;
  }

  if ((db->flags & 0x00004000) != 0 && pDest->u.tab.pFKey != 0) {
    return 0;
  }

  if ((db->flags & ((u64)(0x00001) << 32)) != 0) {
    return 0;
  }

  iDbSrc = sqlite3SchemaToIndex(db, pSrc->pSchema);
  v = sqlite3GetVdbe(pParse);
  sqlite3CodeVerifySchema(pParse, iDbSrc);
  iSrc = pParse->nTab++;
  iDest = pParse->nTab++;
  regAutoinc = autoIncBegin(pParse, iDbDest, pDest);
  regData = sqlite3GetTempReg(pParse);
  sqlite3VdbeAddOp2(v, 77, 0, regData);
  regRowid = sqlite3GetTempReg(pParse);
  sqlite3OpenTable(pParse, iDest, iDbDest, pDest, 116);

  if ((db->mDbFlags & 0x0004) == 0 &&
      ((pDest->iPKey < 0 && pDest->pIndex != 0) || destHasUniqueIdx || (onError != 2 && onError != 1))) {
    addr1 = sqlite3VdbeAddOp2(v, 36, iDest, 0);
    emptyDestTest = sqlite3VdbeAddOp0(v, 9);
    sqlite3VdbeJumpHere(v, addr1);
  }
  if ((((pSrc)->tabFlags & 0x00000080) == 0)) {
    u8 insFlags;
    sqlite3OpenTable(pParse, iSrc, iDbSrc, pSrc, 114);
    emptySrcTest = sqlite3VdbeAddOp2(v, 36, iSrc, 0);
    if (pDest->iPKey >= 0) {
      addr1 = sqlite3VdbeAddOp2(v, 137, iSrc, regRowid);
      if ((db->mDbFlags & 0x0004) == 0) {
        addr2 = sqlite3VdbeAddOp3(v, 31, iDest, 0, regRowid);
        sqlite3RowidConstraint(pParse, onError, pDest);
        sqlite3VdbeJumpHere(v, addr2);
      }
      autoIncStep(pParse, regAutoinc, regRowid);
    } else if (pDest->pIndex == 0 && !(db->mDbFlags & 0x0008)) {
      addr1 = sqlite3VdbeAddOp2(v, 129, iDest, regRowid);
    } else {
      addr1 = sqlite3VdbeAddOp2(v, 137, iSrc, regRowid);
    }

    if (db->mDbFlags & 0x0004) {
      sqlite3VdbeAddOp1(v, 139, iDest);
      insFlags = 0x08 | 0x10 | 0x80;
    } else {
      insFlags = 0x01 | 0x20 | 0x08 | 0x80;
    }

    {
      sqlite3VdbeAddOp3(v, 131, iDest, iSrc, regRowid);
    }
    sqlite3VdbeAddOp3(v, 130, iDest, regData, regRowid);
    if ((db->mDbFlags & 0x0004) == 0) {
      sqlite3VdbeChangeP4(v, -1, (char *)pDest, (-5));
    }
    sqlite3VdbeChangeP5(v, insFlags);

    sqlite3VdbeAddOp2(v, 40, iSrc, addr1);
    sqlite3VdbeAddOp2(v, 124, iSrc, 0);
    sqlite3VdbeAddOp2(v, 124, iDest, 0);
  } else {
    sqlite3TableLock(pParse, iDbDest, pDest->tnum, 1, pDest->zName);
    sqlite3TableLock(pParse, iDbSrc, pSrc->tnum, 0, pSrc->zName);
  }
  for (pDestIdx = pDest->pIndex; pDestIdx; pDestIdx = pDestIdx->pNext) {
    u8 idxInsFlags = 0;
    for (pSrcIdx = pSrc->pIndex; (pSrcIdx); pSrcIdx = pSrcIdx->pNext) {
      if (xferCompatibleIndex(pDestIdx, pSrcIdx))
        break;
    }

    sqlite3VdbeAddOp3(v, 114, iSrc, pSrcIdx->tnum, iDbSrc);
    sqlite3VdbeSetP4KeyInfo(pParse, pSrcIdx);
    sqlite3VdbeAddOp3(v, 116, iDest, pDestIdx->tnum, iDbDest);
    sqlite3VdbeSetP4KeyInfo(pParse, pDestIdx);
    sqlite3VdbeChangeP5(v, 0x01);
    addr1 = sqlite3VdbeAddOp2(v, 36, iSrc, 0);
    if (db->mDbFlags & 0x0004) {
      for (i = 0; i < pSrcIdx->nColumn; i++) {
        const char *zColl = pSrcIdx->azColl[i];
        if (sqlite3_stricmp(sqlite3StrBINARY, zColl))
          break;
      }
      if (i == pSrcIdx->nColumn) {
        idxInsFlags = 0x10 | 0x80;
        sqlite3VdbeAddOp1(v, 139, iDest);
        sqlite3VdbeAddOp2(v, 131, iDest, iSrc);
      }
    } else if (!(((pSrc)->tabFlags & 0x00000080) == 0) && pDestIdx->idxType == 2) {
      idxInsFlags |= 0x01;
    }
    if (idxInsFlags != (0x10 | 0x80)) {
      sqlite3VdbeAddOp3(v, 136, iSrc, regData, 1);
      if ((db->mDbFlags & 0x0004) == 0 && !(((pDest)->tabFlags & 0x00000080) == 0) && ((pDestIdx)->idxType == 2)) {
      }
    }
    sqlite3VdbeAddOp2(v, 140, iDest, regData);
    sqlite3VdbeChangeP5(v, idxInsFlags | 0x08);
    sqlite3VdbeAddOp2(v, 40, iSrc, addr1 + 1);
    sqlite3VdbeJumpHere(v, addr1);
    sqlite3VdbeAddOp2(v, 124, iSrc, 0);
    sqlite3VdbeAddOp2(v, 124, iDest, 0);
  }
  if (emptySrcTest)
    sqlite3VdbeJumpHere(v, emptySrcTest);
  sqlite3ReleaseTempReg(pParse, regRowid);
  sqlite3ReleaseTempReg(pParse, regData);
  if (emptyDestTest) {
    sqlite3AutoincrementEnd(pParse);
    sqlite3VdbeAddOp2(v, 72, SQLITE_OK, 0);
    sqlite3VdbeJumpHere(v, emptyDestTest);
    sqlite3VdbeAddOp2(v, 124, iDest, 0);
    return 0;
  } else {
    return 1;
  }
}

int invalidateTempStorage(Parse *pParse) {
  sqlite3 *db = pParse->db;
  if (db->aDb[1].pBt != 0) {
    if (!db->autoCommit || sqlite3BtreeTxnState(db->aDb[1].pBt) != SQLITE_TXN_NONE) {
      sqlite3ErrorMsg(pParse,
                      "temporary storage cannot be changed "
                      "from within a transaction");
      return SQLITE_ERROR;
    }
    sqlite3BtreeClose(db->aDb[1].pBt);
    db->aDb[1].pBt = 0;
    sqlite3ResetAllSchemasOfConnection(db);
  }
  return SQLITE_OK;
}

int changeTempStorage(Parse *pParse, const char *zStorageType) {
  int ts = getTempStore(zStorageType);
  sqlite3 *db = pParse->db;
  if (db->temp_store == ts)
    return SQLITE_OK;
  if (invalidateTempStorage(pParse) != SQLITE_OK) {
    return SQLITE_ERROR;
  }
  db->temp_store = (u8)ts;
  return SQLITE_OK;
}

void sqlite3Pragma(Parse *pParse, Token *pId1, Token *pId2, Token *pValue, int minusFlag) {
  char *zLeft = 0;
  char *zRight = 0;
  const char *zDb = 0;
  Token *pId;
  char *aFcntl[4];
  int iDb;
  int rc;
  sqlite3 *db = pParse->db;
  Db *pDb;
  Vdbe *v = sqlite3GetVdbe(pParse);
  const PragmaName *pPragma;

  if (v == 0)
    return;
  sqlite3VdbeRunOnlyOnce(v);
  pParse->nMem = 2;

  iDb = sqlite3TwoPartName(pParse, pId1, pId2, &pId);
  if (iDb < 0)
    return;
  pDb = &db->aDb[iDb];

  if (iDb == 1 && sqlite3OpenTempDatabase(pParse)) {
    return;
  }

  zLeft = sqlite3NameFromToken(db, pId);
  if (!zLeft)
    return;
  if (minusFlag) {
    zRight = sqlite3MPrintf(db, "-%T", pValue);
  } else {
    zRight = sqlite3NameFromToken(db, pValue);
  }

  zDb = pId2->n > 0 ? pDb->zDbSName : 0;
  if (sqlite3AuthCheck(pParse, SQLITE_PRAGMA, zLeft, zRight, zDb)) {
    goto pragma_out;
  }

  aFcntl[0] = 0;
  aFcntl[1] = zLeft;
  aFcntl[2] = zRight;
  aFcntl[3] = 0;
  db->busyHandler.nBusy = 0;
  rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_PRAGMA, (void *)aFcntl);
  if (rc == SQLITE_OK) {
    sqlite3VdbeSetNumCols(v, 1);
    sqlite3VdbeSetColName(v, 0, 0, aFcntl[0], ((sqlite3_destructor_type)-1));
    returnSingleText(v, aFcntl[0]);
    sqlite3_free(aFcntl[0]);
    goto pragma_out;
  }
  if (rc != SQLITE_NOTFOUND) {
    if (aFcntl[0]) {
      sqlite3ErrorMsg(pParse, "%s", aFcntl[0]);
      sqlite3_free(aFcntl[0]);
    }
    pParse->nErr++;
    pParse->rc = rc;
    goto pragma_out;
  }

  pPragma = pragmaLocate(zLeft);
  if (pPragma == 0) {
    goto pragma_out;
  }

  if ((pPragma->mPragFlg & 0x01) != 0) {
    if (sqlite3ReadSchema(pParse))
      goto pragma_out;
  }

  if ((pPragma->mPragFlg & 0x02) == 0 && ((pPragma->mPragFlg & 0x04) == 0 || zRight == 0)) {
    setPragmaResultColumnNames(v, pPragma);
  }

  switch (pPragma->ePragTyp) {
    case 13: {
      static const int iLn = 0;
      static const VdbeOpList getCacheSize[] = {
          {2, 0, 0, 0},  {101, 0, 1, 3}, {61, 1, 8, 0},  {73, 0, 2, 0}, {108, 1, 2, 1},
          {61, 1, 8, 0}, {73, 0, 1, 0},  {189, 0, 0, 0}, {86, 1, 1, 0},
      };
      VdbeOp *aOp;
      sqlite3VdbeUsesBtree(v, iDb);
      if (!zRight) {
        pParse->nMem += 2;
        aOp = sqlite3VdbeAddOpList(v, ((int)(sizeof(getCacheSize) / sizeof(getCacheSize[0]))), getCacheSize, iLn);
        if ((0))
          break;
        aOp[0].p1 = iDb;
        aOp[1].p1 = iDb;
        aOp[6].p1 = -2000;
      } else {
        int size = sqlite3AbsInt32(sqlite3Atoi(zRight));
        sqlite3BeginWriteOperation(pParse, 0, iDb);
        sqlite3VdbeAddOp3(v, 102, iDb, 3, size);

        pDb->pSchema->cache_size = size;
        sqlite3BtreeSetCacheSize(pDb->pBt, pDb->pSchema->cache_size);
      }
      break;
    }

    case 31: {
      Btree *pBt = pDb->pBt;

      if (!zRight) {
        int size = (pBt) ? sqlite3BtreeGetPageSize(pBt) : 0;
        returnSingleInt(v, size);
      } else {
        db->nextPagesize = sqlite3Atoi(zRight);
        if (SQLITE_NOMEM == sqlite3BtreeSetPageSize(pBt, db->nextPagesize, 0, 0)) {
          sqlite3OomFault(db);
        }
      }
      break;
    }

    case 33: {
      Btree *pBt = pDb->pBt;
      int b = -1;

      if (zRight) {
        if (sqlite3_stricmp(zRight, "fast") == 0) {
          b = 2;
        } else {
          b = sqlite3GetBoolean(zRight, 0);
        }
      }
      if (pId2->n == 0 && b >= 0) {
        int ii;
        for (ii = 0; ii < db->nDb; ii++) {
          sqlite3BtreeSecureDelete(db->aDb[ii].pBt, b);
        }
      }
      b = sqlite3BtreeSecureDelete(pBt, b);
      returnSingleInt(v, b);
      break;
    }

    case 27: {
      int iReg;
      i64 x = 0;
      sqlite3CodeVerifySchema(pParse, iDb);
      iReg = ++pParse->nMem;
      if ((sqlite3UpperToLower[(unsigned char)(zLeft[0])]) == 'p') {
        sqlite3VdbeAddOp2(v, 180, iDb, iReg);
      } else {
        if (zRight && sqlite3DecOrHexToI64(zRight, &x) == 0) {
          if (x < 0)
            x = 0;
          else if (x > 0xfffffffe)
            x = 0xfffffffe;
        } else {
          x = 0;
        }
        sqlite3VdbeAddOp3(v, 181, iDb, iReg, (int)x);
      }
      sqlite3VdbeAddOp2(v, 86, iReg, 1);
      break;
    }

    case 26: {
      const char *zRet = "normal";
      int eMode = getLockingMode(zRight);

      if (pId2->n == 0 && eMode == -1) {
        eMode = db->dfltLockMode;
      } else {
        Pager *pPager;
        if (pId2->n == 0) {
          int ii;

          for (ii = 2; ii < db->nDb; ii++) {
            pPager = sqlite3BtreePager(db->aDb[ii].pBt);
            sqlite3PagerLockingMode(pPager, eMode);
          }
          db->dfltLockMode = (u8)eMode;
        }
        pPager = sqlite3BtreePager(pDb->pBt);
        eMode = sqlite3PagerLockingMode(pPager, eMode);
      }

      if (eMode == 1) {
        zRet = "exclusive";
      }
      returnSingleText(v, zRet);
      break;
    }

    case 23: {
      int eMode;
      int ii;

      if (zRight == 0) {
        eMode = (-1);
      } else {
        const char *zMode;
        int n = sqlite3Strlen30(zRight);
        for (eMode = 0; (zMode = sqlite3JournalModename(eMode)) != 0; eMode++) {
          if (sqlite3_strnicmp(zRight, zMode, n) == 0)
            break;
        }
        if (!zMode) {
          eMode = (-1);
        }
        if (eMode == 2 && (db->flags & 0x10000000) != 0) {
          eMode = (-1);
        }
      }
      if (eMode == (-1) && pId2->n == 0) {
        iDb = 0;
        pId2->n = 1;
      }
      for (ii = db->nDb - 1; ii >= 0; ii--) {
        if (db->aDb[ii].pBt && (ii == iDb || pId2->n == 0)) {
          sqlite3VdbeUsesBtree(v, ii);
          sqlite3VdbeAddOp3(v, 4, ii, 1, eMode);
        }
      }
      sqlite3VdbeAddOp2(v, 86, 1, 1);
      break;
    }

    case 24: {
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      i64 iLimit = -2;
      if (zRight) {
        sqlite3DecOrHexToI64(zRight, &iLimit);
        if (iLimit < -1)
          iLimit = -1;
      }
      iLimit = sqlite3PagerJournalSizeLimit(pPager, iLimit);
      returnSingleInt(v, iLimit);
      break;
    }

    case 3: {
      Btree *pBt = pDb->pBt;

      if (!zRight) {
        returnSingleInt(v, sqlite3BtreeGetAutoVacuum(pBt));
      } else {
        int eAuto = getAutoVacuum(zRight);

        db->nextAutovac = (u8)eAuto;

        rc = sqlite3BtreeSetAutoVacuum(pBt, eAuto);
        if (rc == SQLITE_OK && (eAuto == 1 || eAuto == 2)) {
          static const int iLn = 0;
          static const VdbeOpList setMeta6[] = {
              {2, 0, 1, 0}, {101, 0, 1, 4}, {16, 1, 0, 0}, {72, SQLITE_OK, 2, 0}, {102, 0, 7, 0},
          };
          VdbeOp *aOp;
          int iAddr = sqlite3VdbeCurrentAddr(v);
          aOp = sqlite3VdbeAddOpList(v, ((int)(sizeof(setMeta6) / sizeof(setMeta6[0]))), setMeta6, iLn);
          if ((0))
            break;
          aOp[0].p1 = iDb;
          aOp[1].p1 = iDb;
          aOp[2].p2 = iAddr + 4;
          aOp[4].p1 = iDb;
          aOp[4].p3 = eAuto - 1;
          sqlite3VdbeUsesBtree(v, iDb);
        }
      }
      break;
    }

    case 19: {
      int iLimit = 0, addr;
      if (zRight == 0 || !sqlite3GetInt32(zRight, &iLimit) || iLimit <= 0) {
        iLimit = 0x7fffffff;
      }
      sqlite3BeginWriteOperation(pParse, 0, iDb);
      sqlite3VdbeAddOp2(v, 73, iLimit, 1);
      addr = sqlite3VdbeAddOp1(v, 64, iDb);
      sqlite3VdbeAddOp1(v, 86, 1);
      sqlite3VdbeAddOp2(v, 88, 1, -1);
      sqlite3VdbeAddOp2(v, 61, 1, addr);
      sqlite3VdbeJumpHere(v, addr);
      break;
    }

    case 6: {
      if (!zRight) {
        returnSingleInt(v, pDb->pSchema->cache_size);
      } else {
        int size = sqlite3Atoi(zRight);
        pDb->pSchema->cache_size = size;
        sqlite3BtreeSetCacheSize(pDb->pBt, pDb->pSchema->cache_size);
      }
      break;
    }

    case 7: {
      if (!zRight) {
        returnSingleInt(v, (db->flags & 0x00000020) == 0 ? 0 : sqlite3BtreeSetSpillSize(pDb->pBt, 0));
      } else {
        int size = 1;
        if (sqlite3GetInt32(zRight, &size)) {
          sqlite3BtreeSetSpillSize(pDb->pBt, size);
        }
        if (sqlite3GetBoolean(zRight, size != 0)) {
          db->flags |= 0x00000020;
        } else {
          db->flags &= ~(u64)0x00000020;
        }
        setAllPagerFlags(db);
      }
      break;
    }

    case 28: {
      sqlite3_int64 sz;

      if (zRight) {
        int ii;
        sqlite3DecOrHexToI64(zRight, &sz);
        if (sz < 0)
          sz = sqlite3Config.szMmap;
        if (pId2->n == 0)
          db->szMmap = sz;
        for (ii = db->nDb - 1; ii >= 0; ii--) {
          if (db->aDb[ii].pBt && (ii == iDb || pId2->n == 0)) {
            sqlite3BtreeSetMmapLimit(db->aDb[ii].pBt, sz);
          }
        }
      }
      sz = -1;
      rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_MMAP_SIZE, &sz);

      if (rc == SQLITE_OK) {
        returnSingleInt(v, sz);
      } else if (rc != SQLITE_NOTFOUND) {
        pParse->nErr++;
        pParse->rc = rc;
      }
      break;
    }

    case 39: {
      if (!zRight) {
        returnSingleInt(v, db->temp_store);
      } else {
        changeTempStorage(pParse, zRight);
      }
      break;
    }

    case 40: {
      sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
      if (!zRight) {
        returnSingleText(v, sqlite3_temp_directory);
      } else {
        if (zRight[0]) {
          int res;
          rc = sqlite3OsAccess(db->pVfs, zRight, SQLITE_ACCESS_READWRITE, &res);
          if (rc != SQLITE_OK || res == 0) {
            sqlite3ErrorMsg(pParse, "not a writable directory");
            sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
            goto pragma_out;
          }
        }
        if (1 == 0 || (1 == 1 && db->temp_store <= 1) || (1 == 2 && db->temp_store == 1)) {
          invalidateTempStorage(pParse);
        }
        sqlite3_free(sqlite3_temp_directory);
        if (zRight[0]) {
          sqlite3_temp_directory = sqlite3_mprintf("%s", zRight);
        } else {
          sqlite3_temp_directory = 0;
        }
      }
      sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
      break;
    }

    case 36: {
      if (!zRight) {
        returnSingleInt(v, pDb->safety_level - 1);
      } else {
        if (!db->autoCommit) {
          sqlite3ErrorMsg(pParse, "Safety level may not be changed inside a transaction");
        } else if (iDb != 1) {
          int iLevel = (getSafetyLevel(zRight, 0, 1) + 1) & 0x07;
          if (iLevel == 0)
            iLevel = 1;
          pDb->safety_level = iLevel;
          pDb->bSyncSet = 1;
          setAllPagerFlags(db);
        }
      }
      break;
    }

    case 4: {
      if (zRight == 0) {
        setPragmaResultColumnNames(v, pPragma);
        returnSingleInt(v, (db->flags & pPragma->iArg) != 0);
      } else {
        u64 mask = pPragma->iArg;
        if (db->autoCommit == 0) {
          mask &= ~(0x00004000);
        }

        if (sqlite3GetBoolean(zRight, 0)) {
          if ((mask & 0x00000001) == 0 || (db->flags & 0x10000000) == 0) {
            db->flags |= mask;
          }
        } else {
          db->flags &= ~mask;
          if (mask == 0x00080000) {
            db->nDeferredImmCons = 0;
            db->nDeferredCons = 0;
          }
          if ((mask & 0x00000001) != 0 && sqlite3_stricmp(zRight, "reset") == 0) {
            sqlite3ResetAllSchemasOfConnection(db);
          }
        }

        sqlite3VdbeAddOp0(v, 168);
        setAllPagerFlags(db);
      }
      break;
    }

    case 37:
      if (zRight) {
        Table *pTab;
        sqlite3CodeVerifyNamedSchema(pParse, zDb);
        pTab = sqlite3LocateTable(pParse, 0x02, zRight, zDb);
        if (pTab) {
          int i, k;
          int nHidden = 0;
          Column *pCol;
          Index *pPk = sqlite3PrimaryKeyIndex(pTab);
          pParse->nMem = 7;
          sqlite3ViewGetColumnNames(pParse, pTab);
          for (i = 0, pCol = pTab->aCol; i < pTab->nCol; i++, pCol++) {
            int isHidden = 0;
            const Expr *pColExpr;
            if (pCol->colFlags & 0x0062) {
              if (pPragma->iArg == 0) {
                nHidden++;
                continue;
              }
              if (pCol->colFlags & 0x0020) {
                isHidden = 2;
              } else if (pCol->colFlags & 0x0040) {
                isHidden = 3;
              } else {
                isHidden = 1;
              }
            }
            if ((pCol->colFlags & 0x0001) == 0) {
              k = 0;
            } else if (pPk == 0) {
              k = 1;
            } else {
              for (k = 1; k <= pTab->nCol && pPk->aiColumn[k - 1] != i; k++) {
              }
            }
            pColExpr = sqlite3ColumnExpr(pTab, pCol);

            sqlite3VdbeMultiLoad(v, 1, pPragma->iArg ? "issisii" : "issisi", i - nHidden, pCol->zCnName,
                                 sqlite3ColumnType(pCol, (char*)("")), pCol->notNull ? 1 : 0,
                                 (isHidden >= 2 || pColExpr == 0) ? 0 : pColExpr->u.zToken, k, isHidden);
          }
        }
      }
      break;

    case 38: {
      int ii;
      pParse->nMem = 6;
      sqlite3CodeVerifyNamedSchema(pParse, zDb);
      for (ii = 0; ii < db->nDb; ii++) {
        HashElem *k;
        Hash *pHash;
        int initNCol;
        if (zDb && sqlite3_stricmp(zDb, db->aDb[ii].zDbSName) != 0)
          continue;

        pHash = &db->aDb[ii].pSchema->tblHash;
        initNCol = ((pHash)->count);
        while (initNCol--) {
          for (k = ((pHash)->first); 1; k = ((k)->next)) {
            Table *pTab;
            if (k == 0) {
              initNCol = 0;
              break;
            }
            pTab = (Table*)(((k)->data));
            if (pTab->nCol == 0) {
              char *zSql = sqlite3MPrintf(db, "SELECT*FROM\"%w\"", pTab->zName);
              if (zSql) {
                sqlite3_stmt *pDummy = 0;
                (void)sqlite3_prepare_v3(db, zSql, -1, SQLITE_PREPARE_DONT_LOG, &pDummy, 0);
                (void)sqlite3_finalize(pDummy);
                sqlite3DbFree(db, zSql);
              }
              if (db->mallocFailed) {
                sqlite3ErrorMsg(db->pParse, "out of memory");
                db->pParse->rc = 7;
              }
              pHash = &db->aDb[ii].pSchema->tblHash;
              break;
            }
          }
        }

        for (k = ((pHash)->first); k; k = ((k)->next)) {
          Table *pTab = (Table*)(((k)->data));
          const char *zType;
          if (zRight && sqlite3_stricmp(zRight, pTab->zName) != 0)
            continue;
          if ((pTab)->eTabType == 2) {
            zType = "view";
          } else if ((pTab)->eTabType == 1) {
            zType = "virtual";
          } else if (pTab->tabFlags & 0x00001000) {
            zType = "shadow";
          } else {
            zType = "table";
          }
          sqlite3VdbeMultiLoad(v, 1, "sssiii", db->aDb[ii].zDbSName, sqlite3PreferredTableName(pTab->zName), zType,
                               pTab->nCol, (pTab->tabFlags & 0x00000080) != 0, (pTab->tabFlags & 0x00010000) != 0);
        }
      }
    } break;

    case 20:
      if (zRight) {
        Index *pIdx;
        Table *pTab;
        pIdx = sqlite3FindIndex(db, zRight, zDb);
        if (pIdx == 0) {
          pTab = sqlite3LocateTable(pParse, 0x02, zRight, zDb);
          if (pTab && !(((pTab)->tabFlags & 0x00000080) == 0)) {
            pIdx = sqlite3PrimaryKeyIndex(pTab);
          }
        }
        if (pIdx) {
          int iIdxDb = sqlite3SchemaToIndex(db, pIdx->pSchema);
          int i;
          int mx;
          if (pPragma->iArg) {
            mx = pIdx->nColumn;
            pParse->nMem = 6;
          } else {
            mx = pIdx->nKeyCol;
            pParse->nMem = 3;
          }
          pTab = pIdx->pTable;
          sqlite3CodeVerifySchema(pParse, iIdxDb);

          for (i = 0; i < mx; i++) {
            i16 cnum = pIdx->aiColumn[i];
            sqlite3VdbeMultiLoad(v, 1, "iisX", i, cnum, cnum < 0 ? 0 : pTab->aCol[cnum].zCnName);
            if (pPragma->iArg) {
              sqlite3VdbeMultiLoad(v, 4, "isiX", pIdx->aSortOrder[i], pIdx->azColl[i], i < pIdx->nKeyCol);
            }
            sqlite3VdbeAddOp2(v, 86, 1, pParse->nMem);
          }
        }
      }
      break;

    case 21:
      if (zRight) {
        Index *pIdx;
        Table *pTab;
        int i;
        pTab = sqlite3FindTable(db, zRight, zDb);
        if (pTab) {
          int iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
          pParse->nMem = 5;
          sqlite3CodeVerifySchema(pParse, iTabDb);
          for (pIdx = pTab->pIndex, i = 0; pIdx; pIdx = pIdx->pNext, i++) {
            const char *azOrigin[] = {"c", "u", "pk"};
            sqlite3VdbeMultiLoad(v, 1, "isisi", i, pIdx->zName, ((pIdx)->onError != 0), azOrigin[pIdx->idxType],
                                 pIdx->pPartIdxWhere != 0);
          }
        }
      }
      break;

    case 12: {
      int i;
      pParse->nMem = 3;
      for (i = 0; i < db->nDb; i++) {
        if (db->aDb[i].pBt == 0)
          continue;

        sqlite3VdbeMultiLoad(v, 1, "iss", i, db->aDb[i].zDbSName, sqlite3BtreeGetFilename(db->aDb[i].pBt));
      }
    } break;

    case 9: {
      int i = 0;
      HashElem *p;
      pParse->nMem = 2;
      for (p = ((&db->aCollSeq)->first); p; p = ((p)->next)) {
        CollSeq *pColl = (CollSeq *)((p)->data);
        sqlite3VdbeMultiLoad(v, 1, "is", i++, pColl->zName);
      }
    } break;

    case 17: {
      int i;
      HashElem *j;
      FuncDef *p;
      int showInternFunc = (db->mDbFlags & 0x0020) != 0;
      pParse->nMem = 6;
      for (i = 0; i < 23; i++) {
        for (p = sqlite3BuiltinFunctions.a[i]; p; p = p->u.pHash) {
          pragmaFunclistLine(v, p, 1, showInternFunc);
        }
      }
      for (j = ((&db->aFunc)->first); j; j = ((j)->next)) {
        p = (FuncDef *)((j)->data);

        pragmaFunclistLine(v, p, 0, showInternFunc);
      }
    } break;

    case 29: {
      HashElem *j;
      pParse->nMem = 1;
      for (j = ((&db->aModule)->first); j; j = ((j)->next)) {
        Module *pMod = (Module *)((j)->data);
        sqlite3VdbeMultiLoad(v, 1, "s", pMod->zName);
      }
    } break;

    case 32: {
      int i;
      for (i = 0; i < ((int)(sizeof(aPragmaName) / sizeof(aPragmaName[0]))); i++) {
        sqlite3VdbeMultiLoad(v, 1, "s", aPragmaName[i].zName);
      }
    } break;

    case 16:
      if (zRight) {
        FKey *pFK;
        Table *pTab;
        pTab = sqlite3FindTable(db, zRight, zDb);
        if (pTab && ((pTab)->eTabType == 0)) {
          pFK = pTab->u.tab.pFKey;
          if (pFK) {
            int iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
            int i = 0;
            pParse->nMem = 8;
            sqlite3CodeVerifySchema(pParse, iTabDb);
            while (pFK) {
              int j;
              for (j = 0; j < pFK->nCol; j++) {
                sqlite3VdbeMultiLoad(v, 1, "iissssss", i, j, pFK->zTo, pTab->aCol[pFK->aCol[j].iFrom].zCnName,
                                     pFK->aCol[j].zCol, actionName(pFK->aAction[1]), actionName(pFK->aAction[0]),
                                     "NONE");
              }
              ++i;
              pFK = pFK->pNextFrom;
            }
          }
        }
      }
      break;

    case 15: {
      FKey *pFK;
      Table *pTab;
      Table *pParent;
      Index *pIdx;
      int i;
      int j;
      HashElem *k;
      int x;
      int regResult;
      int regRow;
      int addrTop;
      int addrOk;
      int *aiCols;

      regResult = pParse->nMem + 1;
      pParse->nMem += 4;
      regRow = ++pParse->nMem;
      k = ((&db->aDb[iDb].pSchema->tblHash)->first);
      while (k) {
        if (zRight) {
          pTab = sqlite3LocateTable(pParse, 0, zRight, zDb);
          k = 0;
        } else {
          pTab = (Table *)((k)->data);
          k = ((k)->next);
        }
        if (pTab == 0 || !((pTab)->eTabType == 0) || pTab->u.tab.pFKey == 0)
          continue;
        iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
        zDb = db->aDb[iDb].zDbSName;
        sqlite3CodeVerifySchema(pParse, iDb);
        sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);
        sqlite3TouchRegister(pParse, pTab->nCol + regRow);
        sqlite3OpenTable(pParse, 0, iDb, pTab, 114);
        sqlite3VdbeLoadString(v, regResult, pTab->zName);

        for (i = 1, pFK = pTab->u.tab.pFKey; pFK; i++, pFK = pFK->pNextFrom) {
          pParent = sqlite3FindTable(db, pFK->zTo, zDb);
          if (pParent == 0)
            continue;
          pIdx = 0;
          sqlite3TableLock(pParse, iDb, pParent->tnum, 0, pParent->zName);
          x = sqlite3FkLocateIndex(pParse, pParent, pFK, &pIdx, 0);
          if (x == 0) {
            if (pIdx == 0) {
              sqlite3OpenTable(pParse, i, iDb, pParent, 114);
            } else {
              sqlite3VdbeAddOp3(v, 114, i, pIdx->tnum, iDb);
              sqlite3VdbeSetP4KeyInfo(pParse, pIdx);
            }
          } else {
            k = 0;
            break;
          }
        }

        if (pFK)
          break;
        if (pParse->nTab < i)
          pParse->nTab = i;
        addrTop = sqlite3VdbeAddOp1(v, 36, 0);

        for (i = 1, pFK = pTab->u.tab.pFKey; pFK; i++, pFK = pFK->pNextFrom) {
          pParent = sqlite3FindTable(db, pFK->zTo, zDb);
          pIdx = 0;
          aiCols = 0;
          if (pParent) {
            x = sqlite3FkLocateIndex(pParse, pParent, pFK, &pIdx, &aiCols);
          }
          addrOk = sqlite3VdbeMakeLabel(pParse);

          sqlite3TouchRegister(pParse, regRow + pFK->nCol);
          for (j = 0; j < pFK->nCol; j++) {
            int iCol = aiCols ? aiCols[j] : pFK->aCol[j].iFrom;
            sqlite3ExprCodeGetColumnOfTable(v, pTab, 0, iCol, regRow + j);
            sqlite3VdbeAddOp2(v, 51, regRow + j, addrOk);
          }

          if (pIdx) {
            sqlite3VdbeAddOp4(v, 98, regRow, pFK->nCol, 0, sqlite3IndexAffinityStr(db, pIdx), pFK->nCol);
            sqlite3VdbeAddOp4Int(v, 29, i, addrOk, regRow, pFK->nCol);
          } else if (pParent) {
            int jmp = sqlite3VdbeCurrentAddr(v) + 2;
            sqlite3VdbeAddOp3(v, 30, i, jmp, regRow);
            sqlite3VdbeGoto(v, addrOk);
          }

          if ((((pTab)->tabFlags & 0x00000080) == 0)) {
            sqlite3VdbeAddOp2(v, 137, 0, regResult + 1);
          } else {
            sqlite3VdbeAddOp2(v, 77, 0, regResult + 1);
          }
          sqlite3VdbeMultiLoad(v, regResult + 2, "siX", pFK->zTo, i - 1);
          sqlite3VdbeAddOp2(v, 86, regResult, 4);
          sqlite3VdbeResolveLabel(v, addrOk);
          sqlite3DbFree(db, aiCols);
        }
        sqlite3VdbeAddOp2(v, 40, 0, addrTop + 1);
        sqlite3VdbeJumpHere(v, addrTop);
      }
    } break;

    case 8: {
      if (zRight) {
        sqlite3RegisterLikeFunctions(db, sqlite3GetBoolean(zRight, 0));
      }
    } break;

    case 22: {
      int i, j, addr, mxErr;
      Table *pObjTab = 0;

      int isQuick = ((sqlite3UpperToLower[(unsigned char)(zLeft[0])]) == 'q');

      if (pId2->z == 0)
        iDb = -1;

      pParse->nMem = 6;

      mxErr = 100;
      if (zRight) {
        if (sqlite3GetInt32(pValue->z, &mxErr)) {
          if (mxErr <= 0) {
            mxErr = 100;
          }
        } else {
          pObjTab = sqlite3LocateTable(pParse, 0, zRight, iDb >= 0 ? db->aDb[iDb].zDbSName : 0);
        }
      }
      sqlite3VdbeAddOp2(v, 73, mxErr - 1, 1);

      for (i = 0; i < db->nDb; i++) {
        HashElem *x;
        Hash *pTbls;
        int *aRoot;
        int cnt = 0;

        if (0 && i == 1)
          continue;
        if (iDb >= 0 && i != iDb)
          continue;

        sqlite3CodeVerifySchema(pParse, i);
        pParse->okConstFactor = 0;

        pTbls = &db->aDb[i].pSchema->tblHash;
        for (cnt = 0, x = ((pTbls)->first); x; x = ((x)->next)) {
          Table *pTab = (Table*)(((x)->data));
          Index *pIdx;
          if (tableSkipIntegrityCheck(pTab, pObjTab))
            continue;
          if ((((pTab)->tabFlags & 0x00000080) == 0))
            cnt++;
          for (pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext) {
            cnt++;
          }
        }
        if (cnt == 0)
          continue;
        if (pObjTab)
          cnt++;
        aRoot = (int*)(sqlite3DbMallocRawNN(db, sizeof(int) * (cnt + 1)));
        if (aRoot == 0)
          break;
        cnt = 0;
        if (pObjTab)
          aRoot[++cnt] = 0;
        for (x = ((pTbls)->first); x; x = ((x)->next)) {
          Table *pTab = (Table*)(((x)->data));
          Index *pIdx;
          if (tableSkipIntegrityCheck(pTab, pObjTab))
            continue;
          if ((((pTab)->tabFlags & 0x00000080) == 0))
            aRoot[++cnt] = pTab->tnum;
          for (pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext) {
            aRoot[++cnt] = pIdx->tnum;
          }
        }
        aRoot[0] = cnt;

        sqlite3TouchRegister(pParse, 8 + cnt);
        sqlite3VdbeAddOp3(v, 77, 0, 8, 8 + cnt);
        sqlite3ClearTempRegCache(pParse);

        sqlite3VdbeAddOp4(v, 157, 1, cnt, 8, (char *)aRoot, (-15));
        sqlite3VdbeChangeP5(v, (u16)i);
        addr = sqlite3VdbeAddOp1(v, 51, 2);
        sqlite3VdbeAddOp4(v, 118, 0, 3, 0, sqlite3MPrintf(db, "*** in database %s ***\n", db->aDb[i].zDbSName), (-7));
        sqlite3VdbeAddOp3(v, 112, 2, 3, 3);
        integrityCheckResultRow(v);
        sqlite3VdbeJumpHere(v, addr);

        cnt = pObjTab ? 1 : 0;
        sqlite3VdbeLoadString(v, 2, "wrong # of entries in index ");
        for (x = ((pTbls)->first); x; x = ((x)->next)) {
          int iTab = 0;
          Table *pTab = (Table*)(((x)->data));
          Index *pIdx;
          if (tableSkipIntegrityCheck(pTab, pObjTab))
            continue;
          if ((((pTab)->tabFlags & 0x00000080) == 0)) {
            iTab = cnt++;
          } else {
            iTab = cnt;
            for (pIdx = pTab->pIndex; (pIdx); pIdx = pIdx->pNext) {
              if ((pIdx)->idxType == 2)
                break;
              iTab++;
            }
          }
          for (pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext) {
            if (pIdx->pPartIdxWhere == 0) {
              addr = sqlite3VdbeAddOp3(v, 54, 8 + cnt, 0, 8 + iTab);
              sqlite3VdbeLoadString(v, 4, pIdx->zName);
              sqlite3VdbeAddOp3(v, 112, 4, 2, 3);
              integrityCheckResultRow(v);
              sqlite3VdbeJumpHere(v, addr);
            }
            cnt++;
          }
        }

        for (x = ((pTbls)->first); x; x = ((x)->next)) {
          Table *pTab = (Table*)(((x)->data));
          Index *pIdx, *pPk;
          Index *pPrior = 0;
          int loopTop;
          int iDataCur, iIdxCur;
          int r1 = -1;
          int bStrict;
          int r2;
          int mxCol;

          if (tableSkipIntegrityCheck(pTab, pObjTab))
            continue;
          if (!((pTab)->eTabType == 0))
            continue;
          if (isQuick || (((pTab)->tabFlags & 0x00000080) == 0)) {
            pPk = 0;
            r2 = 0;
          } else {
            pPk = sqlite3PrimaryKeyIndex(pTab);
            r2 = sqlite3GetTempRange(pParse, pPk->nKeyCol);
            sqlite3VdbeAddOp3(v, 77, 1, r2, r2 + pPk->nKeyCol - 1);
          }
          sqlite3OpenTableAndIndices(pParse, pTab, 114, 0, 1, 0, &iDataCur, &iIdxCur);

          sqlite3VdbeAddOp2(v, 73, 0, 7);
          for (j = 0, pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext, j++) {
            sqlite3VdbeAddOp2(v, 73, 0, 8 + j);
          }

          sqlite3VdbeAddOp2(v, 36, iDataCur, 0);
          loopTop = sqlite3VdbeAddOp2(v, 88, 7, 1);

          if ((((pTab)->tabFlags & 0x00000080) == 0)) {
            mxCol = -1;
            for (j = 0; j < pTab->nCol; j++) {
              if ((pTab->aCol[j].colFlags & 0x0020) == 0)
                mxCol++;
            }
            if (mxCol == pTab->iPKey)
              mxCol--;
          } else {
            mxCol = sqlite3PrimaryKeyIndex(pTab)->nColumn - 1;
          }
          if (mxCol >= 0) {
            sqlite3VdbeAddOp3(v, 96, iDataCur, mxCol, 3);
            sqlite3VdbeTypeofColumn(v, 3);
          }

          if (!isQuick) {
            if (pPk) {
              int a1;
              char *zErr;
              a1 = sqlite3VdbeAddOp4Int(v, 42, iDataCur, 0, r2, pPk->nKeyCol);
              sqlite3VdbeAddOp1(v, 51, r2);
              zErr = sqlite3MPrintf(db, "row not in PRIMARY KEY order for %s", pTab->zName);
              sqlite3VdbeAddOp4(v, 118, 0, 3, 0, zErr, (-7));
              integrityCheckResultRow(v);
              sqlite3VdbeJumpHere(v, a1);
              sqlite3VdbeJumpHere(v, a1 + 1);
              for (j = 0; j < pPk->nKeyCol; j++) {
                sqlite3ExprCodeLoadIndexColumn(pParse, pPk, iDataCur, j, r2 + j);
              }
            }
          }

          bStrict = (pTab->tabFlags & 0x00010000) != 0;
          for (j = 0; j < pTab->nCol; j++) {
            char *zErr;
            Column *pCol = pTab->aCol + j;
            int labelError;
            int labelOk;
            int p1, p3, p4;
            int doTypeCheck;

            if (j == pTab->iPKey)
              continue;
            if (bStrict) {
              doTypeCheck = pCol->eCType > 1;
            } else {
              doTypeCheck = pCol->affinity > 0x41;
            }
            if (pCol->notNull == 0 && !doTypeCheck)
              continue;

            p4 = SQLITE_NULL;
            if (pCol->colFlags & 0x0020) {
              sqlite3ExprCodeGetColumnOfTable(v, pTab, iDataCur, j, 3);
              p1 = -1;
              p3 = 3;
            } else {
              if (pCol->iDflt) {
                sqlite3_value *pDfltValue = 0;
                sqlite3ValueFromExpr(db, sqlite3ColumnExpr(pTab, pCol), ((db)->enc), pCol->affinity, &pDfltValue);
                if (pDfltValue) {
                  p4 = sqlite3_value_type(pDfltValue);
                  sqlite3ValueFree(pDfltValue);
                }
              }
              p1 = iDataCur;
              if (!(((pTab)->tabFlags & 0x00000080) == 0)) {
                p3 = sqlite3TableColumnToIndex(sqlite3PrimaryKeyIndex(pTab), j);
              } else {
                p3 = sqlite3TableColumnToStorage(pTab, j);
              }
            }

            labelError = sqlite3VdbeMakeLabel(pParse);
            labelOk = sqlite3VdbeMakeLabel(pParse);
            if (pCol->notNull) {
              int jmp3;
              int jmp2 = sqlite3VdbeAddOp4Int(v, 18, p1, labelOk, p3, p4);
              if (p1 < 0) {
                sqlite3VdbeChangeP5(v, 0x0f);
                jmp3 = jmp2;
              } else {
                sqlite3VdbeChangeP5(v, 0x0d);

                sqlite3VdbeAddOp3(v, 96, p1, p3, 3);
                sqlite3ColumnDefault(v, pTab, j, 3);
                jmp3 = sqlite3VdbeAddOp2(v, 52, 3, labelOk);
              }
              zErr = sqlite3MPrintf(db, "NULL value in %s.%s", pTab->zName, pCol->zCnName);
              sqlite3VdbeAddOp4(v, 118, 0, 3, 0, zErr, (-7));
              if (doTypeCheck) {
                sqlite3VdbeGoto(v, labelError);
                sqlite3VdbeJumpHere(v, jmp2);
                sqlite3VdbeJumpHere(v, jmp3);
              } else {
              }
            }
            if (bStrict && doTypeCheck) {
              static unsigned char aStdTypeMask[] = {0x1f, 0x18, 0x11, 0x11, 0x13, 0x14};
              sqlite3VdbeAddOp4Int(v, 18, p1, labelOk, p3, p4);

              sqlite3VdbeChangeP5(v, aStdTypeMask[pCol->eCType - 1]);
              zErr = sqlite3MPrintf(db, "non-%s value in %s.%s", sqlite3StdType[pCol->eCType - 1], pTab->zName,
                                    pTab->aCol[j].zCnName);
              sqlite3VdbeAddOp4(v, 118, 0, 3, 0, zErr, (-7));
            } else if (!bStrict && pCol->affinity == 0x42) {
              sqlite3VdbeAddOp4Int(v, 18, p1, labelOk, p3, p4);
              sqlite3VdbeChangeP5(v, 0x1c);
              zErr = sqlite3MPrintf(db, "NUMERIC value in %s.%s", pTab->zName, pTab->aCol[j].zCnName);
              sqlite3VdbeAddOp4(v, 118, 0, 3, 0, zErr, (-7));
            } else if (!bStrict && pCol->affinity >= 0x43) {
              sqlite3VdbeAddOp4Int(v, 18, p1, labelOk, p3, p4);
              sqlite3VdbeChangeP5(v, 0x1b);
              if (p1 >= 0) {
                sqlite3ExprCodeGetColumnOfTable(v, pTab, iDataCur, j, 3);
              }
              sqlite3VdbeAddOp4(v, 98, 3, 1, 0, "C", (-1));
              sqlite3VdbeAddOp4Int(v, 18, -1, labelOk, 3, p4);
              sqlite3VdbeChangeP5(v, 0x1c);
              zErr = sqlite3MPrintf(db, "TEXT value in %s.%s", pTab->zName, pTab->aCol[j].zCnName);
              sqlite3VdbeAddOp4(v, 118, 0, 3, 0, zErr, (-7));
            }
            sqlite3VdbeResolveLabel(v, labelError);
            integrityCheckResultRow(v);
            sqlite3VdbeResolveLabel(v, labelOk);
          }

          if (pTab->pCheck && (db->flags & 0x00000200) == 0) {
            ExprList *pCheck = sqlite3ExprListDup(db, pTab->pCheck, 0);
            if (db->mallocFailed == 0) {
              int addrCkFault = sqlite3VdbeMakeLabel(pParse);
              int addrCkOk = sqlite3VdbeMakeLabel(pParse);
              char *zErr;
              int k;
              pParse->iSelfTab = iDataCur + 1;
              for (k = pCheck->nExpr - 1; k > 0; k--) {
                sqlite3ExprIfFalse(pParse, pCheck->a[k].pExpr, addrCkFault, 0);
              }
              sqlite3ExprIfTrue(pParse, pCheck->a[0].pExpr, addrCkOk, 0x10);
              sqlite3VdbeResolveLabel(v, addrCkFault);
              pParse->iSelfTab = 0;
              zErr = sqlite3MPrintf(db, "CHECK constraint failed in %s", pTab->zName);
              sqlite3VdbeAddOp4(v, 118, 0, 3, 0, zErr, (-7));
              integrityCheckResultRow(v);
              sqlite3VdbeResolveLabel(v, addrCkOk);
            }
            sqlite3ExprListDelete(db, pCheck);
          }
          if (!isQuick) {
            for (j = 0, pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext, j++) {
              int jmp2, jmp3, jmp4, jmp5, label6;
              int kk;
              int ckUniq = sqlite3VdbeMakeLabel(pParse);
              if (pPk == pIdx)
                continue;
              r1 = sqlite3GenerateIndexKey(pParse, pIdx, iDataCur, 0, 0, &jmp3, pPrior, r1);
              pPrior = pIdx;
              sqlite3VdbeAddOp2(v, 88, 8 + j, 1);

              sqlite3VdbeAddOp4Int(v, 29, iIdxCur + j, ckUniq, r1, pIdx->nColumn);
              jmp2 = sqlite3VdbeAddOp3(v, 47, iIdxCur + j, ckUniq, r1);
              sqlite3VdbeChangeP4(v, -1, (const char *)pIdx, (-6));
              sqlite3VdbeAddOp4(v, 118, 0, 3, 0,
                                sqlite3MPrintf(db,
                                               "index %s stores an imprecise floating-point "
                                               "value for row ",
                                               pIdx->zName),
                                (-7));
              sqlite3VdbeAddOp3(v, 112, 7, 3, 3);
              integrityCheckResultRow(v);
              sqlite3VdbeAddOp2(v, 9, 0, ckUniq);

              sqlite3VdbeJumpHere(v, jmp2);
              sqlite3VdbeLoadString(v, 3, "row ");
              sqlite3VdbeAddOp3(v, 112, 7, 3, 3);
              sqlite3VdbeLoadString(v, 4, " missing from index ");
              sqlite3VdbeAddOp3(v, 112, 4, 3, 3);
              jmp5 = sqlite3VdbeLoadString(v, 4, pIdx->zName);
              sqlite3VdbeAddOp3(v, 112, 4, 3, 3);
              jmp4 = integrityCheckResultRow(v);
              sqlite3VdbeResolveLabel(v, ckUniq);

              if ((((pTab)->tabFlags & 0x00000080) == 0)) {
                int jmp7;
                sqlite3VdbeAddOp2(v, 144, iIdxCur + j, 3);
                jmp7 = sqlite3VdbeAddOp3(v, 54, 3, 0, r1 + pIdx->nColumn - 1);
                sqlite3VdbeLoadString(v, 3, "rowid not at end-of-record for row ");
                sqlite3VdbeAddOp3(v, 112, 7, 3, 3);
                sqlite3VdbeLoadString(v, 4, " of index ");
                sqlite3VdbeGoto(v, jmp5 - 1);
                sqlite3VdbeJumpHere(v, jmp7);
              }

              label6 = 0;
              for (kk = 0; kk < pIdx->nKeyCol; kk++) {
                if (pIdx->azColl[kk] == sqlite3StrBINARY)
                  continue;
                if (label6 == 0)
                  label6 = sqlite3VdbeMakeLabel(pParse);
                sqlite3VdbeAddOp3(v, 96, iIdxCur + j, kk, 3);
                sqlite3VdbeAddOp3(v, 53, 3, label6, r1 + kk);
              }
              if (label6) {
                int jmp6 = sqlite3VdbeAddOp0(v, 9);
                sqlite3VdbeResolveLabel(v, label6);
                sqlite3VdbeLoadString(v, 3, "row ");
                sqlite3VdbeAddOp3(v, 112, 7, 3, 3);
                sqlite3VdbeLoadString(v, 4, " values differ from index ");
                sqlite3VdbeGoto(v, jmp5 - 1);
                sqlite3VdbeJumpHere(v, jmp6);
              }

              if (((pIdx)->onError != 0)) {
                int uniqOk = sqlite3VdbeMakeLabel(pParse);
                int jmp6;
                for (kk = 0; kk < pIdx->nKeyCol; kk++) {
                  int iCol = pIdx->aiColumn[kk];

                  if (iCol >= 0 && pTab->aCol[iCol].notNull)
                    continue;
                  sqlite3VdbeAddOp2(v, 51, r1 + kk, uniqOk);
                }
                jmp6 = sqlite3VdbeAddOp1(v, 40, iIdxCur + j);
                sqlite3VdbeGoto(v, uniqOk);
                sqlite3VdbeJumpHere(v, jmp6);
                sqlite3VdbeAddOp4Int(v, 42, iIdxCur + j, uniqOk, r1, pIdx->nKeyCol);
                sqlite3VdbeLoadString(v, 3, "non-unique entry in index ");
                sqlite3VdbeGoto(v, jmp5);
                sqlite3VdbeResolveLabel(v, uniqOk);
              }
              sqlite3VdbeJumpHere(v, jmp4);
              sqlite3ResolvePartIdxLabel(pParse, jmp3);
            }
          }
          sqlite3VdbeAddOp2(v, 40, iDataCur, loopTop);
          sqlite3VdbeJumpHere(v, loopTop - 1);
          if (pPk) {
            sqlite3ReleaseTempRange(pParse, r2, pPk->nKeyCol);
          }
        }

        for (x = ((pTbls)->first); x; x = ((x)->next)) {
          Table *pTab = (Table*)(((x)->data));
          sqlite3_vtab *pVTab;
          int a1;
          if (tableSkipIntegrityCheck(pTab, pObjTab))
            continue;
          if ((pTab)->eTabType == 0)
            continue;
          if (!((pTab)->eTabType == 1))
            continue;
          if (pTab->nCol <= 0) {
            const char *zMod = pTab->u.vtab.azArg[0];
            if (sqlite3HashFind(&db->aModule, zMod) == 0)
              continue;
          }
          sqlite3ViewGetColumnNames(pParse, pTab);
          if (pTab->u.vtab.p == 0)
            continue;
          pVTab = pTab->u.vtab.p->pVtab;
          if (pVTab == 0)
            continue;
          if (pVTab->pModule == 0)
            continue;
          if (pVTab->pModule->iVersion < 4)
            continue;
          if (pVTab->pModule->xIntegrity == 0)
            continue;
          sqlite3VdbeAddOp3(v, 176, i, 3, isQuick);
          pTab->nTabRef++;
          sqlite3VdbeAppendP4(v, pTab, (-17));
          a1 = sqlite3VdbeAddOp1(v, 51, 3);
          integrityCheckResultRow(v);
          sqlite3VdbeJumpHere(v, a1);
          continue;
        }
      }
      {
        static const int iLn = 0;
        static const VdbeOpList endCode[] = {
            {88, 1, 0, 0}, {62, 1, 4, 0}, {118, 0, 3, 0}, {86, 3, 1, 0}, {72, 0, 0, 0}, {118, 0, 3, 0}, {9, 0, 3, 0},
        };
        VdbeOp *aOp;

        aOp = sqlite3VdbeAddOpList(v, ((int)(sizeof(endCode) / sizeof(endCode[0]))), endCode, iLn);
        if (aOp) {
          aOp[0].p2 = 1 - mxErr;
          aOp[2].p4type = (-1);
          aOp[2].p4.z = (char*)("ok");
          aOp[5].p4type = (-1);
          aOp[5].p4.z = (char *)sqlite3ErrStr(SQLITE_CORRUPT);
        }
        sqlite3VdbeChangeP3(v, 0, sqlite3VdbeCurrentAddr(v) - 2);
      }
    } break;

    case 14: {
      static const struct EncName {
        char *zName;
        u8 enc;
      } encnames[] = {{(char*)("UTF8"), SQLITE_UTF8},
                      {(char*)("UTF-8"), SQLITE_UTF8},
                      {(char*)("UTF-16le"), SQLITE_UTF16LE},
                      {(char*)("UTF-16be"), SQLITE_UTF16BE},
                      {(char*)("UTF16le"), SQLITE_UTF16LE},
                      {(char*)("UTF16be"), SQLITE_UTF16BE},
                      {(char*)("UTF-16"), 0},
                      {(char*)("UTF16"), 0},
                      {0, 0}};
      const struct EncName *pEnc;
      if (!zRight) {
        if (sqlite3ReadSchema(pParse))
          goto pragma_out;

        returnSingleText(v, encnames[((pParse->db)->enc)].zName);
      } else {
        if ((db->mDbFlags & 0x0040) == 0) {
          for (pEnc = &encnames[0]; pEnc->zName; pEnc++) {
            if (0 == sqlite3StrICmp(zRight, pEnc->zName)) {
              u8 enc = pEnc->enc ? pEnc->enc : 2;
              ((db)->aDb[0].pSchema->enc) = enc;
              sqlite3SetTextEncoding(db, enc);
              break;
            }
          }
          if (!pEnc->zName) {
            sqlite3ErrorMsg(pParse, "unsupported encoding: %s", zRight);
          }
        }
      }
    } break;

    case 2: {
      int iCookie = pPragma->iArg;
      sqlite3VdbeUsesBtree(v, iDb);
      if (zRight && (pPragma->mPragFlg & 0x08) == 0) {
        static const VdbeOpList setCookie[] = {
            {2, 0, 1, 0},
            {102, 0, 0, 0},
        };
        VdbeOp *aOp;
        aOp = sqlite3VdbeAddOpList(v, ((int)(sizeof(setCookie) / sizeof(setCookie[0]))), setCookie, 0);
        if ((0))
          break;
        aOp[0].p1 = iDb;
        aOp[1].p1 = iDb;
        aOp[1].p2 = iCookie;
        aOp[1].p3 = sqlite3Atoi(zRight);
        aOp[1].p5 = 1;
        if (iCookie == 1 && (db->flags & 0x10000000) != 0) {
          aOp[1].opcode = 189;
        }
      } else {
        static const VdbeOpList readCookie[] = {{2, 0, 0, 0}, {101, 0, 1, 0}, {86, 1, 1, 0}};
        VdbeOp *aOp;
        aOp = sqlite3VdbeAddOpList(v, ((int)(sizeof(readCookie) / sizeof(readCookie[0]))), readCookie, 0);
        if ((0))
          break;
        aOp[0].p1 = iDb;
        aOp[1].p1 = iDb;
        aOp[1].p3 = iCookie;
        sqlite3VdbeReusable(v);
      }
    } break;

    case 10: {
      int i = 0;
      const char *zOpt;
      pParse->nMem = 1;
      while ((zOpt = sqlite3_compileoption_get(i++)) != 0) {
        sqlite3VdbeLoadString(v, 1, zOpt);
        sqlite3VdbeAddOp2(v, 86, 1, 1);
      }
      sqlite3VdbeReusable(v);
    } break;

    case 43: {
      int iBt = (pId2->z ? iDb : (10 + 2));
      int eMode = SQLITE_CHECKPOINT_PASSIVE;
      if (zRight) {
        if (sqlite3StrICmp(zRight, "full") == 0) {
          eMode = SQLITE_CHECKPOINT_FULL;
        } else if (sqlite3StrICmp(zRight, "restart") == 0) {
          eMode = SQLITE_CHECKPOINT_RESTART;
        } else if (sqlite3StrICmp(zRight, "truncate") == 0) {
          eMode = SQLITE_CHECKPOINT_TRUNCATE;
        } else if (sqlite3StrICmp(zRight, "noop") == 0) {
          eMode = -1;
        }
      }
      pParse->nMem = 3;
      sqlite3VdbeAddOp3(v, 3, iBt, eMode, 1);
      sqlite3VdbeAddOp2(v, 86, 1, 3);
    } break;

    case 42: {
      if (zRight) {
        sqlite3_wal_autocheckpoint(db, sqlite3Atoi(zRight));
      }
      returnSingleInt(v, db->xWalCallback == sqlite3WalDefaultHook ? ((int)(intptr_t)(db->pWalArg)) : 0);
    } break;

    case 34: {
      sqlite3_db_release_memory(db);
      break;
    }

    case 30: {
      int iDbLast;
      int iTabCur;
      HashElem *k;
      Schema *pSchema;
      Table *pTab;
      Index *pIdx;
      LogEst szThreshold;
      char *zSubSql;
      u32 opMask;
      int nLimit;
      int nCheck = 0;
      int nBtree = 0;
      int nIndex;

      if (zRight) {
        opMask = (u32)sqlite3Atoi(zRight);
        if ((opMask & 0x02) == 0)
          break;
      } else {
        opMask = 0xfffe;
      }
      if ((opMask & 0x10) == 0) {
        nLimit = 0;
      } else if (db->nAnalysisLimit > 0 && db->nAnalysisLimit < 2000) {
        nLimit = 0;
      } else {
        nLimit = 2000;
      }
      iTabCur = pParse->nTab++;
      for (iDbLast = zDb ? iDb : db->nDb - 1; iDb <= iDbLast; iDb++) {
        if (iDb == 1)
          continue;
        sqlite3CodeVerifySchema(pParse, iDb);
        pSchema = db->aDb[iDb].pSchema;
        for (k = ((&pSchema->tblHash)->first); k; k = ((k)->next)) {
          pTab = (Table *)((k)->data);

          if (!((pTab)->eTabType == 0))
            continue;

          if (0 == sqlite3_strnicmp(pTab->zName, "sqlite_", 7))
            continue;

          szThreshold = pTab->nRowLogEst;
          nIndex = 0;
          for (pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext) {
            nIndex++;
            if (!pIdx->hasStat1) {
              szThreshold = -1;
            }
          }

          if ((pTab->tabFlags & 0x00000100) != 0) {
          } else if (opMask & 0x10000) {
          } else if (pTab->pIndex != 0 && szThreshold < 0) {
          } else {
            continue;
          }

          nCheck++;
          if (nCheck == 2) {
            sqlite3BeginWriteOperation(pParse, 0, iDb);
          }
          nBtree += nIndex + 1;

          sqlite3OpenTable(pParse, iTabCur, iDb, pTab, 114);
          if (szThreshold >= 0) {
            const LogEst iRange = 33;
            sqlite3VdbeAddOp4Int(v, 33, iTabCur, sqlite3VdbeCurrentAddr(v) + 2 + (opMask & 1),
                                 szThreshold >= iRange ? szThreshold - iRange : -1, szThreshold + iRange);
          } else {
            sqlite3VdbeAddOp2(v, 36, iTabCur, sqlite3VdbeCurrentAddr(v) + 2 + (opMask & 1));
          }
          zSubSql = sqlite3MPrintf(db, "ANALYZE \"%w\".\"%w\"", db->aDb[iDb].zDbSName, pTab->zName);
          if (opMask & 0x01) {
            int r1 = sqlite3GetTempReg(pParse);
            sqlite3VdbeAddOp4(v, 118, 0, r1, 0, zSubSql, (-7));
            sqlite3VdbeAddOp2(v, 86, r1, 1);
          } else {
            sqlite3VdbeAddOp4(v, 150, nLimit ? 0x02 : 00, nLimit, 0, zSubSql, (-7));
          }
        }
      }
      sqlite3VdbeAddOp0(v, 168);

      if (!db->mallocFailed && nLimit > 0 && nBtree > 100) {
        int iAddr, iEnd;
        VdbeOp *aOp;
        nLimit = 100 * nLimit / nBtree;
        if (nLimit < 100)
          nLimit = 100;
        aOp = sqlite3VdbeGetOp(v, 0);
        iEnd = sqlite3VdbeCurrentAddr(v);
        for (iAddr = 0; iAddr < iEnd; iAddr++) {
          if (aOp[iAddr].opcode == 150)
            aOp[iAddr].p2 = nLimit;
        }
      }
      break;
    }

    default: {
      if (zRight) {
        sqlite3_busy_timeout(db, sqlite3Atoi(zRight));
      }
      returnSingleInt(v, db->busyTimeout);
      break;
    }

    case 35: {
      sqlite3_int64 N;
      if (zRight && sqlite3DecOrHexToI64(zRight, &N) == SQLITE_OK) {
        sqlite3_soft_heap_limit64(N);
      }
      returnSingleInt(v, sqlite3_soft_heap_limit64(-1));
      break;
    }

    case 18: {
      sqlite3_int64 N;
      if (zRight && sqlite3DecOrHexToI64(zRight, &N) == SQLITE_OK) {
        sqlite3_int64 iPrior = sqlite3_hard_heap_limit64(-1);
        if (N > 0 && (iPrior == 0 || iPrior > N))
          sqlite3_hard_heap_limit64(N);
      }
      returnSingleInt(v, sqlite3_hard_heap_limit64(-1));
      break;
    }

    case 41: {
      sqlite3_int64 N;
      if (zRight && sqlite3DecOrHexToI64(zRight, &N) == SQLITE_OK && N >= 0) {
        sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, (int)(N & 0x7fffffff));
      }
      returnSingleInt(v, sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, -1));
      break;
    }

    case 1: {
      sqlite3_int64 N;
      if (zRight && sqlite3DecOrHexToI64(zRight, &N) == 0 && N >= 0) {
        db->nAnalysisLimit = (int)(N & 0x7fffffff);
      }
      returnSingleInt(v, db->nAnalysisLimit);
      break;
    }
  }

  if ((pPragma->mPragFlg & 0x04) && zRight) {
  }

pragma_out:
  sqlite3DbFree(db, zLeft);
  sqlite3DbFree(db, zRight);
}

int sqlite3ReadSchema(Parse *pParse) {
  int rc = SQLITE_OK;
  sqlite3 *db = pParse->db;

  if (!db->init.busy) {
    rc = sqlite3Init(db, &pParse->zErrMsg);
    if (rc != SQLITE_OK) {
      pParse->rc = rc;
      pParse->nErr++;
    } else if (db->noSharedCache) {
      db->mDbFlags |= 0x0010;
    }
  }
  return rc;
}

void schemaIsValid(Parse *pParse) {
  sqlite3 *db = pParse->db;
  int iDb;
  int rc;
  int cookie;

  for (iDb = 0; iDb < db->nDb; iDb++) {
    int openedTransaction = 0;
    Btree *pBt = db->aDb[iDb].pBt;
    if (pBt == 0)
      continue;

    if (sqlite3BtreeTxnState(pBt) == SQLITE_TXN_NONE) {
      rc = sqlite3BtreeBeginTrans(pBt, 0, 0);
      if (rc == SQLITE_NOMEM || rc == (10 | (12 << 8))) {
        sqlite3OomFault(db);
        pParse->rc = SQLITE_NOMEM;
      }
      if (rc != SQLITE_OK)
        return;
      openedTransaction = 1;
    }

    sqlite3BtreeGetMeta(pBt, 1, (u32 *)&cookie);

    if (cookie != db->aDb[iDb].pSchema->schema_cookie) {
      if ((((db)->aDb[iDb].pSchema->schemaFlags & (0x0001)) == (0x0001)))
        pParse->rc = SQLITE_SCHEMA;
      sqlite3ResetOneSchema(db, iDb);
    }

    if (openedTransaction) {
      sqlite3BtreeCommit(pBt);
    }
  }
}

void sqlite3ParseObjectReset(Parse *pParse) {
  sqlite3 *db = pParse->db;

  if (pParse->aTableLock)
    sqlite3DbNNFreeNN(db, pParse->aTableLock);

  while (pParse->pCleanup) {
    ParseCleanup *pCleanup = pParse->pCleanup;
    pParse->pCleanup = pCleanup->pNext;
    pCleanup->xCleanup(db, pCleanup->pPtr);
    sqlite3DbNNFreeNN(db, pCleanup);
  }
  if (pParse->aLabel)
    sqlite3DbNNFreeNN(db, pParse->aLabel);
  if (pParse->pConstExpr) {
    sqlite3ExprListDelete(db, pParse->pConstExpr);
  }

  db->lookaside.bDisable -= pParse->disableLookaside;
  db->lookaside.sz = db->lookaside.bDisable ? 0 : db->lookaside.szTrue;

  db->pParse = pParse->pOuterParse;
}

void *sqlite3ParserAddCleanup(Parse *pParse, void (*xCleanup)(sqlite3 *, void *), void *pPtr) {
  ParseCleanup *pCleanup;
  if (sqlite3FaultSim(300)) {
    pCleanup = 0;
    sqlite3OomFault(pParse->db);
  } else {
    pCleanup = (ParseCleanup*)(sqlite3DbMallocRaw(pParse->db, sizeof(*pCleanup)));
  }
  if (pCleanup) {
    pCleanup->pNext = pParse->pCleanup;
    pParse->pCleanup = pCleanup;
    pCleanup->pPtr = pPtr;
    pCleanup->xCleanup = xCleanup;
  } else {
    xCleanup(pParse->db, pPtr);
    pPtr = 0;
  }
  return pPtr;
}

void sqlite3ParseObjectInit(Parse *pParse, sqlite3 *db) {
  memset((((char *)(pParse)) + offsetof(Parse, zErrMsg)), 0, (offsetof(Parse, aTempReg) - offsetof(Parse, zErrMsg)));
  memset((((char *)(pParse)) + offsetof(Parse, sLastToken)), 0, (sizeof(Parse) - offsetof(Parse, sLastToken)));

  pParse->pOuterParse = db->pParse;
  db->pParse = pParse;
  pParse->db = db;
  if (db->mallocFailed)
    sqlite3ErrorMsg(pParse, "out of memory");
}

Select *sqlite3SelectNew(Parse *pParse, ExprList *pEList, SrcList *pSrc, Expr *pWhere, ExprList *pGroupBy,
                         Expr *pHaving, ExprList *pOrderBy, u32 selFlags, Expr *pLimit) {
  Select *pNew, *pAllocated;
  Select standin;
  pAllocated = pNew = (Select*)(sqlite3DbMallocRawNN(pParse->db, sizeof(*pNew)));
  if (pNew == 0) {
    pNew = &standin;
  }
  if (pEList == 0) {
    pEList = sqlite3ExprListAppend(pParse, 0, sqlite3Expr(pParse->db, 180, 0));
  }
  pNew->pEList = pEList;
  pNew->op = 139;
  pNew->selFlags = selFlags;
  pNew->iLimit = 0;
  pNew->iOffset = 0;
  pNew->selId = ++pParse->nSelect;
  pNew->nSelectRow = 0;
  if (pSrc == 0)
    pSrc = (SrcList*)(sqlite3DbMallocZero(pParse->db, (offsetof(SrcList, a) + sizeof(SrcItem))));
  pNew->pSrc = pSrc;
  pNew->pWhere = pWhere;
  pNew->pGroupBy = pGroupBy;
  pNew->pHaving = pHaving;
  pNew->pOrderBy = pOrderBy;
  pNew->pPrior = 0;
  pNew->pNext = 0;
  pNew->pLimit = pLimit;
  pNew->pWith = 0;

  pNew->pWin = 0;
  pNew->pWinDefn = 0;

  if (pParse->db->mallocFailed) {
    clearSelect(pParse->db, pNew, pNew != &standin);
    pAllocated = 0;
  } else {
  }
  return pAllocated;
}

int sqlite3JoinType(Parse *pParse, Token *pA, Token *pB, Token *pC) {
  int jointype = 0;
  Token *apAll[3];
  Token *p;

  static const char zKeyText[] = "naturaleftouterightfullinnercross";
  static const struct {
    u8 i;
    u8 nChar;
    u8 code;
  } aKeyword[] = {
      {0, 7, 0x04},  {6, 4, 0x08 | 0x20},  {10, 5, 0x20}, {14, 5, 0x10 | 0x20}, {19, 4, 0x08 | 0x10 | 0x20},
      {23, 5, 0x01}, {28, 5, 0x01 | 0x02},
  };
  int i, j;
  apAll[0] = pA;
  apAll[1] = pB;
  apAll[2] = pC;
  for (i = 0; i < 3 && apAll[i]; i++) {
    p = apAll[i];
    for (j = 0; j < ((int)(sizeof(aKeyword) / sizeof(aKeyword[0]))); j++) {
      if (p->n == aKeyword[j].nChar && sqlite3_strnicmp((char *)p->z, &zKeyText[aKeyword[j].i], p->n) == 0) {
        jointype |= aKeyword[j].code;
        break;
      }
    };
    if (j >= ((int)(sizeof(aKeyword) / sizeof(aKeyword[0])))) {
      jointype |= 0x80;
      break;
    }
  }
  if ((jointype & (0x01 | 0x20)) == (0x01 | 0x20) || (jointype & 0x80) != 0 ||
      (jointype & (0x20 | 0x08 | 0x10)) == 0x20) {
    const char *zSp1 = " ";
    const char *zSp2 = " ";
    if (pB == 0) {
      zSp1++;
    }
    if (pC == 0) {
      zSp2++;
    }
    sqlite3ErrorMsg(pParse,
                    "unknown join type: "
                    "%T%s%T%s%T",
                    pA, zSp1, pB, zSp2, pC);
    jointype = 0x01;
  }
  return jointype;
}

int sqlite3ProcessJoin(Parse *pParse, Select *p) {
  SrcList *pSrc;
  int i, j;
  SrcItem *pLeft;
  SrcItem *pRight;

  pSrc = p->pSrc;
  pLeft = &pSrc->a[0];
  pRight = &pLeft[1];
  for (i = 0; i < pSrc->nSrc - 1; i++, pRight++, pLeft++) {
    Table *pRightTab = pRight->pSTab;
    u32 joinType;

    if ((pLeft->pSTab == 0 || pRightTab == 0))
      continue;
    joinType = (pRight->fg.jointype & 0x20) != 0 ? 0x000001 : 0x000002;

    if (pRight->fg.jointype & 0x04) {
      IdList *pUsing = 0;
      if (pRight->fg.isUsing || pRight->u3.pOn) {
        sqlite3ErrorMsg(pParse,
                        "a NATURAL join may not have "
                        "an ON or USING clause",
                        0);
        return 1;
      }
      for (j = 0; j < pRightTab->nCol; j++) {
        char *zName;

        if ((((&pRightTab->aCol[j])->colFlags & 0x0002) != 0))
          continue;
        zName = pRightTab->aCol[j].zCnName;
        if (tableAndColumnIndex(pSrc, 0, i, zName, 0, 0, 1)) {
          pUsing = sqlite3IdListAppend(pParse, pUsing, 0);
          if (pUsing) {
            pUsing->a[pUsing->nId - 1].zName = sqlite3DbStrDup(pParse->db, zName);
          }
        }
      }
      if (pUsing) {
        pRight->fg.isUsing = 1;
        pRight->fg.isSynthUsing = 1;
        pRight->u3.pUsing = pUsing;
      }
      if (pParse->nErr)
        return 1;
    }

    if (pRight->fg.isUsing) {
      IdList *pList = pRight->u3.pUsing;
      sqlite3 *db = pParse->db;

      for (j = 0; j < pList->nId; j++) {
        char *zName;
        int iLeft;
        int iLeftCol;
        int iRightCol;
        Expr *pE1;
        Expr *pE2;
        Expr *pEq;

        zName = pList->a[j].zName;
        iRightCol = sqlite3ColumnIndex(pRightTab, zName);
        if (iRightCol < 0 || tableAndColumnIndex(pSrc, 0, i, zName, &iLeft, &iLeftCol, pRight->fg.isSynthUsing) == 0) {
          sqlite3ErrorMsg(pParse,
                          "cannot join using column %s - column "
                          "not present in both tables",
                          zName);
          return 1;
        }
        pE1 = sqlite3CreateColumnExpr(db, pSrc, iLeft, iLeftCol);
        sqlite3SrcItemColumnUsed(&pSrc->a[iLeft], iLeftCol);
        if ((pSrc->a[0].fg.jointype & 0x40) != 0 && pParse->nErr == 0) {
          ExprList *pFuncArgs = 0;
          static const Token tkCoalesce = {"coalesce", 8};

          (pE1)->flags |= (u32)(0x200000);
          while (tableAndColumnIndex(pSrc, iLeft + 1, i, zName, &iLeft, &iLeftCol, pRight->fg.isSynthUsing) != 0) {
            if (pSrc->a[iLeft].fg.isUsing == 0 || sqlite3IdListIndex(pSrc->a[iLeft].u3.pUsing, zName) < 0) {
              sqlite3ErrorMsg(pParse, "ambiguous reference to %s in USING()", zName);
              break;
            }
            pFuncArgs = sqlite3ExprListAppend(pParse, pFuncArgs, pE1);
            pE1 = sqlite3CreateColumnExpr(db, pSrc, iLeft, iLeftCol);
            sqlite3SrcItemColumnUsed(&pSrc->a[iLeft], iLeftCol);
          }
          if (pFuncArgs) {
            pFuncArgs = sqlite3ExprListAppend(pParse, pFuncArgs, pE1);
            pE1 = sqlite3ExprFunction(pParse, pFuncArgs, &tkCoalesce, 0);
            if (pE1) {
              pE1->affExpr = 0x58;
            }
          }
        } else if ((pSrc->a[i + 1].fg.jointype & 0x08) != 0 && pParse->nErr == 0) {
          (pE1)->flags |= (u32)(0x200000);
        }
        pE2 = sqlite3CreateColumnExpr(db, pSrc, i + 1, iRightCol);
        sqlite3SrcItemColumnUsed(pRight, iRightCol);
        pEq = sqlite3PExpr(pParse, 54, pE1, pE2);

        if (pEq) {
          (pEq)->flags |= (u32)(joinType);

          pEq->w.iJoin = pE2->iTable;
        }
        p->pWhere = sqlite3ExprAnd(pParse, p->pWhere, pEq);
      }
    }

    else if (pRight->u3.pOn) {
      sqlite3SetJoinExpr(pRight->u3.pOn, pRight->iCursor, joinType);
      p->pWhere = sqlite3ExprAnd(pParse, p->pWhere, pRight->u3.pOn);
      pRight->u3.pOn = 0;
      pRight->fg.isOn = 1;
      p->selFlags |= 0x40000000;
    }

    if (((pRightTab)->eTabType == 1) && joinType == 0x000001 && pRight->u1.pFuncArg) {
      p->selFlags |= 0x40000000;
    }
  }
  return 0;
}

void innerLoopLoadRow(Parse *pParse, Select *pSelect, RowLoadInfo *pInfo) {
  sqlite3ExprCodeExprList(pParse, pSelect->pEList, pInfo->regResult, 0, pInfo->ecelFlags);
}

int makeSorterRecord(Parse *pParse, SortCtx *pSort, Select *pSelect, int regBase, int nBase) {
  int nOBSat = pSort->nOBSat;
  Vdbe *v = pParse->pVdbe;
  int regOut = ++pParse->nMem;
  if (pSort->pDeferredRowLoad) {
    innerLoopLoadRow(pParse, pSelect, pSort->pDeferredRowLoad);
  }
  sqlite3VdbeAddOp3(v, 99, regBase + nOBSat, nBase - nOBSat, regOut);
  return regOut;
}

void pushOntoSorter(Parse *pParse, SortCtx *pSort, Select *pSelect, int regData, int regOrigData, int nData,
                    int nPrefixReg) {
  Vdbe *v = pParse->pVdbe;
  int bSeq = ((pSort->sortFlags & 0x01) == 0);
  int nExpr = pSort->pOrderBy->nExpr;
  int nBase = nExpr + bSeq + nData;
  int regBase;
  int regRecord = 0;
  int nOBSat = pSort->nOBSat;
  int op;
  int iLimit;
  int iSkip = 0;

  if (nPrefixReg) {
    regBase = regData - nPrefixReg;
  } else {
    regBase = pParse->nMem + 1;
    pParse->nMem += nBase;
  }

  iLimit = pSelect->iOffset ? pSelect->iOffset + 1 : pSelect->iLimit;
  pSort->labelDone = sqlite3VdbeMakeLabel(pParse);
  sqlite3ExprCodeExprList(pParse, pSort->pOrderBy, regBase, regOrigData, 0x01 | (regOrigData ? 0x04 : 0));
  if (bSeq) {
    sqlite3VdbeAddOp2(v, 128, pSort->iECursor, regBase + nExpr);
  }
  if (nPrefixReg == 0 && nData > 0) {
    sqlite3ExprCodeMove(pParse, regData, regBase + nExpr + bSeq, nData);
  }
  if (nOBSat > 0) {
    int regPrevKey;
    int addrFirst;
    int addrJmp;
    VdbeOp *pOp;
    int nKey;
    KeyInfo *pKI;

    regRecord = makeSorterRecord(pParse, pSort, pSelect, regBase, nBase);
    regPrevKey = pParse->nMem + 1;
    pParse->nMem += pSort->nOBSat;
    nKey = nExpr - pSort->nOBSat + bSeq;
    if (bSeq) {
      addrFirst = sqlite3VdbeAddOp1(v, 17, regBase + nExpr);
    } else {
      addrFirst = sqlite3VdbeAddOp1(v, 122, pSort->iECursor);
    };
    sqlite3VdbeAddOp3(v, 92, regPrevKey, regBase, pSort->nOBSat);
    pOp = sqlite3VdbeGetOp(v, pSort->addrSortIndex);
    if (pParse->db->mallocFailed)
      return;
    pOp->p2 = nKey + nData;
    pKI = pOp->p4.pKeyInfo;
    memset(pKI->aSortFlags, 0, pKI->nKeyField);
    sqlite3VdbeChangeP4(v, -1, (char *)pKI, (-9));
    pOp->p4.pKeyInfo = sqlite3KeyInfoFromExprList(pParse, pSort->pOrderBy, nOBSat, pKI->nAllField - pKI->nKeyField - 1);
    pOp = 0;
    addrJmp = sqlite3VdbeCurrentAddr(v);
    sqlite3VdbeAddOp3(v, 14, addrJmp + 1, 0, addrJmp + 1);
    pSort->labelBkOut = sqlite3VdbeMakeLabel(pParse);
    pSort->regReturn = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, 10, pSort->regReturn, pSort->labelBkOut);
    sqlite3VdbeAddOp1(v, 148, pSort->iECursor);
    if (iLimit) {
      sqlite3VdbeAddOp2(v, 17, iLimit, pSort->labelDone);
    }
    sqlite3VdbeJumpHere(v, addrFirst);
    sqlite3ExprCodeMove(pParse, regBase, regPrevKey, pSort->nOBSat);
    sqlite3VdbeJumpHere(v, addrJmp);
  }
  if (iLimit) {
    int iCsr = pSort->iECursor;
    sqlite3VdbeAddOp2(v, 62, iLimit, sqlite3VdbeCurrentAddr(v) + 4);
    sqlite3VdbeAddOp2(v, 32, iCsr, 0);
    iSkip = sqlite3VdbeAddOp4Int(v, 41, iCsr, 0, regBase + nOBSat, nExpr - nOBSat);
    sqlite3VdbeAddOp1(v, 132, iCsr);
  }
  if (regRecord == 0) {
    regRecord = makeSorterRecord(pParse, pSort, pSelect, regBase, nBase);
  }
  if (pSort->sortFlags & 0x01) {
    op = 141;
  } else {
    op = 140;
  }
  sqlite3VdbeAddOp4Int(v, op, pSort->iECursor, regRecord, regBase + nOBSat, nBase - nOBSat);
  if (iSkip) {
    sqlite3VdbeChangeP2(v, iSkip, pSort->labelOBLopt ? pSort->labelOBLopt : sqlite3VdbeCurrentAddr(v));
  }
}

int codeDistinct(Parse *pParse, int eTnctType, int iTab, int addrRepeat, ExprList *pEList, int regElem) {
  int iRet = 0;
  int nResultCol = pEList->nExpr;
  Vdbe *v = pParse->pVdbe;

  switch (eTnctType) {
    case 2: {
      int i;
      int iJump;
      int regPrev;

      iRet = regPrev = pParse->nMem + 1;
      pParse->nMem += nResultCol;

      iJump = sqlite3VdbeCurrentAddr(v) + nResultCol;
      for (i = 0; i < nResultCol; i++) {
        CollSeq *pColl = sqlite3ExprCollSeq(pParse, pEList->a[i].pExpr);
        if (i < nResultCol - 1) {
          sqlite3VdbeAddOp3(v, 53, regElem + i, iJump, regPrev + i);
        } else {
          sqlite3VdbeAddOp3(v, 54, regElem + i, addrRepeat, regPrev + i);
        }
        sqlite3VdbeChangeP4(v, -1, (const char *)pColl, (-2));
        sqlite3VdbeChangeP5(v, 0x80);
      }

      sqlite3VdbeAddOp3(v, 82, regElem, regPrev, nResultCol - 1);
      break;
    }

    case 1: {
      break;
    }

    default: {
      int r1 = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp4Int(v, 29, iTab, addrRepeat, regElem, nResultCol);
      sqlite3VdbeAddOp3(v, 99, regElem, nResultCol, r1);
      sqlite3VdbeAddOp4Int(v, 140, iTab, r1, regElem, nResultCol);
      sqlite3VdbeChangeP5(v, 0x10);
      sqlite3ReleaseTempReg(pParse, r1);
      iRet = iTab;
      break;
    }
  }

  return iRet;
}

void fixDistinctOpenEph(Parse *pParse, int eTnctType, int iVal, int iOpenEphAddr) {
  if (pParse->nErr == 0 && (eTnctType == 1 || eTnctType == 2)) {
    Vdbe *v = pParse->pVdbe;
    sqlite3VdbeChangeToNoop(v, iOpenEphAddr);
    if (sqlite3VdbeGetOp(v, iOpenEphAddr + 1)->opcode == 190) {
      sqlite3VdbeChangeToNoop(v, iOpenEphAddr + 1);
    }
    if (eTnctType == 2) {
      VdbeOp *pOp = sqlite3VdbeGetOp(v, iOpenEphAddr);
      pOp->opcode = 77;
      pOp->p1 = 1;
      pOp->p2 = iVal;
    }
  }
}

void selectInnerLoop(Parse *pParse, Select *p, int srcTab, SortCtx *pSort, DistinctCtx *pDistinct, SelectDest *pDest,
                     int iContinue, int iBreak) {
  Vdbe *v = pParse->pVdbe;
  int i;
  int hasDistinct;
  int eDest = pDest->eDest;
  int iParm = pDest->iSDParm;
  int nResultCol;
  int nPrefixReg = 0;
  RowLoadInfo sRowLoadInfo;

  int regResult;
  int regOrig;

  hasDistinct = pDistinct ? pDistinct->eTnctType : 0;
  if (pSort && pSort->pOrderBy == 0)
    pSort = 0;
  if (pSort == 0 && !hasDistinct) {
    codeOffset(v, p->iOffset, iContinue);
  }

  nResultCol = p->pEList->nExpr;

  if (pDest->iSdst == 0) {
    if (pSort) {
      nPrefixReg = pSort->pOrderBy->nExpr;
      if (!(pSort->sortFlags & 0x01))
        nPrefixReg++;
      pParse->nMem += nPrefixReg;
    }
    pDest->iSdst = pParse->nMem + 1;
    pParse->nMem += nResultCol;
  } else if (pDest->iSdst + nResultCol > pParse->nMem) {
    pParse->nMem += nResultCol;
  }
  pDest->nSdst = nResultCol;
  regOrig = regResult = pDest->iSdst;
  if (srcTab >= 0) {
    for (i = 0; i < nResultCol; i++) {
      sqlite3VdbeAddOp3(v, 96, srcTab, i, regResult + i);
    }
  } else if (eDest != 1) {
    u8 ecelFlags;
    ExprList *pEList;
    if (eDest == 8 || eDest == 7 || eDest == 11) {
      ecelFlags = 0x01;
    } else {
      ecelFlags = 0;
    }
    if (pSort && hasDistinct == 0 && eDest != 10 && eDest != 12) {
      ecelFlags |= (0x08 | 0x04);

      for (i = pSort->nOBSat; i < pSort->pOrderBy->nExpr; i++) {
        int j;
        if ((j = pSort->pOrderBy->a[i].u.x.iOrderByCol) > 0) {
          p->pEList->a[j - 1].u.x.iOrderByCol = i + 1 - pSort->nOBSat;
        }
      }

      pEList = p->pEList;
      for (i = 0; i < pEList->nExpr; i++) {
        if (pEList->a[i].u.x.iOrderByCol > 0) {
          nResultCol--;
          regOrig = 0;
        }
      }
    }
    sRowLoadInfo.regResult = regResult;
    sRowLoadInfo.ecelFlags = ecelFlags;

    if (p->iLimit && (ecelFlags & 0x08) != 0 && nPrefixReg > 0) {
      pSort->pDeferredRowLoad = &sRowLoadInfo;
      regOrig = 0;
    } else {
      innerLoopLoadRow(pParse, p, &sRowLoadInfo);
    }
  }

  if (hasDistinct) {
    int eType = pDistinct->eTnctType;
    int iTab = pDistinct->tabTnct;

    iTab = codeDistinct(pParse, eType, iTab, iContinue, p->pEList, regResult);
    fixDistinctOpenEph(pParse, eType, iTab, pDistinct->addrTnct);
    if (pSort == 0) {
      codeOffset(v, p->iOffset, iContinue);
    }
  }

  switch (eDest) {
    case 6:
    case 3:
    case 12:
    case 10: {
      int r1 = sqlite3GetTempRange(pParse, nPrefixReg + 1);
      sqlite3VdbeAddOp3(v, 99, regResult, nResultCol, r1 + nPrefixReg);

      if (eDest == 3) {
        int addr = sqlite3VdbeCurrentAddr(v) + 4;
        sqlite3VdbeAddOp4Int(v, 29, iParm + 1, addr, r1, 0);
        sqlite3VdbeAddOp4Int(v, 140, iParm + 1, r1, regResult, nResultCol);
      }

      if (pSort) {
        pushOntoSorter(pParse, pSort, p, r1 + nPrefixReg, regOrig, 1, nPrefixReg);
      } else {
        int r2 = sqlite3GetTempReg(pParse);
        sqlite3VdbeAddOp2(v, 129, iParm, r2);
        sqlite3VdbeAddOp3(v, 130, iParm, r1, r2);
        sqlite3VdbeChangeP5(v, 0x08);
        sqlite3ReleaseTempReg(pParse, r2);
      }
      sqlite3ReleaseTempRange(pParse, r1, nPrefixReg + 1);
      break;
    }

    case 13: {
      if (pSort) {
        pushOntoSorter(pParse, pSort, p, regResult, regOrig, nResultCol, nPrefixReg);
      } else {
        int i2 = pDest->iSDParm2;
        int r1 = sqlite3GetTempReg(pParse);

        sqlite3VdbeAddOp2(v, 51, regResult, iBreak);

        sqlite3VdbeAddOp3(v, 99, regResult + (i2 < 0), nResultCol - (i2 < 0), r1);
        if (i2 < 0) {
          sqlite3VdbeAddOp3(v, 130, iParm, r1, regResult);
        } else {
          sqlite3VdbeAddOp4Int(v, 140, iParm, r1, regResult, i2);
        }
      }
      break;
    }

    case 9: {
      if (pSort) {
        pushOntoSorter(pParse, pSort, p, regResult, regOrig, nResultCol, nPrefixReg);
        pDest->iSDParm2 = 0;
      } else {
        int r1 = sqlite3GetTempReg(pParse);

        sqlite3VdbeAddOp4(v, 99, regResult, nResultCol, r1, pDest->zAffSdst, nResultCol);
        sqlite3VdbeAddOp4Int(v, 140, iParm, r1, regResult, nResultCol);
        if (pDest->iSDParm2) {
          sqlite3VdbeAddOp4Int(v, 185, pDest->iSDParm2, 0, regResult, nResultCol);
          sqlite3VdbeExplain(pParse, 0, "CREATE BLOOM FILTER");
        }
        sqlite3ReleaseTempReg(pParse, r1);
      }
      break;
    }

    case 1: {
      sqlite3VdbeAddOp2(v, 73, 1, iParm);

      break;
    }

    case 8: {
      if (pSort) {
        pushOntoSorter(pParse, pSort, p, regResult, regOrig, nResultCol, nPrefixReg);
        pDest->iSDParm = regResult;
      } else {
        if (regResult != iParm) {
          sqlite3VdbeAddOp3(v, 82, regResult, iParm, nResultCol - 1);
        }
      }
      break;
    }

    case 11:
    case 7: {
      if (pSort) {
        pushOntoSorter(pParse, pSort, p, regResult, regOrig, nResultCol, nPrefixReg);
      } else if (eDest == 11) {
        sqlite3VdbeAddOp1(v, 12, pDest->iSDParm);
      } else {
        sqlite3VdbeAddOp2(v, 86, regResult, nResultCol);
      }
      break;
    }

    case 4:
    case 5: {
      int nKey;
      int r1, r2, r3;
      int addrTest = 0;
      ExprList *pSO;
      pSO = pDest->pOrderBy;

      nKey = pSO->nExpr;
      r1 = sqlite3GetTempReg(pParse);
      r2 = sqlite3GetTempRange(pParse, nKey + 2);
      r3 = r2 + nKey + 1;
      if (eDest == 4) {
        addrTest = sqlite3VdbeAddOp4Int(v, 29, iParm + 1, 0, regResult, nResultCol);
      }
      sqlite3VdbeAddOp3(v, 99, regResult, nResultCol, r3);
      if (eDest == 4) {
        sqlite3VdbeAddOp2(v, 140, iParm + 1, r3);
        sqlite3VdbeChangeP5(v, 0x10);
      }
      for (i = 0; i < nKey; i++) {
        sqlite3VdbeAddOp2(v, 83, regResult + pSO->a[i].u.x.iOrderByCol - 1, r2 + i);
      }
      sqlite3VdbeAddOp2(v, 128, iParm, r2 + nKey);
      sqlite3VdbeAddOp3(v, 99, r2, nKey + 2, r1);
      sqlite3VdbeAddOp4Int(v, 140, iParm, r1, r2, nKey + 2);
      if (addrTest)
        sqlite3VdbeJumpHere(v, addrTest);
      sqlite3ReleaseTempReg(pParse, r1);
      sqlite3ReleaseTempRange(pParse, r2, nKey + 2);
      break;
    }

    default: {
      break;
    }
  }

  if (pSort == 0 && p->iLimit) {
    sqlite3VdbeAddOp2(v, 63, p->iLimit, iBreak);
  }
}

KeyInfo *sqlite3KeyInfoFromExprList(Parse *pParse, ExprList *pList, int iStart, int nExtra) {
  int nExpr;
  KeyInfo *pInfo;
  struct ExprList_item *pItem;
  sqlite3 *db = pParse->db;
  int i;

  nExpr = pList->nExpr;
  pInfo = sqlite3KeyInfoAlloc(db, nExpr - iStart, nExtra + 1);
  if (pInfo) {
    for (i = iStart, pItem = pList->a + iStart; i < nExpr; i++, pItem++) {
      pInfo->aColl[i - iStart] = sqlite3ExprNNCollSeq(pParse, pItem->pExpr);
      pInfo->aSortFlags[i - iStart] = pItem->fg.sortFlags;
    }
  }
  return pInfo;
}

void explainTempTable(Parse *pParse, const char *zUsage) {
  sqlite3VdbeExplain(pParse, 0, "USE TEMP B-TREE FOR %s", zUsage);
}

void generateSortTail(Parse *pParse, Select *p, SortCtx *pSort, int nColumn, SelectDest *pDest) {
  Vdbe *v = pParse->pVdbe;
  int addrBreak = pSort->labelDone;
  int addrContinue = sqlite3VdbeMakeLabel(pParse);
  int addr;
  int addrOnce = 0;
  int iTab;
  ExprList *pOrderBy = pSort->pOrderBy;
  int eDest = pDest->eDest;
  int iParm = pDest->iSDParm;
  int regRow;
  int regRowid;
  int iCol;
  int nKey;
  int iSortTab;
  int i;
  int bSeq;
  int nRefKey = 0;
  struct ExprList_item *aOutEx = p->pEList->a;

  nKey = pOrderBy->nExpr - pSort->nOBSat;
  if (pSort->nOBSat == 0 || nKey == 1) {
    sqlite3VdbeExplain(pParse, 0, "USE TEMP B-TREE FOR %sORDER BY", pSort->nOBSat ? "LAST TERM OF " : "");
  } else {
    sqlite3VdbeExplain(pParse, 0, "USE TEMP B-TREE FOR LAST %d TERMS OF ORDER BY", nKey);
  };

  if (pSort->labelBkOut) {
    sqlite3VdbeAddOp2(v, 10, pSort->regReturn, pSort->labelBkOut);
    sqlite3VdbeGoto(v, addrBreak);
    sqlite3VdbeResolveLabel(v, pSort->labelBkOut);
  }

  iTab = pSort->iECursor;
  if (eDest == 7 || eDest == 11 || eDest == 8) {
    if (eDest == 8 && p->iOffset) {
      sqlite3VdbeAddOp2(v, 77, 0, pDest->iSdst);
    }
    regRowid = 0;
    regRow = pDest->iSdst;
  } else {
    regRowid = sqlite3GetTempReg(pParse);
    if (eDest == 10 || eDest == 12) {
      regRow = sqlite3GetTempReg(pParse);
      nColumn = 0;
    } else {
      regRow = sqlite3GetTempRange(pParse, nColumn);
    }
  }
  if (pSort->sortFlags & 0x01) {
    int regSortOut = ++pParse->nMem;
    iSortTab = pParse->nTab++;
    if (pSort->labelBkOut) {
      addrOnce = sqlite3VdbeAddOp0(v, 15);
    }
    sqlite3VdbeAddOp3(v, 123, iSortTab, regSortOut, nKey + 1 + nColumn + nRefKey);
    if (addrOnce)
      sqlite3VdbeJumpHere(v, addrOnce);
    addr = 1 + sqlite3VdbeAddOp2(v, 34, iTab, addrBreak);

    sqlite3VdbeAddOp3(v, 135, iTab, regSortOut, iSortTab);
    bSeq = 0;
  } else {
    addr = 1 + sqlite3VdbeAddOp2(v, 35, iTab, addrBreak);
    codeOffset(v, p->iOffset, addrContinue);
    iSortTab = iTab;
    bSeq = 1;
    if (p->iOffset > 0) {
      sqlite3VdbeAddOp2(v, 88, p->iLimit, -1);
    }
  }
  for (i = 0, iCol = nKey + bSeq - 1; i < nColumn; i++) {
    if (aOutEx[i].u.x.iOrderByCol == 0)
      iCol++;
  }

  for (i = nColumn - 1; i >= 0; i--) {
    {
      int iRead;
      if (aOutEx[i].u.x.iOrderByCol) {
        iRead = aOutEx[i].u.x.iOrderByCol - 1;
      } else {
        iRead = iCol--;
      }
      sqlite3VdbeAddOp3(v, 96, iSortTab, iRead, regRow + i);
    }
  };
  switch (eDest) {
    case 12:
    case 10: {
      sqlite3VdbeAddOp3(v, 96, iSortTab, nKey + bSeq, regRow);
      sqlite3VdbeAddOp2(v, 129, iParm, regRowid);
      sqlite3VdbeAddOp3(v, 130, iParm, regRow, regRowid);
      sqlite3VdbeChangeP5(v, 0x08);
      break;
    }

    case 9: {
      sqlite3VdbeAddOp4(v, 99, regRow, nColumn, regRowid, pDest->zAffSdst, nColumn);
      sqlite3VdbeAddOp4Int(v, 140, iParm, regRowid, regRow, nColumn);
      break;
    }
    case 8: {
      break;
    }

    case 13: {
      int i2 = pDest->iSDParm2;
      int r1 = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp3(v, 99, regRow + (i2 < 0), nColumn - (i2 < 0), r1);
      if (i2 < 0) {
        sqlite3VdbeAddOp3(v, 130, iParm, r1, regRow);
      } else {
        sqlite3VdbeAddOp4Int(v, 140, iParm, r1, regRow, i2);
      }
      break;
    }
    default: {
      if (eDest == 7) {
        sqlite3VdbeAddOp2(v, 86, pDest->iSdst, nColumn);
      } else {
        sqlite3VdbeAddOp1(v, 12, pDest->iSDParm);
      }
      break;
    }
  }
  if (regRowid) {
    if (eDest == 9) {
      sqlite3ReleaseTempRange(pParse, regRow, nColumn);
    } else {
      sqlite3ReleaseTempReg(pParse, regRow);
    }
    sqlite3ReleaseTempReg(pParse, regRowid);
  }

  sqlite3VdbeResolveLabel(v, addrContinue);
  if (pSort->sortFlags & 0x01) {
    sqlite3VdbeAddOp2(v, 38, iTab, addr);
  } else {
    sqlite3VdbeAddOp2(v, 40, iTab, addr);
  };
  if (pSort->regReturn)
    sqlite3VdbeAddOp1(v, 69, pSort->regReturn);
  sqlite3VdbeResolveLabel(v, addrBreak);
}

void generateColumnTypes(Parse *pParse, SrcList *pTabList, ExprList *pEList) {
  Vdbe *v = pParse->pVdbe;
  int i;
  NameContext sNC;
  sNC.pSrcList = pTabList;
  sNC.pParse = pParse;
  sNC.pNext = 0;
  for (i = 0; i < pEList->nExpr; i++) {
    Expr *p = pEList->a[i].pExpr;
    const char *zType;

    zType = columnTypeImpl(&sNC, p);

    sqlite3VdbeSetColName(v, i, 1, zType, ((sqlite3_destructor_type)-1));
  }
}

void sqlite3GenerateColumnNames(Parse *pParse, Select *pSelect) {
  Vdbe *v = pParse->pVdbe;
  int i;
  Table *pTab;
  SrcList *pTabList;
  ExprList *pEList;
  sqlite3 *db = pParse->db;
  int fullName;
  int srcName;

  if (pParse->colNamesSet)
    return;

  while (pSelect->pPrior)
    pSelect = pSelect->pPrior;
  pTabList = pSelect->pSrc;
  pEList = pSelect->pEList;

  pParse->colNamesSet = 1;
  fullName = (db->flags & 0x00000004) != 0;
  srcName = (db->flags & 0x00000040) != 0 || fullName;
  sqlite3VdbeSetNumCols(v, pEList->nExpr);
  for (i = 0; i < pEList->nExpr; i++) {
    Expr *p = pEList->a[i].pExpr;

    if (pEList->a[i].zEName && pEList->a[i].fg.eEName == 0) {
      char *zName = pEList->a[i].zEName;
      sqlite3VdbeSetColName(v, i, 0, zName, ((sqlite3_destructor_type)-1));
    } else if (srcName && p->op == 168) {
      char *zCol;
      int iCol = p->iColumn;
      pTab = p->y.pTab;

      if (iCol < 0)
        iCol = pTab->iPKey;

      if (iCol < 0) {
        zCol = (char*)("rowid");
      } else {
        zCol = pTab->aCol[iCol].zCnName;
      }
      if (fullName) {
        char *zName = 0;
        zName = sqlite3MPrintf(db, "%s.%s", pTab->zName, zCol);
        sqlite3VdbeSetColName(v, i, 0, zName, ((sqlite3_destructor_type)sqlite3RowSetClear));
      } else {
        sqlite3VdbeSetColName(v, i, 0, zCol, ((sqlite3_destructor_type)-1));
      }
    } else {
      const char *z = pEList->a[i].zEName;
      z = z == 0 ? sqlite3MPrintf(db, "column%d", i + 1) : sqlite3DbStrDup(db, z);
      sqlite3VdbeSetColName(v, i, 0, z, ((sqlite3_destructor_type)sqlite3RowSetClear));
    }
  }
  generateColumnTypes(pParse, pTabList, pEList);
}

int sqlite3ColumnsFromExprList(Parse *pParse, ExprList *pEList, i16 *pnCol, Column **paCol) {
  sqlite3 *db = pParse->db;
  int i, j;
  u32 cnt;
  Column *aCol, *pCol;
  int nCol;
  char *zName;
  int nName;
  Hash ht;
  Table *pTab;

  sqlite3HashInit(&ht);
  if (pEList) {
    nCol = pEList->nExpr;
    aCol = (Column*)(sqlite3DbMallocZero(db, sizeof(aCol[0]) * nCol));
    if ((nCol > 32767))
      nCol = 32767;
  } else {
    nCol = 0;
    aCol = 0;
  }

  *pnCol = nCol;
  *paCol = aCol;

  for (i = 0, pCol = aCol; i < nCol && !pParse->nErr; i++, pCol++) {
    struct ExprList_item *pX = &pEList->a[i];
    struct ExprList_item *pCollide;

    if ((zName = pX->zEName) != 0 && pX->fg.eEName == 0) {
    } else {
      Expr *pColExpr = sqlite3ExprSkipCollateAndLikely(pX->pExpr);
      while ((pColExpr != 0) && pColExpr->op == 142) {
        pColExpr = pColExpr->pRight;
      }
      if (pColExpr->op == 168 && ((((pColExpr)->flags & (0x1000000 | 0x2000000)) == 0)) && (pColExpr->y.pTab != 0)) {
        int iCol = pColExpr->iColumn;
        pTab = pColExpr->y.pTab;
        if (iCol < 0)
          iCol = pTab->iPKey;
        zName = iCol >= 0 ? pTab->aCol[iCol].zCnName : (char *)"rowid";
      } else if (pColExpr->op == 60) {
        zName = pColExpr->u.zToken;
      } else {
      }
    }
    if (zName && !sqlite3IsTrueOrFalse(zName)) {
      zName = sqlite3DbStrDup(db, zName);
    } else {
      zName = sqlite3MPrintf(db, "column%d", i + 1);
    }

    cnt = 0;
    while (zName && (pCollide = (ExprList_item*)(sqlite3HashFind(&ht, zName))) != 0) {
      if (pCollide->fg.bUsingTerm) {
        pCol->colFlags |= 0x0400;
      }
      nName = sqlite3Strlen30(zName);
      if (nName > 0) {
        for (j = nName - 1; j > 0 && (sqlite3CtypeMap[(unsigned char)(zName[j])] & 0x04); j--) {
        }
        if (zName[j] == ':')
          nName = j;
      }
      zName = sqlite3MPrintf(db, "%.*z:%u", nName, zName, ++cnt);
      sqlite3ProgressCheck(pParse);
      if (cnt > 3) {
        sqlite3_randomness(sizeof(cnt), &cnt);
      }
    }
    pCol->zCnName = zName;
    pCol->hName = sqlite3StrIHash(zName);
    if (pX->fg.bNoExpand) {
      pCol->colFlags |= 0x0400;
    };
    if (zName && sqlite3HashInsert(&ht, zName, pX) == pX) {
      sqlite3OomFault(db);
    }
  }
  sqlite3HashClear(&ht);
  if (pParse->nErr) {
    for (j = 0; j < i; j++) {
      sqlite3DbFree(db, aCol[j].zCnName);
    }
    sqlite3DbFree(db, aCol);
    *paCol = 0;
    *pnCol = 0;
    return pParse->rc;
  }
  return SQLITE_OK;
}

void sqlite3SubqueryColumnTypes(Parse *pParse, Table *pTab, Select *pSelect, char aff) {
  sqlite3 *db = pParse->db;
  Column *pCol;
  CollSeq *pColl;
  int i, j;
  Expr *p;
  struct ExprList_item *a;
  NameContext sNC;

  if (db->mallocFailed || (pParse->eParseMode >= 2))
    return;
  while (pSelect->pPrior)
    pSelect = pSelect->pPrior;
  a = pSelect->pEList->a;
  memset(&sNC, 0, sizeof(sNC));
  sNC.pSrcList = pSelect->pSrc;
  for (i = 0, pCol = pTab->aCol; i < pTab->nCol; i++, pCol++) {
    const char *zType;
    i64 n;
    int m = 0;
    Select *pS2 = pSelect;
    pTab->tabFlags |= (pCol->colFlags & 0x0062);
    p = a[i].pExpr;

    pCol->affinity = sqlite3ExprAffinity(p);
    while (pCol->affinity <= 0x40 && pS2->pNext != 0) {
      m |= sqlite3ExprDataType(pS2->pEList->a[i].pExpr);
      pS2 = pS2->pNext;
      pCol->affinity = sqlite3ExprAffinity(pS2->pEList->a[i].pExpr);
    }
    if (pCol->affinity <= 0x40) {
      pCol->affinity = aff;
    }
    if (pCol->affinity >= 0x42 && (pS2->pNext || pS2 != pSelect)) {
      for (pS2 = pS2->pNext; pS2; pS2 = pS2->pNext) {
        m |= sqlite3ExprDataType(pS2->pEList->a[i].pExpr);
      }
      if (pCol->affinity == 0x42 && (m & 0x01) != 0) {
        pCol->affinity = 0x41;
      } else if (pCol->affinity >= 0x43 && (m & 0x02) != 0) {
        pCol->affinity = 0x41;
      }
      if (pCol->affinity >= 0x43 && p->op == 36) {
        pCol->affinity = 0x46;
      }
    }
    zType = columnTypeImpl(&sNC, p);
    if (zType == 0 || pCol->affinity != sqlite3AffinityType(zType, 0)) {
      if (pCol->affinity == 0x43 || pCol->affinity == 0x46) {
        zType = "NUM";
      } else {
        zType = 0;
        for (j = 1; j < 6; j++) {
          if (sqlite3StdTypeAffinity[j] == pCol->affinity) {
            zType = sqlite3StdType[j];
            break;
          }
        }
      }
    }
    if (zType) {
      const i64 k = strlen(zType);
      n = strlen(pCol->zCnName);
      pCol->zCnName = (char*)(sqlite3DbReallocOrFree(db, pCol->zCnName, n + k + 2));
      pCol->colFlags &= ~(0x0004 | 0x0200);
      if (pCol->zCnName) {
        memcpy(&pCol->zCnName[n + 1], zType, k + 1);
        pCol->colFlags |= 0x0004;
      }
    }
    pColl = sqlite3ExprCollSeq(pParse, p);
    if (pColl) {
      sqlite3ColumnSetColl(db, pCol, pColl->zName);
    }
  }
  pTab->szTabRow = 1;
}

Table *sqlite3ResultSetOfSelect(Parse *pParse, Select *pSelect, char aff) {
  Table *pTab;
  sqlite3 *db = pParse->db;
  u64 savedFlags;

  pParse->nNestSel++;

  if (pParse->nNestSel >= db->aLimit[SQLITE_LIMIT_EXPR_DEPTH]) {
    sqlite3ErrorMsg(pParse, "VIEWs and/or subqueries nested too deep");
    return 0;
  }

  savedFlags = db->flags;
  db->flags &= ~(u64)0x00000004;
  db->flags |= 0x00000040;
  sqlite3SelectPrep(pParse, pSelect, 0);
  db->flags = savedFlags;
  if (pParse->nErr)
    return 0;
  while (pSelect->pPrior)
    pSelect = pSelect->pPrior;
  pTab = (Table*)(sqlite3DbMallocZero(db, sizeof(Table)));
  if (pTab == 0) {
    return 0;
  }
  pTab->nTabRef = 1;
  pTab->zName = 0;
  pTab->nRowLogEst = 200;

  sqlite3ColumnsFromExprList(pParse, pSelect->pEList, &pTab->nCol, &pTab->aCol);
  sqlite3SubqueryColumnTypes(pParse, pTab, pSelect, aff);
  pTab->iPKey = -1;
  if (db->mallocFailed) {
    sqlite3DeleteTable(db, pTab);
    return 0;
  }
  pParse->nNestSel--;

  return pTab;
}

Vdbe *sqlite3GetVdbe(Parse *pParse) {
  if (pParse->pVdbe) {
    return pParse->pVdbe;
  }
  if (pParse->pToplevel == 0 && (((pParse->db)->dbOptFlags & (0x00000008)) == 0)) {
    pParse->okConstFactor = 1;
  }
  return sqlite3VdbeCreate(pParse);
}

void computeLimitRegisters(Parse *pParse, Select *p, int iBreak) {
  Vdbe *v = 0;
  int iLimit = 0;
  int iOffset;
  int n;
  Expr *pLimit = p->pLimit;

  if (p->iLimit)
    return;

  if (pLimit) {
    p->iLimit = iLimit = ++pParse->nMem;
    v = sqlite3GetVdbe(pParse);

    if (sqlite3ExprIsInteger(pLimit->pLeft, &n, pParse)) {
      sqlite3VdbeAddOp2(v, 73, n, iLimit);
      if (n == 0) {
        sqlite3VdbeGoto(v, iBreak);
      } else if (n >= 0 && p->nSelectRow > sqlite3LogEst((u64)n)) {
        p->nSelectRow = sqlite3LogEst((u64)n);
        p->selFlags |= 0x0004000;
      }
    } else {
      sqlite3ExprCode(pParse, pLimit->pLeft, iLimit);
      sqlite3VdbeAddOp1(v, 13, iLimit);
      sqlite3VdbeAddOp2(v, 17, iLimit, iBreak);
    }
    if (pLimit->pRight) {
      p->iOffset = iOffset = ++pParse->nMem;
      pParse->nMem++;
      sqlite3ExprCode(pParse, pLimit->pRight, iOffset);
      sqlite3VdbeAddOp1(v, 13, iOffset);
      sqlite3VdbeAddOp3(v, 162, iLimit, iOffset + 1, iOffset);
    }
  }
}

CollSeq *multiSelectCollSeq(Parse *pParse, Select *p, int iCol) {
  CollSeq *pRet;
  if (p->pPrior) {
    pRet = multiSelectCollSeq(pParse, p->pPrior, iCol);
  } else {
    pRet = 0;
  }

  if (pRet == 0 && (iCol < p->pEList->nExpr)) {
    pRet = sqlite3ExprCollSeq(pParse, p->pEList->a[iCol].pExpr);
  }
  return pRet;
}

KeyInfo *multiSelectByMergeKeyInfo(Parse *pParse, Select *p, int nExtra) {
  ExprList *pOrderBy = p->pOrderBy;
  int nOrderBy = (pOrderBy != 0) ? pOrderBy->nExpr : 0;
  sqlite3 *db = pParse->db;
  KeyInfo *pRet = sqlite3KeyInfoAlloc(db, nOrderBy + nExtra, 1);
  if (pRet) {
    int i;
    for (i = 0; i < nOrderBy; i++) {
      struct ExprList_item *pItem = &pOrderBy->a[i];
      Expr *pTerm = pItem->pExpr;
      CollSeq *pColl;

      if (pTerm->flags & 0x000200) {
        pColl = sqlite3ExprCollSeq(pParse, pTerm);
      } else {
        pColl = multiSelectCollSeq(pParse, p, pItem->u.x.iOrderByCol - 1);
        if (pColl == 0)
          pColl = db->pDfltColl;
        pOrderBy->a[i].pExpr = sqlite3ExprAddCollateString(pParse, pTerm, pColl->zName);
      }

      pRet->aColl[i] = pColl;
      pRet->aSortFlags[i] = pOrderBy->a[i].fg.sortFlags;
    }
  }

  return pRet;
}

void generateWithRecursiveQuery(Parse *pParse, Select *p, SelectDest *pDest) {
  SrcList *pSrc = p->pSrc;
  int nCol = p->pEList->nExpr;
  Vdbe *v = pParse->pVdbe;
  Select *pSetup;
  Select *pFirstRec;
  int addrTop;
  int addrCont, addrBreak;
  int iCurrent = 0;
  int regCurrent;
  int iQueue;
  int iDistinct = 0;
  int eDest = 6;
  SelectDest destQueue;
  int i;
  int rc;
  ExprList *pOrderBy;
  Expr *pLimit;
  int regLimit, regOffset;

  if (p->pWin) {
    sqlite3ErrorMsg(pParse, "cannot use window functions in recursive queries");
    return;
  }

  if (sqlite3AuthCheck(pParse, SQLITE_RECURSIVE, 0, 0, 0))
    return;

  addrBreak = sqlite3VdbeMakeLabel(pParse);
  p->nSelectRow = 320;
  computeLimitRegisters(pParse, p, addrBreak);
  pLimit = p->pLimit;
  regLimit = p->iLimit;
  regOffset = p->iOffset;
  p->pLimit = 0;
  p->iLimit = p->iOffset = 0;
  pOrderBy = p->pOrderBy;

  for (i = 0; (i < pSrc->nSrc); i++) {
    if (pSrc->a[i].fg.isRecursive) {
      iCurrent = pSrc->a[i].iCursor;
      break;
    }
  }

  iQueue = pParse->nTab++;
  if (p->op == 135) {
    eDest = pOrderBy ? 4 : 3;
    iDistinct = pParse->nTab++;
  } else {
    eDest = pOrderBy ? 5 : 6;
  }
  sqlite3SelectDestInit(&destQueue, eDest, iQueue);

  regCurrent = ++pParse->nMem;
  sqlite3VdbeAddOp3(v, 123, iCurrent, regCurrent, nCol);
  if (pOrderBy) {
    KeyInfo *pKeyInfo = multiSelectByMergeKeyInfo(pParse, p, 1);
    sqlite3VdbeAddOp4(v, 120, iQueue, pOrderBy->nExpr + 2, 0, (char *)pKeyInfo, (-9));
    destQueue.pOrderBy = pOrderBy;
  } else {
    sqlite3VdbeAddOp2(v, 120, iQueue, nCol);
  };
  if (iDistinct) {
    KeyInfo *pKeyInfo;
    CollSeq **apColl;

    nCol = p->pEList->nExpr;
    pKeyInfo = sqlite3KeyInfoAlloc(pParse->db, nCol, 1);
    if (pKeyInfo) {
      for (i = 0, apColl = pKeyInfo->aColl; i < nCol; i++, apColl++) {
        *apColl = multiSelectCollSeq(pParse, p, i);
        if (0 == *apColl) {
          *apColl = pParse->db->pDfltColl;
        }
      }
      sqlite3VdbeAddOp4(v, 120, iDistinct, nCol, 0, (const char*)((void *)pKeyInfo), (-9));
    } else {
    }
  }

  p->pOrderBy = 0;

  for (pFirstRec = p; (pFirstRec != 0); pFirstRec = pFirstRec->pPrior) {
    if (pFirstRec->selFlags & 0x0000008) {
      sqlite3ErrorMsg(pParse, "recursive aggregate queries not supported");
      goto end_of_recursive_query;
    }
    pFirstRec->op = 136;
    if ((pFirstRec->pPrior->selFlags & 0x0002000) == 0)
      break;
  }

  pSetup = pFirstRec->pPrior;
  pSetup->pNext = 0;
  sqlite3VdbeExplain(pParse, 1, "SETUP");
  rc = sqlite3Select(pParse, pSetup, &destQueue);
  pSetup->pNext = p;
  if (rc)
    goto end_of_recursive_query;

  addrTop = sqlite3VdbeAddOp2(v, 36, iQueue, addrBreak);

  sqlite3VdbeAddOp1(v, 138, iCurrent);
  if (pOrderBy) {
    sqlite3VdbeAddOp3(v, 96, iQueue, pOrderBy->nExpr + 1, regCurrent);
  } else {
    sqlite3VdbeAddOp2(v, 136, iQueue, regCurrent);
  }
  sqlite3VdbeAddOp1(v, 132, iQueue);

  addrCont = sqlite3VdbeMakeLabel(pParse);
  codeOffset(v, regOffset, addrCont);
  selectInnerLoop(pParse, p, iCurrent, 0, 0, pDest, addrCont, addrBreak);
  if (regLimit) {
    sqlite3VdbeAddOp2(v, 63, regLimit, addrBreak);
  }
  sqlite3VdbeResolveLabel(v, addrCont);

  pFirstRec->pPrior = 0;
  sqlite3VdbeExplain(pParse, 1, "RECURSIVE STEP");
  sqlite3Select(pParse, p, &destQueue);

  pFirstRec->pPrior = pSetup;

  sqlite3VdbeGoto(v, addrTop);
  sqlite3VdbeResolveLabel(v, addrBreak);

end_of_recursive_query:
  sqlite3ExprListDelete(pParse->db, p->pOrderBy);
  p->pOrderBy = pOrderBy;
  p->pLimit = pLimit;
  return;
}

int multiSelectValues(Parse *pParse, Select *p, SelectDest *pDest) {
  int nRow = 1;
  int rc = 0;
  int bShowAll = p->pLimit == 0;

  do {
    if (p->pWin)
      return -1;

    if (p->pPrior == 0)
      break;

    p = p->pPrior;
    nRow += bShowAll;
  } while (1);
  sqlite3VdbeExplain(pParse, 0, "SCAN %d CONSTANT ROW%s", nRow, nRow == 1 ? "" : "S");
  while (p) {
    selectInnerLoop(pParse, p, -1, 0, 0, pDest, 1, 1);
    if (!bShowAll)
      break;
    p->nSelectRow = nRow;
    p = p->pNext;
  }
  return rc;
}

int multiSelect(Parse *pParse, Select *p, SelectDest *pDest) {
  int rc = SQLITE_OK;
  Select *pPrior;
  Vdbe *v;
  SelectDest dest;
  Select *pDelete = 0;
  sqlite3 *db;

  db = pParse->db;
  pPrior = p->pPrior;
  dest = *pDest;

  v = sqlite3GetVdbe(pParse);

  if (dest.eDest == 10) {
    sqlite3VdbeAddOp2(v, 120, dest.iSDParm, p->pEList->nExpr);
    dest.eDest = 12;
  }

  if (p->selFlags & 0x0000400) {
    rc = multiSelectValues(pParse, p, &dest);
    if (rc >= 0)
      goto multi_select_end;
    rc = SQLITE_OK;
  }

  if ((p->selFlags & 0x0002000) != 0 && hasAnchor(p)) {
    generateWithRecursiveQuery(pParse, p, &dest);
  } else if (p->pOrderBy) {
    return multiSelectByMerge(pParse, p, pDest);
  } else if (p->op != 136) {
    Expr *pOne = sqlite3ExprInt32(db, 1);
    p->pOrderBy = sqlite3ExprListAppend(pParse, 0, pOne);
    if (pParse->nErr)
      goto multi_select_end;

    p->pOrderBy->a[0].u.x.iOrderByCol = 1;
    return multiSelectByMerge(pParse, p, pDest);
  } else {
    int addr = 0;
    int nLimit = 0;

    if (pPrior->pPrior == 0) {
      sqlite3VdbeExplain(pParse, 1, "COMPOUND QUERY");
      sqlite3VdbeExplain(pParse, 1, "LEFT-MOST SUBQUERY");
    }

    pPrior->iLimit = p->iLimit;
    pPrior->iOffset = p->iOffset;
    pPrior->pLimit = sqlite3ExprDup(db, p->pLimit, 0);
    rc = sqlite3Select(pParse, pPrior, &dest);
    sqlite3ExprDelete(db, pPrior->pLimit);
    pPrior->pLimit = 0;
    if (rc) {
      goto multi_select_end;
    }
    p->pPrior = 0;
    p->iLimit = pPrior->iLimit;
    p->iOffset = pPrior->iOffset;
    if (p->iLimit) {
      addr = sqlite3VdbeAddOp1(v, 17, p->iLimit);
      if (p->iOffset) {
        sqlite3VdbeAddOp3(v, 162, p->iLimit, p->iOffset + 1, p->iOffset);
      }
    }
    sqlite3VdbeExplain(pParse, 1, "UNION ALL");
    rc = sqlite3Select(pParse, p, &dest);
    pDelete = p->pPrior;
    p->pPrior = pPrior;
    p->nSelectRow = sqlite3LogEstAdd(p->nSelectRow, pPrior->nSelectRow);
    if (p->pLimit && sqlite3ExprIsInteger(p->pLimit->pLeft, &nLimit, pParse) && nLimit > 0 &&
        p->nSelectRow > sqlite3LogEst((u64)nLimit)) {
      p->nSelectRow = sqlite3LogEst((u64)nLimit);
    }
    if (addr) {
      sqlite3VdbeJumpHere(v, addr);
    }

    if (p->pNext == 0) {
      sqlite3VdbeExplainPop(pParse);
    }
  }

multi_select_end:
  pDest->iSdst = dest.iSdst;
  pDest->nSdst = dest.nSdst;
  pDest->iSDParm2 = dest.iSDParm2;
  if (pDelete) {
    sqlite3ParserAddCleanup(pParse, sqlite3SelectDeleteGeneric, pDelete);
  }
  return rc;
}

void sqlite3SelectWrongNumTermsError(Parse *pParse, Select *p) {
  if (p->selFlags & 0x0000200) {
    sqlite3ErrorMsg(pParse, "all VALUES must have the same number of terms");
  } else {
    sqlite3ErrorMsg(pParse,
                    "SELECTs to the left and right of %s"
                    " do not have the same number of result columns",
                    sqlite3SelectOpName(p->op));
  }
}

int generateOutputSubroutine(Parse *pParse, Select *p, SelectDest *pIn, SelectDest *pDest, int regReturn, int regPrev,
                             KeyInfo *pKeyInfo, int iBreak) {
  Vdbe *v = pParse->pVdbe;
  int iContinue;
  int addr;

  addr = sqlite3VdbeCurrentAddr(v);
  iContinue = sqlite3VdbeMakeLabel(pParse);

  if (regPrev) {
    int addr1, addr2;
    addr1 = sqlite3VdbeAddOp1(v, 17, regPrev);
    addr2 = sqlite3VdbeAddOp4(v, 92, pIn->iSdst, regPrev + 1, pIn->nSdst, (char *)sqlite3KeyInfoRef(pKeyInfo), (-9));
    sqlite3VdbeAddOp3(v, 14, addr2 + 2, iContinue, addr2 + 2);
    sqlite3VdbeJumpHere(v, addr1);
    sqlite3VdbeAddOp3(v, 82, pIn->iSdst, regPrev + 1, pIn->nSdst - 1);
    sqlite3VdbeAddOp2(v, 73, 1, regPrev);
  }
  if (pParse->db->mallocFailed)
    return 0;

  codeOffset(v, p->iOffset, iContinue);

  switch (pDest->eDest) {
    case 6:
    case 3:
    case 12:
    case 10: {
      int r1 = sqlite3GetTempReg(pParse);
      int r2 = sqlite3GetTempReg(pParse);
      int iParm = pDest->iSDParm;
      sqlite3VdbeAddOp3(v, 99, pIn->iSdst, pIn->nSdst, r1);

      if (pDest->eDest == 3) {
        sqlite3VdbeAddOp4Int(v, 140, iParm + 1, r1, pIn->iSdst, pIn->nSdst);
      }

      sqlite3VdbeAddOp2(v, 129, iParm, r2);
      sqlite3VdbeAddOp3(v, 130, iParm, r1, r2);
      sqlite3VdbeChangeP5(v, 0x08);
      sqlite3ReleaseTempReg(pParse, r2);
      sqlite3ReleaseTempReg(pParse, r1);
      break;
    }

    case 1: {
      sqlite3VdbeAddOp2(v, 73, 1, pDest->iSDParm);

      break;
    }

    case 9: {
      int r1;
      r1 = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp4(v, 99, pIn->iSdst, pIn->nSdst, r1, pDest->zAffSdst, pIn->nSdst);
      sqlite3VdbeAddOp4Int(v, 140, pDest->iSDParm, r1, pIn->iSdst, pIn->nSdst);
      if (pDest->iSDParm2 > 0) {
        sqlite3VdbeAddOp4Int(v, 185, pDest->iSDParm2, 0, pIn->iSdst, pIn->nSdst);
        sqlite3VdbeExplain(pParse, 0, "CREATE BLOOM FILTER");
      }
      sqlite3ReleaseTempReg(pParse, r1);
      break;
    }

    case 8: {
      sqlite3ExprCodeMove(pParse, pIn->iSdst, pDest->iSDParm, pIn->nSdst);

      break;
    }

    case 11: {
      if (pDest->iSdst == 0) {
        pDest->iSdst = sqlite3GetTempRange(pParse, pIn->nSdst);
        pDest->nSdst = pIn->nSdst;
      }
      sqlite3ExprCodeMove(pParse, pIn->iSdst, pDest->iSdst, pIn->nSdst);
      sqlite3VdbeAddOp1(v, 12, pDest->iSDParm);
      break;
    }

    case 4:
    case 5: {
      int nKey;
      int r1, r2, r3, ii;
      ExprList *pSO;
      int iParm = pDest->iSDParm;
      pSO = pDest->pOrderBy;

      nKey = pSO->nExpr;
      r1 = sqlite3GetTempReg(pParse);
      r2 = sqlite3GetTempRange(pParse, nKey + 2);
      r3 = r2 + nKey + 1;

      sqlite3VdbeAddOp3(v, 99, pIn->iSdst, pIn->nSdst, r3);
      if (pDest->eDest == 4) {
        sqlite3VdbeAddOp2(v, 140, iParm + 1, r3);
      }
      for (ii = 0; ii < nKey; ii++) {
        sqlite3VdbeAddOp2(v, 83, pIn->iSdst + pSO->a[ii].u.x.iOrderByCol - 1, r2 + ii);
      }
      sqlite3VdbeAddOp2(v, 128, iParm, r2 + nKey);
      sqlite3VdbeAddOp3(v, 99, r2, nKey + 2, r1);
      sqlite3VdbeAddOp4Int(v, 140, iParm, r1, r2, nKey + 2);
      sqlite3ReleaseTempReg(pParse, r1);
      sqlite3ReleaseTempRange(pParse, r2, nKey + 2);
      break;
    }

    case 2: {
      break;
    }

    default: {
      sqlite3VdbeAddOp2(v, 86, pIn->iSdst, pIn->nSdst);
      break;
    }
  }

  if (p->iLimit) {
    sqlite3VdbeAddOp2(v, 63, p->iLimit, iBreak);
  }

  sqlite3VdbeResolveLabel(v, iContinue);
  sqlite3VdbeAddOp1(v, 69, regReturn);

  return addr;
}

int multiSelectByMerge(Parse *pParse, Select *p, SelectDest *pDest) {
  int i, j;
  Select *pPrior;
  Select *pSplit;
  int nSelect;
  Vdbe *v;
  SelectDest destA;
  SelectDest destB;
  int regAddrA;
  int regAddrB;
  int addrSelectA;
  int addrSelectB;
  int regOutA;
  int regOutB;
  int addrOutA;
  int addrOutB = 0;
  int addrEofA;
  int addrEofA_noB;
  int addrEofB;
  int addrAltB;
  int addrAeqB;
  int addrAgtB;
  int regLimitA;
  int regLimitB;
  int regPrev;
  int savedLimit;
  int savedOffset;
  int labelCmpr;
  int labelEnd;
  int addr1;
  int op;
  KeyInfo *pKeyDup = 0;
  KeyInfo *pKeyMerge;
  sqlite3 *db;
  ExprList *pOrderBy;
  int nOrderBy;
  u32 *aPermute;

  db = pParse->db;
  v = pParse->pVdbe;

  labelEnd = sqlite3VdbeMakeLabel(pParse);
  labelCmpr = sqlite3VdbeMakeLabel(pParse);

  op = p->op;

  pOrderBy = p->pOrderBy;

  nOrderBy = pOrderBy->nExpr;

  if (op != 136) {
    for (i = 1; db->mallocFailed == 0 && i <= p->pEList->nExpr; i++) {
      struct ExprList_item *pItem;
      for (j = 0, pItem = pOrderBy->a; j < nOrderBy; j++, pItem++) {
        if (pItem->u.x.iOrderByCol == i)
          break;
      }
      if (j == nOrderBy) {
        Expr *pNew = sqlite3ExprInt32(db, i);
        if (pNew == 0)
          return 7;
        p->pOrderBy = pOrderBy = sqlite3ExprListAppend(pParse, pOrderBy, pNew);
        if (pOrderBy)
          pOrderBy->a[nOrderBy++].u.x.iOrderByCol = (u16)i;
      }
    }
  }

  aPermute = (u32*)(sqlite3DbMallocRawNN(db, sizeof(u32) * (nOrderBy + 1)));
  if (aPermute) {
    struct ExprList_item *pItem;
    int bKeep = 0;
    aPermute[0] = nOrderBy;
    for (i = 1, pItem = pOrderBy->a; i <= nOrderBy; i++, pItem++) {
      aPermute[i] = pItem->u.x.iOrderByCol - 1;
      if (aPermute[i] != (u32)i - 1)
        bKeep = 1;
    }
    if (bKeep == 0) {
      sqlite3DbFreeNN(db, aPermute);
      aPermute = 0;
    }
  }
  pKeyMerge = multiSelectByMergeKeyInfo(pParse, p, 1);

  if (op == 136) {
    regPrev = 0;
  } else {
    int nExpr = p->pEList->nExpr;

    regPrev = pParse->nMem + 1;
    pParse->nMem += nExpr + 1;
    sqlite3VdbeAddOp2(v, 73, 0, regPrev);
    pKeyDup = sqlite3KeyInfoAlloc(db, nExpr, 1);
    if (pKeyDup) {
      for (i = 0; i < nExpr; i++) {
        pKeyDup->aColl[i] = multiSelectCollSeq(pParse, p, i);
        pKeyDup->aSortFlags[i] = 0;
      }
    }
  }

  nSelect = 1;
  if ((op == 136 || op == 135) && (((db)->dbOptFlags & (0x00200000)) == 0)) {
    for (pSplit = p; pSplit->pPrior != 0 && pSplit->op == op; pSplit = pSplit->pPrior) {
      nSelect++;
    }
  }
  if (nSelect <= 3) {
    pSplit = p;
  } else {
    pSplit = p;
    for (i = 2; i < nSelect; i += 2) {
      pSplit = pSplit->pPrior;
    }
  }
  pPrior = pSplit->pPrior;

  pSplit->pPrior = 0;
  pPrior->pNext = 0;

  pPrior->pOrderBy = sqlite3ExprListDup(pParse->db, pOrderBy, 0);
  sqlite3ResolveOrderGroupBy(pParse, p, p->pOrderBy, "ORDER");
  sqlite3ResolveOrderGroupBy(pParse, pPrior, pPrior->pOrderBy, "ORDER");

  computeLimitRegisters(pParse, p, labelEnd);
  if (p->iLimit && op == 136) {
    regLimitA = ++pParse->nMem;
    regLimitB = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, 82, p->iOffset ? p->iOffset + 1 : p->iLimit, regLimitA);
    sqlite3VdbeAddOp2(v, 82, regLimitA, regLimitB);
  } else {
    regLimitA = regLimitB = 0;
  }
  sqlite3ExprDelete(db, p->pLimit);
  p->pLimit = 0;

  regAddrA = ++pParse->nMem;
  regAddrB = ++pParse->nMem;
  regOutA = ++pParse->nMem;
  regOutB = ++pParse->nMem;
  sqlite3SelectDestInit(&destA, 11, regAddrA);
  sqlite3SelectDestInit(&destB, 11, regAddrB);

  sqlite3VdbeExplain(pParse, 1, "MERGE (%s)", sqlite3SelectOpName(p->op));

  addrSelectA = sqlite3VdbeCurrentAddr(v) + 1;
  addr1 = sqlite3VdbeAddOp3(v, 11, regAddrA, 0, addrSelectA);
  pPrior->iLimit = regLimitA;
  sqlite3VdbeExplain(pParse, 1, "LEFT");
  sqlite3Select(pParse, pPrior, &destA);
  sqlite3VdbeEndCoroutine(v, regAddrA);
  sqlite3VdbeJumpHere(v, addr1);

  addrSelectB = sqlite3VdbeCurrentAddr(v) + 1;
  addr1 = sqlite3VdbeAddOp3(v, 11, regAddrB, 0, addrSelectB);
  savedLimit = p->iLimit;
  savedOffset = p->iOffset;
  p->iLimit = regLimitB;
  p->iOffset = 0;
  sqlite3VdbeExplain(pParse, 1, "RIGHT");
  sqlite3Select(pParse, p, &destB);
  p->iLimit = savedLimit;
  p->iOffset = savedOffset;
  sqlite3VdbeEndCoroutine(v, regAddrB);

  addrOutA = generateOutputSubroutine(pParse, p, &destA, pDest, regOutA, regPrev, pKeyDup, labelEnd);

  if (op == 136 || op == 135) {
    addrOutB = generateOutputSubroutine(pParse, p, &destB, pDest, regOutB, regPrev, pKeyDup, labelEnd);
  }
  sqlite3KeyInfoUnref(pKeyDup);

  if (op == 137 || op == 138) {
    addrEofA_noB = addrEofA = labelEnd;
  } else {
    addrEofA = sqlite3VdbeAddOp2(v, 10, regOutB, addrOutB);
    addrEofA_noB = sqlite3VdbeAddOp2(v, 12, regAddrB, labelEnd);
    sqlite3VdbeGoto(v, addrEofA);
    p->nSelectRow = sqlite3LogEstAdd(p->nSelectRow, pPrior->nSelectRow);
  }

  if (op == 138) {
    addrEofB = addrEofA;
    if (p->nSelectRow > pPrior->nSelectRow)
      p->nSelectRow = pPrior->nSelectRow;
  } else {
    addrEofB = sqlite3VdbeAddOp2(v, 10, regOutA, addrOutA);
    sqlite3VdbeAddOp2(v, 12, regAddrA, labelEnd);
    sqlite3VdbeGoto(v, addrEofB);
  }

  addrAltB = sqlite3VdbeAddOp2(v, 10, regOutA, addrOutA);
  sqlite3VdbeAddOp2(v, 12, regAddrA, addrEofA);
  sqlite3VdbeGoto(v, labelCmpr);

  if (op == 136) {
    addrAeqB = addrAltB;
  } else if (op == 138) {
    addrAeqB = addrAltB;
    addrAltB++;
  } else {
    addrAeqB = addrAltB + 1;
  }

  addrAgtB = sqlite3VdbeCurrentAddr(v);
  if (op == 136 || op == 135) {
    sqlite3VdbeAddOp2(v, 10, regOutB, addrOutB);
    sqlite3VdbeAddOp2(v, 12, regAddrB, addrEofB);
    sqlite3VdbeGoto(v, labelCmpr);
  } else {
    addrAgtB++;
  }

  sqlite3VdbeJumpHere(v, addr1);
  sqlite3VdbeAddOp2(v, 12, regAddrA, addrEofA_noB);

  sqlite3VdbeAddOp2(v, 12, regAddrB, addrEofB);

  if (aPermute != 0) {
    sqlite3VdbeAddOp4(v, 91, 0, 0, 0, (char *)aPermute, (-15));
  }
  sqlite3VdbeResolveLabel(v, labelCmpr);
  sqlite3VdbeAddOp4(v, 92, destA.iSdst, destB.iSdst, nOrderBy, (char *)pKeyMerge, (-9));
  if (aPermute != 0) {
    sqlite3VdbeChangeP5(v, 0x01);
  }
  sqlite3VdbeAddOp3(v, 14, addrAltB, addrAeqB, addrAgtB);

  sqlite3VdbeResolveLabel(v, labelEnd);

  if (pSplit->pPrior) {
    sqlite3ParserAddCleanup(pParse, sqlite3SelectDeleteGeneric, pSplit->pPrior);
  }
  pSplit->pPrior = pPrior;
  pPrior->pNext = pSplit;
  sqlite3ExprListDelete(db, pPrior->pOrderBy);
  pPrior->pOrderBy = 0;

  sqlite3VdbeExplainPop(pParse);
  return pParse->nErr != 0;
}

void srclistRenumberCursors(Parse *pParse, int *aCsrMap, SrcList *pSrc, int iExcept) {
  int i;
  SrcItem *pItem;
  for (i = 0, pItem = pSrc->a; i < pSrc->nSrc; i++, pItem++) {
    if (i != iExcept) {
      Select *p;

      if (!pItem->fg.isRecursive || aCsrMap[pItem->iCursor + 1] == 0) {
        aCsrMap[pItem->iCursor + 1] = pParse->nTab++;
      }
      pItem->iCursor = aCsrMap[pItem->iCursor + 1];
      if (pItem->fg.isSubquery) {
        for (p = pItem->u4.pSubq->pSelect; p; p = p->pPrior) {
          srclistRenumberCursors(pParse, aCsrMap, p->pSrc, -1);
        }
      }
    }
  }
}

void renumberCursors(Parse *pParse, Select *p, int iExcept, int *aCsrMap) {
  Walker w;
  srclistRenumberCursors(pParse, aCsrMap, p->pSrc, iExcept);
  memset(&w, 0, sizeof(w));
  w.u.aiCol = aCsrMap;
  w.xExprCallback = renumberCursorsCb;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  sqlite3WalkSelect(&w, p);
}

int flattenSubquery(Parse *pParse, Select *p, int iFrom, int isAgg) {
  const char *zSavedAuthContext = pParse->zAuthContext;
  Select *pParent;
  Select *pSub;
  Select *pSub1;
  SrcList *pSrc;
  SrcList *pSubSrc;
  int iParent;
  int iNewParent = -1;
  int isOuterJoin = 0;
  int i;
  Expr *pWhere;
  SrcItem *pSubitem;
  sqlite3 *db = pParse->db;
  Walker w;
  int *aCsrMap = 0;

  if ((((db)->dbOptFlags & (0x00000001)) != 0))
    return 0;
  pSrc = p->pSrc;

  pSubitem = &pSrc->a[iFrom];
  iParent = pSubitem->iCursor;

  pSub = pSubitem->u4.pSubq->pSelect;

  if (p->pWin || pSub->pWin)
    return 0;

  pSubSrc = pSub->pSrc;

  if (pSub->pLimit && p->pLimit)
    return 0;
  if (pSub->pLimit && pSub->pLimit->pRight)
    return 0;
  if ((p->selFlags & 0x0000100) != 0 && pSub->pLimit) {
    return 0;
  }
  if (pSubSrc->nSrc == 0)
    return 0;
  if (pSub->selFlags & 0x0000001)
    return 0;
  if (pSub->pLimit && (pSrc->nSrc > 1 || isAgg)) {
    return 0;
  }
  if (p->pOrderBy && pSub->pOrderBy) {
    return 0;
  }
  if (isAgg && pSub->pOrderBy)
    return 0;
  if (pSub->pLimit && p->pWhere)
    return 0;
  if (pSub->pLimit && (p->selFlags & 0x0000001) != 0) {
    return 0;
  }
  if (pSub->selFlags & (0x0002000)) {
    return 0;
  }

  if ((pSubitem->fg.jointype & (0x20 | 0x40)) != 0) {
    if (pSubSrc->nSrc > 1 || (p->selFlags & 0x0000001) != 0 || (pSubitem->fg.jointype & 0x10) != 0) {
      return 0;
    }
    isOuterJoin = 1;
  }

  if (iFrom > 0 && (pSubSrc->a[0].fg.jointype & 0x40) != 0) {
    return 0;
  }

  if (pSub->pPrior) {
    int ii;
    if (pSub->pOrderBy) {
      return 0;
    }
    if (isAgg || (p->selFlags & 0x0000001) != 0 || isOuterJoin > 0) {
      return 0;
    }
    for (pSub1 = pSub; pSub1; pSub1 = pSub1->pPrior) {
      if ((pSub1->selFlags & (0x0000001 | 0x0000008)) != 0 || (pSub1->pPrior && pSub1->op != 136) ||
          pSub1->pSrc->nSrc < 1 || pSub1->pWin) {
        return 0;
      }
      if (iFrom > 0 && (pSub1->pSrc->a[0].fg.jointype & 0x40) != 0) {
        return 0;
      };
    }

    if (p->pOrderBy) {
      for (ii = 0; ii < p->pOrderBy->nExpr; ii++) {
        if (p->pOrderBy->a[ii].u.x.iOrderByCol == 0)
          return 0;
      }
    }

    if ((p->selFlags & 0x0002000))
      return 0;

    if (compoundHasDifferentAffinities(pSub))
      return 0;

    if (pSrc->nSrc > 1) {
      if (pParse->nSelect > 500)
        return 0;
      if ((((db)->dbOptFlags & (0x00800000)) != 0))
        return 0;
      aCsrMap = (int*)(sqlite3DbMallocZero(db, ((i64)pParse->nTab + 1) * sizeof(int)));
      if (aCsrMap)
        aCsrMap[0] = pParse->nTab;
    }
  }

  pParse->zAuthContext = pSubitem->zName;
  sqlite3AuthCheck(pParse, SQLITE_SELECT, 0, 0, 0);
  pParse->zAuthContext = zSavedAuthContext;

  if ((pSubitem->fg.isSubquery)) {
    pSub1 = sqlite3SubqueryDetach(db, pSubitem);
  } else {
    pSub1 = 0;
  }

  sqlite3DbFree(db, pSubitem->zName);
  sqlite3DbFree(db, pSubitem->zAlias);
  pSubitem->zName = 0;
  pSubitem->zAlias = 0;

  for (pSub = pSub->pPrior; pSub; pSub = pSub->pPrior) {
    Select *pNew;
    ExprList *pOrderBy = p->pOrderBy;
    Expr *pLimit = p->pLimit;
    Select *pPrior = p->pPrior;
    Table *pItemTab = pSubitem->pSTab;
    pSubitem->pSTab = 0;
    p->pOrderBy = 0;
    p->pPrior = 0;
    p->pLimit = 0;
    pNew = sqlite3SelectDup(db, p, 0);
    p->pLimit = pLimit;
    p->pOrderBy = pOrderBy;
    p->op = 136;
    pSubitem->pSTab = pItemTab;
    if (pNew == 0) {
      p->pPrior = pPrior;
    } else {
      pNew->selId = ++pParse->nSelect;
      if (aCsrMap && (db->mallocFailed == 0)) {
        renumberCursors(pParse, pNew, iFrom, aCsrMap);
      }
      pNew->pPrior = pPrior;
      if (pPrior)
        pPrior->pNext = pNew;
      pNew->pNext = p;
      p->pPrior = pNew;
    }
  }
  sqlite3DbFree(db, aCsrMap);
  if (db->mallocFailed) {
    sqlite3SrcItemAttachSubquery(pParse, pSubitem, pSub1, 0);
    return 1;
  }

  if ((pSubitem->pSTab != 0)) {
    Table *pTabToDel = pSubitem->pSTab;
    if (pTabToDel->nTabRef == 1) {
      Parse *pToplevel = ((pParse)->pToplevel ? (pParse)->pToplevel : (pParse));
      sqlite3ParserAddCleanup(pToplevel, sqlite3DeleteTableGeneric, pTabToDel);
    } else {
      pTabToDel->nTabRef--;
    }
    pSubitem->pSTab = 0;
  }

  pSub = pSub1;
  for (pParent = p; pParent; pParent = pParent->pPrior, pSub = pSub->pPrior) {
    int nSubSrc;
    u8 jointype = pSubitem->fg.jointype;

    pSubSrc = pSub->pSrc;
    nSubSrc = pSubSrc->nSrc;
    pSrc = pParent->pSrc;

    if (nSubSrc > 1) {
      pSrc = sqlite3SrcListEnlarge(pParse, pSrc, nSubSrc - 1, iFrom + 1);
      if (pSrc == 0)
        break;
      pParent->pSrc = pSrc;
      pSubitem = &pSrc->a[iFrom];
    }

    iNewParent = pSubSrc->a[0].iCursor;
    for (i = 0; i < nSubSrc; i++) {
      SrcItem *pItem = &pSrc->a[i + iFrom];

      if (pItem->fg.isUsing)
        sqlite3IdListDelete(db, pItem->u3.pUsing);
      *pItem = pSubSrc->a[i];
      pItem->fg.jointype |= (jointype & 0x40);
      memset(&pSubSrc->a[i], 0, sizeof(pSubSrc->a[i]));
    }
    pSubitem->fg.jointype |= jointype;

    if (pSub->pOrderBy) {
      ExprList *pOrderBy = pSub->pOrderBy;
      for (i = 0; i < pOrderBy->nExpr; i++) {
        pOrderBy->a[i].u.x.iOrderByCol = 0;
      }

      pParent->pOrderBy = pOrderBy;
      pSub->pOrderBy = 0;
    }
    pWhere = pSub->pWhere;
    pSub->pWhere = 0;
    if (isOuterJoin > 0) {
      sqlite3SetJoinExpr(pWhere, iNewParent, 0x000001);
    }
    if (pWhere) {
      if (pParent->pWhere) {
        pParent->pWhere = sqlite3PExpr(pParse, 44, pWhere, pParent->pWhere);
      } else {
        pParent->pWhere = pWhere;
      }
    }
    if (db->mallocFailed == 0) {
      SubstContext x;
      x.pParse = pParse;
      x.iTable = iParent;
      x.iNewTable = iNewParent;
      x.isOuterJoin = isOuterJoin;
      x.nSelDepth = 0;
      x.pEList = pSub->pEList;
      x.pCList = findLeftmostExprlist(pSub);
      substSelect(&x, pParent, 0);
    }

    pParent->selFlags |= pSub->selFlags & 0x0000100;

    if (pSub->pLimit) {
      pParent->pLimit = pSub->pLimit;
      pSub->pLimit = 0;
    }

    for (i = 0; i < nSubSrc; i++) {
      recomputeColumnsUsed(pParent, &pSrc->a[i + iFrom]);
    }
  }

  sqlite3AggInfoPersistWalkerInit(&w, pParse);
  sqlite3WalkSelect(&w, pSub1);
  sqlite3SelectDelete(db, pSub1);

  return 1;
}

int propagateConstants(Parse *pParse, Select *p) {
  WhereConst x;
  Walker w;
  int nChng = 0;
  x.pParse = pParse;
  x.pOomFault = &pParse->db->mallocFailed;
  do {
    x.nConst = 0;
    x.nChng = 0;
    x.apExpr = 0;
    x.bHasAffBlob = 0;
    if ((p->pSrc != 0) && p->pSrc->nSrc > 0 && (p->pSrc->a[0].fg.jointype & 0x40) != 0) {
      x.mExcludeOn = 0x000002 | 0x000001;
    } else {
      x.mExcludeOn = 0x000001;
    }
    findConstInWhere(&x, p->pWhere);
    if (x.nConst) {
      memset(&w, 0, sizeof(w));
      w.pParse = pParse;
      w.xExprCallback = propagateConstantExprRewrite;
      w.xSelectCallback = sqlite3SelectWalkNoop;
      w.xSelectCallback2 = 0;
      w.walkerDepth = 0;
      w.u.pConst = &x;
      sqlite3WalkExpr(&w, p->pWhere);
      sqlite3DbFree(x.pParse->db, x.apExpr);
      nChng += x.nChng;
    }
  } while (x.nChng);
  return nChng;
}

int pushDownWindowCheck(Parse *pParse, Select *pSubq, Expr *pExpr) {
  return sqlite3ExprIsConstantOrGroupBy(pParse, pExpr, pSubq->pWin->pPartition);
}

int pushDownWhereTerms(Parse *pParse, Select *pSubq, Expr *pWhere, SrcList *pSrcList, int iSrc) {
  Expr *pNew;
  SrcItem *pSrc;
  int nChng = 0;
  pSrc = &pSrcList->a[iSrc];
  if (pWhere == 0)
    return 0;
  if (pSubq->selFlags & (0x0002000 | 0x2000000)) {
    return 0;
  }
  if (pSrc->fg.jointype & (0x40 | 0x10)) {
    return 0;
  }

  if (pSubq->pPrior) {
    Select *pSel;
    int notUnionAll = 0;
    for (pSel = pSubq; pSel; pSel = pSel->pPrior) {
      u8 op = pSel->op;

      if (op != 136 && op != 139) {
        notUnionAll = 1;
      }

      if (pSel->pWin)
        return 0;
    }
    if (notUnionAll) {
      for (pSel = pSubq; pSel; pSel = pSel->pPrior) {
        int ii;
        const ExprList *pList = pSel->pEList;

        for (ii = 0; ii < pList->nExpr; ii++) {
          CollSeq *pColl = sqlite3ExprCollSeq(pParse, pList->a[ii].pExpr);
          if (!sqlite3IsBinary(pColl)) {
            return 0;
          }
        }
      }
    }
  } else {
    if (pSubq->pWin && pSubq->pWin->pPartition == 0)
      return 0;
  }

  if (pSubq->pLimit != 0) {
    return 0;
  }
  while (pWhere->op == 44) {
    nChng += pushDownWhereTerms(pParse, pSubq, pWhere->pRight, pSrcList, iSrc);
    pWhere = pWhere->pLeft;
  }

  if (sqlite3ExprIsSingleTableConstraint(pWhere, pSrcList, iSrc, 1)) {
    nChng++;
    pSubq->selFlags |= 0x1000000;
    while (pSubq) {
      SubstContext x;
      pNew = sqlite3ExprDup(pParse->db, pWhere, 0);
      unsetJoinExpr(pNew, -1, 1);
      x.pParse = pParse;
      x.iTable = pSrc->iCursor;
      x.iNewTable = pSrc->iCursor;
      x.isOuterJoin = 0;
      x.nSelDepth = 0;
      x.pEList = pSubq->pEList;
      x.pCList = findLeftmostExprlist(pSubq);
      pNew = substExpr(&x, pNew);

      if (pParse->nErr == 0 && pNew->op == 50 && (((pNew)->flags & 0x001000) != 0)) {
        pNew->x.pSelect->selFlags |= 0x0000020;

        pWhere->x.pSelect->selFlags |= 0x0000020;
      }

      if (pSubq->pWin && 0 == pushDownWindowCheck(pParse, pSubq, pNew)) {
        sqlite3ExprDelete(pParse->db, pNew);
        nChng--;
        break;
      }

      if (pSubq->selFlags & 0x0000008) {
        pSubq->pHaving = sqlite3ExprAnd(pParse, pSubq->pHaving, pNew);
      } else {
        pSubq->pWhere = sqlite3ExprAnd(pParse, pSubq->pWhere, pNew);
      }
      pSubq = pSubq->pPrior;
    }
  }
  return nChng;
}

int sqlite3IndexedByLookup(Parse *pParse, SrcItem *pFrom) {
  Table *pTab = pFrom->pSTab;
  char *zIndexedBy = pFrom->u1.zIndexedBy;
  Index *pIdx;

  for (pIdx = pTab->pIndex; pIdx && sqlite3StrICmp(pIdx->zName, zIndexedBy); pIdx = pIdx->pNext)
    ;
  if (!pIdx) {
    sqlite3ErrorMsg(pParse, "no such index: %s", zIndexedBy, 0);
    pParse->checkSchema = 1;
    return SQLITE_ERROR;
  }

  pFrom->u2.pIBIndex = pIdx;
  return SQLITE_OK;
}

int cannotBeFunction(Parse *pParse, SrcItem *pFrom) {
  if (pFrom->fg.isTabFunc) {
    sqlite3ErrorMsg(pParse, "'%s' is not a function", pFrom->zName);
    return 1;
  }
  return 0;
}

With *sqlite3WithPush(Parse *pParse, With *pWith, u8 bFree) {
  if (pWith) {
    if (bFree) {
      pWith = (With *)sqlite3ParserAddCleanup(pParse, sqlite3WithDeleteGeneric, pWith);
      if (pWith == 0)
        return 0;
    }
    if (pParse->nErr == 0) {
      pWith->pOuter = pParse->pWith;
      pParse->pWith = pWith;
    }
  }
  return pWith;
}

int resolveFromTermToCte(Parse *pParse, Walker *pWalker, SrcItem *pFrom) {
  Cte *pCte;
  With *pWith;

  if (pParse->pWith == 0) {
    return 0;
  }
  if (pParse->nErr) {
    return 0;
  }

  if (pFrom->fg.fixedSchema == 0 && pFrom->u4.zDatabase != 0) {
    return 0;
  }
  if (pFrom->fg.notCte) {
    return 0;
  }
  pCte = searchWith(pParse->pWith, pFrom, &pWith);
  if (pCte) {
    sqlite3 *db = pParse->db;
    Table *pTab;
    ExprList *pEList;
    Select *pSel;
    Select *pLeft;
    Select *pRecTerm;
    int bMayRecursive;
    With *pSavedWith;
    int iRecTab = -1;
    CteUse *pCteUse;

    if (pCte->zCteErr) {
      sqlite3ErrorMsg(pParse, pCte->zCteErr, pCte->zName);
      return 2;
    }
    if (cannotBeFunction(pParse, pFrom))
      return 2;

    pTab = (Table*)(sqlite3DbMallocZero(db, sizeof(Table)));
    if (pTab == 0)
      return 2;
    pCteUse = pCte->pUse;
    if (pCteUse == 0) {
      pCte->pUse = pCteUse = (CteUse*)(sqlite3DbMallocZero(db, sizeof(pCteUse[0])));
      if (pCteUse == 0 || sqlite3ParserAddCleanup(pParse, sqlite3DbFree, pCteUse) == 0) {
        sqlite3DbFree(db, pTab);
        return 2;
      }
      pCteUse->eM10d = pCte->eM10d;
    }
    pFrom->pSTab = pTab;
    pTab->nTabRef = 1;
    pTab->zName = sqlite3DbStrDup(db, pCte->zName);
    pTab->iPKey = -1;
    pTab->nRowLogEst = 200;

    pTab->tabFlags |= 0x00004000 | 0x00000200;
    sqlite3SrcItemAttachSubquery(pParse, pFrom, pCte->pSelect, 1);
    if (db->mallocFailed)
      return 2;

    pSel = pFrom->u4.pSubq->pSelect;

    pSel->selFlags |= 0x4000000;
    if (pFrom->fg.isIndexedBy) {
      sqlite3ErrorMsg(pParse, "no such index: \"%s\"", pFrom->u1.zIndexedBy);
      return 2;
    }

    pFrom->fg.isCte = 1;
    pFrom->u2.pCteUse = pCteUse;
    pCteUse->nUse++;

    pRecTerm = pSel;
    bMayRecursive = (pSel->op == 136 || pSel->op == 135);
    while (bMayRecursive && pRecTerm->op == pSel->op) {
      int i;
      SrcList *pSrc = pRecTerm->pSrc;

      for (i = 0; i < pSrc->nSrc; i++) {
        SrcItem *pItem = &pSrc->a[i];
        if (pItem->zName != 0 && !pItem->fg.hadSchema && (!pItem->fg.isSubquery) &&
            (pItem->fg.fixedSchema || pItem->u4.zDatabase == 0) && 0 == sqlite3StrICmp(pItem->zName, pCte->zName)) {
          pItem->pSTab = pTab;
          pTab->nTabRef++;
          pItem->fg.isRecursive = 1;
          if (pRecTerm->selFlags & 0x0002000) {
            sqlite3ErrorMsg(pParse, "multiple references to recursive table: %s", pCte->zName);
            return 2;
          }
          pRecTerm->selFlags |= 0x0002000;
          if (iRecTab < 0)
            iRecTab = pParse->nTab++;
          pItem->iCursor = iRecTab;
        }
      }
      if ((pRecTerm->selFlags & 0x0002000) == 0)
        break;
      pRecTerm = pRecTerm->pPrior;
    }

    pCte->zCteErr = "circular reference: %s";
    pSavedWith = pParse->pWith;
    pParse->pWith = pWith;
    if (pSel->selFlags & 0x0002000) {
      int rc;

      pRecTerm->pWith = pSel->pWith;
      rc = sqlite3WalkSelect(pWalker, pRecTerm);
      pRecTerm->pWith = 0;
      if (rc) {
        pParse->pWith = pSavedWith;
        return 2;
      }
    } else {
      if (sqlite3WalkSelect(pWalker, pSel)) {
        pParse->pWith = pSavedWith;
        return 2;
      }
    }
    pParse->pWith = pWith;

    for (pLeft = pSel; pLeft->pPrior; pLeft = pLeft->pPrior)
      ;
    pEList = pLeft->pEList;
    if (pCte->pCols) {
      if (pEList && pEList->nExpr != pCte->pCols->nExpr) {
        sqlite3ErrorMsg(pParse, "table %s has %d values for %d columns", pCte->zName, pEList->nExpr,
                        pCte->pCols->nExpr);
        pParse->pWith = pSavedWith;
        return 2;
      }
      pEList = pCte->pCols;
    }

    sqlite3ColumnsFromExprList(pParse, pEList, &pTab->nCol, &pTab->aCol);
    if (bMayRecursive) {
      if (pSel->selFlags & 0x0002000) {
        pCte->zCteErr = "multiple recursive references: %s";
      } else {
        pCte->zCteErr = "recursive reference in a subquery: %s";
      }
      sqlite3WalkSelect(pWalker, pSel);
    }
    pCte->zCteErr = 0;
    pParse->pWith = pSavedWith;
    return 1;
  }
  return 0;
}

int sqlite3ExpandSubquery(Parse *pParse, SrcItem *pFrom) {
  Select *pSel;
  Table *pTab;

  pSel = pFrom->u4.pSubq->pSelect;

  pFrom->pSTab = pTab = (Table*)(sqlite3DbMallocZero(pParse->db, sizeof(Table)));
  if (pTab == 0)
    return SQLITE_NOMEM;
  pTab->nTabRef = 1;
  if (pFrom->zAlias) {
    pTab->zName = sqlite3DbStrDup(pParse->db, pFrom->zAlias);
  } else {
    pTab->zName = sqlite3MPrintf(pParse->db, "%!S", pFrom);
  }
  while (pSel->pPrior) {
    pSel = pSel->pPrior;
  }
  sqlite3ColumnsFromExprList(pParse, pSel->pEList, &pTab->nCol, &pTab->aCol);
  pTab->iPKey = -1;
  pTab->eTabType = 2;
  pTab->nRowLogEst = 200;

  pTab->tabFlags |= 0x00004000 | 0x00000200;

  return pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
}

void sqlite3SelectExpand(Parse *pParse, Select *pSelect) {
  Walker w;
  w.xExprCallback = sqlite3ExprWalkNoop;
  w.pParse = pParse;
  if ((pParse->hasCompound)) {
    w.xSelectCallback = convertCompoundSelectToSubquery;
    w.xSelectCallback2 = 0;
    sqlite3WalkSelect(&w, pSelect);
  }
  w.xSelectCallback = selectExpander;
  w.xSelectCallback2 = sqlite3SelectPopWith;
  w.eCode = 0;
  sqlite3WalkSelect(&w, pSelect);
}

void sqlite3SelectAddTypeInfo(Parse *pParse, Select *pSelect) {
  Walker w;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.xSelectCallback2 = selectAddSubqueryTypeInfo;
  w.xExprCallback = sqlite3ExprWalkNoop;
  w.pParse = pParse;
  sqlite3WalkSelect(&w, pSelect);
}

void sqlite3SelectPrep(Parse *pParse, Select *p, NameContext *pOuterNC) {
  if (pParse->db->mallocFailed)
    return;
  if (p->selFlags & 0x0000080)
    return;
  sqlite3SelectExpand(pParse, p);
  if (pParse->nErr)
    return;
  sqlite3ResolveSelectNames(pParse, p, pOuterNC);
  if (pParse->nErr)
    return;
  sqlite3SelectAddTypeInfo(pParse, p);
}

void optimizeAggregateUseOfIndexedExpr(Parse *pParse, Select *pSelect, AggInfo *pAggInfo, NameContext *pNC) {
  pAggInfo->nColumn = pAggInfo->nAccumulator;
  if ((pAggInfo->nSortingColumn > 0)) {
    int mx = pSelect->pGroupBy->nExpr - 1;
    int j, k;
    for (j = 0; j < pAggInfo->nColumn; j++) {
      k = pAggInfo->aCol[j].iSorterColumn;
      if (k > mx)
        mx = k;
    }
    pAggInfo->nSortingColumn = mx + 1;
  }
  analyzeAggFuncArgs(pAggInfo, pNC);

  (void)(pSelect);
  (void)(pParse);
}

void assignAggregateRegisters(Parse *pParse, AggInfo *pAggInfo) {
  pAggInfo->iFirstReg = pParse->nMem + 1;
  pParse->nMem += pAggInfo->nColumn + pAggInfo->nFunc;
}

void resetAccumulator(Parse *pParse, AggInfo *pAggInfo) {
  Vdbe *v = pParse->pVdbe;
  int i;
  struct AggInfo_func *pFunc;
  int nReg = pAggInfo->nFunc + pAggInfo->nColumn;

  if (nReg == 0)
    return;
  if (pParse->nErr)
    return;
  sqlite3VdbeAddOp3(v, 77, 0, pAggInfo->iFirstReg, pAggInfo->iFirstReg + nReg - 1);
  for (pFunc = pAggInfo->aFunc, i = 0; i < pAggInfo->nFunc; i++, pFunc++) {
    if (pFunc->iDistinct >= 0) {
      Expr *pE = pFunc->pFExpr;

      if (pE->x.pList == 0 || pE->x.pList->nExpr != 1) {
        sqlite3ErrorMsg(pParse,
                        "DISTINCT aggregates must have exactly one "
                        "argument");
        pFunc->iDistinct = -1;
      } else {
        KeyInfo *pKeyInfo = sqlite3KeyInfoFromExprList(pParse, pE->x.pList, 0, 0);
        pFunc->iDistAddr = sqlite3VdbeAddOp4(v, 120, pFunc->iDistinct, 0, 0, (char *)pKeyInfo, (-9));
        sqlite3VdbeExplain(pParse, 0, "USE TEMP B-TREE FOR %s(DISTINCT)", pFunc->pFunc->zName);
      }
    }
    if (pFunc->iOBTab >= 0) {
      ExprList *pOBList;
      KeyInfo *pKeyInfo;
      int nExtra = 0;

      pOBList = pFunc->pFExpr->pLeft->x.pList;
      if (!pFunc->bOBUnique) {
        nExtra++;
      }
      if (pFunc->bOBPayload) {
        nExtra += pFunc->pFExpr->x.pList->nExpr;
      }
      if (pFunc->bUseSubtype) {
        nExtra += pFunc->pFExpr->x.pList->nExpr;
      }
      pKeyInfo = sqlite3KeyInfoFromExprList(pParse, pOBList, 0, nExtra);
      if (!pFunc->bOBUnique && pParse->nErr == 0) {
        pKeyInfo->nKeyField++;
      }
      sqlite3VdbeAddOp4(v, 120, pFunc->iOBTab, pOBList->nExpr + nExtra, 0, (char *)pKeyInfo, (-9));
      sqlite3VdbeExplain(pParse, 0, "USE TEMP B-TREE FOR %s(ORDER BY)", pFunc->pFunc->zName);
    }
  }
}

void finalizeAggFunctions(Parse *pParse, AggInfo *pAggInfo) {
  Vdbe *v = pParse->pVdbe;
  int i;
  struct AggInfo_func *pF;
  for (i = 0, pF = pAggInfo->aFunc; i < pAggInfo->nFunc; i++, pF++) {
    ExprList *pList;

    if (pParse->nErr)
      return;
    pList = pF->pFExpr->x.pList;
    if (pF->iOBTab >= 0) {
      int iTop;
      int nArg;
      int nKey;
      int regAgg;
      int j;

      nArg = pList->nExpr;
      regAgg = sqlite3GetTempRange(pParse, nArg);

      if (pF->bOBPayload == 0) {
        nKey = 0;
      } else {
        nKey = pF->pFExpr->pLeft->x.pList->nExpr;
        if ((!pF->bOBUnique))
          nKey++;
      }
      iTop = sqlite3VdbeAddOp1(v, 36, pF->iOBTab);
      for (j = nArg - 1; j >= 0; j--) {
        sqlite3VdbeAddOp3(v, 96, pF->iOBTab, nKey + j, regAgg + j);
      }
      if (pF->bUseSubtype) {
        int regSubtype = sqlite3GetTempReg(pParse);
        int iBaseCol = nKey + nArg + (pF->bOBPayload == 0 && pF->bOBUnique == 0);
        for (j = nArg - 1; j >= 0; j--) {
          sqlite3VdbeAddOp3(v, 96, pF->iOBTab, iBaseCol + j, regSubtype);
          sqlite3VdbeAddOp2(v, 184, regSubtype, regAgg + j);
        }
        sqlite3ReleaseTempReg(pParse, regSubtype);
      }
      sqlite3VdbeAddOp3(v, 164, 0, regAgg, ((pAggInfo)->iFirstReg + (pAggInfo)->nColumn + (i)));
      sqlite3VdbeAppendP4(v, pF->pFunc, (-8));
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3VdbeAddOp2(v, 40, pF->iOBTab, iTop + 1);
      sqlite3VdbeJumpHere(v, iTop);
      sqlite3ReleaseTempRange(pParse, regAgg, nArg);
    }
    sqlite3VdbeAddOp2(v, 167, ((pAggInfo)->iFirstReg + (pAggInfo)->nColumn + (i)), pList ? pList->nExpr : 0);
    sqlite3VdbeAppendP4(v, pF->pFunc, (-8));
  }
}

void updateAccumulator(Parse *pParse, int regAcc, AggInfo *pAggInfo, int eDistinctType) {
  Vdbe *v = pParse->pVdbe;
  int i;
  int regHit = 0;
  int addrHitTest = 0;
  struct AggInfo_func *pF;
  struct AggInfo_col *pC;

  if (pParse->nErr)
    return;
  pAggInfo->directMode = 1;
  for (i = 0, pF = pAggInfo->aFunc; i < pAggInfo->nFunc; i++, pF++) {
    int nArg;
    int addrNext = 0;
    int regAgg;
    int regAggSz = 0;
    int regDistinct = 0;
    ExprList *pList;

    pList = pF->pFExpr->x.pList;
    if ((((pF->pFExpr)->flags & (u32)(0x1000000)) != 0)) {
      Expr *pFilter = pF->pFExpr->y.pWin->pFilter;
      if (pAggInfo->nAccumulator && (pF->pFunc->funcFlags & 0x0020) && regAcc) {
        if (regHit == 0)
          regHit = ++pParse->nMem;

        sqlite3VdbeAddOp2(v, 82, regAcc, regHit);
      }
      addrNext = sqlite3VdbeMakeLabel(pParse);
      sqlite3ExprIfFalse(pParse, pFilter, addrNext, 0x10);
    }
    if (pF->iOBTab >= 0) {
      int jj;
      ExprList *pOBList;

      nArg = pList->nExpr;

      pOBList = pF->pFExpr->pLeft->x.pList;

      regAggSz = pOBList->nExpr;
      if (!pF->bOBUnique) {
        regAggSz++;
      }
      if (pF->bOBPayload) {
        regAggSz += nArg;
      }
      if (pF->bUseSubtype) {
        regAggSz += nArg;
      }
      regAggSz++;
      regAgg = sqlite3GetTempRange(pParse, regAggSz);
      regDistinct = regAgg;
      sqlite3ExprCodeExprList(pParse, pOBList, regAgg, 0, 0x01);
      jj = pOBList->nExpr;
      if (!pF->bOBUnique) {
        sqlite3VdbeAddOp2(v, 128, pF->iOBTab, regAgg + jj);
        jj++;
      }
      if (pF->bOBPayload) {
        regDistinct = regAgg + jj;
        sqlite3ExprCodeExprList(pParse, pList, regDistinct, 0, 0x01);
        jj += nArg;
      }
      if (pF->bUseSubtype) {
        int kk;
        int regBase = pF->bOBPayload ? regDistinct : regAgg;
        for (kk = 0; kk < nArg; kk++, jj++) {
          sqlite3VdbeAddOp2(v, 183, regBase + kk, regAgg + jj);
        }
      }
    } else if (pList) {
      nArg = pList->nExpr;
      regAgg = sqlite3GetTempRange(pParse, nArg);
      regDistinct = regAgg;
      sqlite3ExprCodeExprList(pParse, pList, regAgg, 0, 0x01);
    } else {
      nArg = 0;
      regAgg = 0;
    }
    if (pF->iDistinct >= 0 && pList) {
      if (addrNext == 0) {
        addrNext = sqlite3VdbeMakeLabel(pParse);
      }
      pF->iDistinct = codeDistinct(pParse, eDistinctType, pF->iDistinct, addrNext, pList, regDistinct);
    }
    if (pF->iOBTab >= 0) {
      sqlite3VdbeAddOp3(v, 99, regAgg, regAggSz - 1, regAgg + regAggSz - 1);
      sqlite3VdbeAddOp4Int(v, 140, pF->iOBTab, regAgg + regAggSz - 1, regAgg, regAggSz - 1);
      sqlite3ReleaseTempRange(pParse, regAgg, regAggSz);
    } else {
      if (pF->pFunc->funcFlags & 0x0020) {
        CollSeq *pColl = 0;
        struct ExprList_item *pItem;
        int j;

        for (j = 0, pItem = pList->a; !pColl && j < nArg; j++, pItem++) {
          pColl = sqlite3ExprCollSeq(pParse, pItem->pExpr);
        }
        if (!pColl) {
          pColl = pParse->db->pDfltColl;
        }
        if (regHit == 0 && pAggInfo->nAccumulator)
          regHit = ++pParse->nMem;
        sqlite3VdbeAddOp4(v, 87, regHit, 0, 0, (char *)pColl, (-2));
      }
      sqlite3VdbeAddOp3(v, 164, 0, regAgg, ((pAggInfo)->iFirstReg + (pAggInfo)->nColumn + (i)));
      sqlite3VdbeAppendP4(v, pF->pFunc, (-8));
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3ReleaseTempRange(pParse, regAgg, nArg);
    }
    if (addrNext) {
      sqlite3VdbeResolveLabel(v, addrNext);
    }
    if (pParse->nErr)
      return;
  }
  if (regHit == 0 && pAggInfo->nAccumulator) {
    regHit = regAcc;
  }
  if (regHit) {
    addrHitTest = sqlite3VdbeAddOp1(v, 16, regHit);
  }
  for (i = 0, pC = pAggInfo->aCol; i < pAggInfo->nAccumulator; i++, pC++) {
    sqlite3ExprCode(pParse, pC->pCExpr, ((pAggInfo)->iFirstReg + (i)));
    if (pParse->nErr)
      return;
  }

  pAggInfo->directMode = 0;
  if (addrHitTest) {
    sqlite3VdbeJumpHereOrPopInst(v, addrHitTest);
  }
}

void explainSimpleCount(Parse *pParse, Table *pTab, Index *pIdx) {
  if (pParse->explain == 2) {
    int bCover = (pIdx != 0 && ((((pTab)->tabFlags & 0x00000080) == 0) || !((pIdx)->idxType == 2)));
    sqlite3VdbeExplain(pParse, 0, "SCAN %s%s%s", pTab->zName, bCover ? " USING COVERING INDEX " : "",
                       bCover ? pIdx->zName : "");
  }
}

void havingToWhere(Parse *pParse, Select *p) {
  Walker sWalker;
  memset(&sWalker, 0, sizeof(sWalker));
  sWalker.pParse = pParse;
  sWalker.xExprCallback = havingToWhereExprCb;
  sWalker.u.pSelect = p;
  sqlite3WalkExpr(&sWalker, p->pHaving);
}

int countOfViewOptimization(Parse *pParse, Select *p) {
  Select *pSub, *pPrior;
  Expr *pExpr;
  Expr *pCount;
  sqlite3 *db;
  SrcItem *pFrom;
  if ((p->selFlags & 0x0000008) == 0)
    return 0;
  if (p->pEList->nExpr != 1)
    return 0;
  if (p->pWhere)
    return 0;
  if (p->pHaving)
    return 0;
  if (p->pGroupBy)
    return 0;
  if (p->pOrderBy)
    return 0;
  pExpr = p->pEList->a[0].pExpr;
  if (pExpr->op != 169)
    return 0;

  if (sqlite3_stricmp(pExpr->u.zToken, "count"))
    return 0;

  if (pExpr->x.pList != 0)
    return 0;
  if (p->pSrc->nSrc != 1)
    return 0;
  if ((((pExpr)->flags & (u32)(0x1000000)) != 0))
    return 0;
  pFrom = p->pSrc->a;
  if (pFrom->fg.isSubquery == 0)
    return 0;
  pSub = pFrom->u4.pSubq->pSelect;
  if (pSub->pPrior == 0)
    return 0;
  if (pSub->selFlags & 0x4000000)
    return 0;
  do {
    if (pSub->op != 136 && pSub->pPrior)
      return 0;
    if (pSub->pWhere)
      return 0;
    if (pSub->pLimit)
      return 0;
    if (pSub->selFlags & (0x0000008 | 0x0000001)) {
      return 0;
    }

    pSub = pSub->pPrior;
  } while (pSub);

  db = pParse->db;
  pCount = pExpr;
  pExpr = 0;
  pSub = sqlite3SubqueryDetach(db, pFrom);
  sqlite3SrcListDelete(db, p->pSrc);
  p->pSrc = (SrcList*)(sqlite3DbMallocZero(pParse->db, (offsetof(SrcList, a) + sizeof(SrcItem))));
  while (pSub) {
    Expr *pTerm;
    pPrior = pSub->pPrior;
    pSub->pPrior = 0;
    pSub->pNext = 0;
    pSub->selFlags |= 0x0000008;
    pSub->selFlags &= ~(u32)0x0000100;
    pSub->nSelectRow = 0;
    sqlite3ParserAddCleanup(pParse, sqlite3ExprListDeleteGeneric, pSub->pEList);
    pTerm = pPrior ? sqlite3ExprDup(db, pCount, 0) : pCount;
    pSub->pEList = sqlite3ExprListAppend(pParse, 0, pTerm);
    pTerm = sqlite3PExpr(pParse, 139, 0, 0);
    sqlite3PExprAddSelect(pParse, pTerm, pSub);
    if (pExpr == 0) {
      pExpr = pTerm;
    } else {
      pExpr = sqlite3PExpr(pParse, 107, pTerm, pExpr);
    }
    pSub = pPrior;
  }
  p->pEList->a[0].pExpr = pExpr;
  p->selFlags &= ~(u32)0x0000008;

  return 1;
}

int fromClauseTermCanBeCoroutine(Parse *pParse, SrcList *pTabList, int i, int selFlags) {
  SrcItem *pItem = &pTabList->a[i];
  if (pItem->fg.isCte) {
    const CteUse *pCteUse = pItem->u2.pCteUse;
    if (pCteUse->eM10d == 0)
      return 0;
    if (pCteUse->nUse >= 2 && pCteUse->eM10d != 2)
      return 0;
  }
  if (pTabList->a[0].fg.jointype & 0x40)
    return 0;
  if ((((pParse->db)->dbOptFlags & (0x02000000)) != 0))
    return 0;
  if (isSelfJoinView(pTabList, pItem, i + 1, pTabList->nSrc) != 0) {
    return 0;
  }
  if (i == 0) {
    if (pTabList->nSrc == 1)
      return 1;
    if (pTabList->a[1].fg.jointype & 0x02)
      return 1;
    if (selFlags & 0x10000000)
      return 0;
    return 1;
  }
  if (selFlags & 0x10000000)
    return 0;
  while (1) {
    if (pItem->fg.jointype & (0x20 | 0x02))
      return 0;
    if (i == 0)
      break;
    i--;
    pItem--;
    if (pItem->fg.isSubquery)
      return 0;
  }
  return 1;
}

__attribute__((noinline)) void existsToJoin(Parse *pParse, Select *p, Expr *pWhere) {
  if (pParse->nErr == 0 && pWhere != 0 && !(((pWhere)->flags & (u32)(0x000001 | 0x000002)) != 0) && (p->pSrc != 0) &&
      p->pSrc->nSrc < ((int)(sizeof(Bitmask) * 8)) && (p->pLimit == 0 || p->pLimit->pRight == 0)) {
    if (pWhere->op == 44) {
      Expr *pRight = pWhere->pRight;
      existsToJoin(pParse, p, pWhere->pLeft);
      existsToJoin(pParse, p, pRight);
    } else if (pWhere->op == 20) {
      Select *pSub = pWhere->x.pSelect;
      Expr *pSubWhere = pSub->pWhere;
      if (pSub->pSrc->nSrc == 1 && (pSub->selFlags & 0x0000008) == 0 && !pSub->pSrc->a[0].fg.isSubquery &&
          pSub->pLimit == 0 && pSub->pPrior == 0) {
        sqlite3 *db = pParse->db;
        int *aCsrMap = (int*)(sqlite3DbMallocZero(db, (pParse->nTab + 2) * sizeof(int)));
        if (aCsrMap == 0)
          return;
        aCsrMap[0] = (pParse->nTab + 1);
        renumberCursors(pParse, pSub, -1, aCsrMap);
        sqlite3DbFree(db, aCsrMap);

        memset(pWhere, 0, sizeof(*pWhere));
        pWhere->op = 156;
        pWhere->u.iValue = 1;
        (pWhere)->flags |= (u32)(0x000800);

        pSub->pSrc->a[0].fg.fromExists = 1;
        p->pSrc = sqlite3SrcListAppendList(pParse, p->pSrc, pSub->pSrc);
        if (pSubWhere) {
          p->pWhere = sqlite3PExpr(pParse, 44, p->pWhere, pSubWhere);
          pSub->pWhere = 0;
        }
        pSub->pSrc = 0;
        sqlite3ParserAddCleanup(pParse, sqlite3SelectDeleteGeneric, pSub);
      }
    }
  }
}

void sqlite3SelectCheckOnClauses(Parse *pParse, Select *pSelect) {
  Walker w;
  CheckOnCtx sCtx;
  int ii;

  memset(&w, 0, sizeof(w));
  w.pParse = pParse;
  w.xExprCallback = selectCheckOnClausesExpr;
  w.xSelectCallback = selectCheckOnClausesSelect;
  w.u.pCheckOnCtx = &sCtx;
  memset(&sCtx, 0, sizeof(sCtx));
  sCtx.pSrc = pSelect->pSrc;
  sqlite3WalkExpr(&w, pSelect->pWhere);
  pSelect->selFlags &= ~0x40000000;

  sCtx.bFuncArg = 1;
  for (ii = 0; ii < pSelect->pSrc->nSrc; ii++) {
    SrcItem *pItem = &pSelect->pSrc->a[ii];
    if (pItem->fg.isTabFunc && (pItem->fg.jointype & 0x20)) {
      sCtx.iJoin = pItem->iCursor;
      sqlite3WalkExprList(&w, pItem->u1.pFuncArg);
    }
  }
}

int sqlite3Select(Parse *pParse, Select *p, SelectDest *pDest) {
  int i, j;
  WhereInfo *pWInfo;
  Vdbe *v;
  int isAgg;
  ExprList *pEList = 0;
  SrcList *pTabList;
  Expr *pWhere;
  ExprList *pGroupBy;
  Expr *pHaving;
  AggInfo *pAggInfo = 0;
  int rc = 1;
  DistinctCtx sDistinct;
  SortCtx sSort;
  int iEnd;
  sqlite3 *db;
  ExprList *pMinMaxOrderBy = 0;
  u8 minMaxFlag;

  db = pParse->db;

  v = sqlite3GetVdbe(pParse);
  if (p == 0 || pParse->nErr) {
    return 1;
  }

  if (sqlite3AuthCheck(pParse, SQLITE_SELECT, 0, 0, 0))
    return 1;

  if (((pDest->eDest) <= 4)) {
    if (p->pOrderBy) {
      sqlite3ParserAddCleanup(pParse, sqlite3ExprListDeleteGeneric, p->pOrderBy);
      p->pOrderBy = 0;
    }
    p->selFlags &= ~(u32)0x0000001;
  }
  sqlite3SelectPrep(pParse, p, 0);
  if (pParse->nErr) {
    goto select_end;
  }

  if (p->selFlags & 0x0800000) {
    SrcItem *p0 = &p->pSrc->a[0];
    if (sameSrcAlias(p0, p->pSrc)) {
      sqlite3ErrorMsg(pParse, "target object/alias may not appear in FROM clause: %s",
                      p0->zAlias ? p0->zAlias : p0->pSTab->zName);
      goto select_end;
    }

    p->selFlags &= ~(u32)0x0800000;
  }

  if (pDest->eDest == 7) {
    sqlite3GenerateColumnNames(pParse, p);
  }

  if (sqlite3WindowRewrite(pParse, p)) {
    goto select_end;
  }

  pTabList = p->pSrc;
  isAgg = (p->selFlags & 0x0000008) != 0;
  memset(&sSort, 0, sizeof(sSort));
  sSort.pOrderBy = p->pOrderBy;

  for (i = 0; !p->pPrior && i < pTabList->nSrc; i++) {
    SrcItem *pItem = &pTabList->a[i];
    Select *pSub = pItem->fg.isSubquery ? pItem->u4.pSubq->pSelect : 0;
    Table *pTab = pItem->pSTab;

    if ((pItem->fg.jointype & (0x08 | 0x40)) != 0 &&
        sqlite3ExprImpliesNonNullRow(p->pWhere, pItem->iCursor, pItem->fg.jointype & 0x40) &&
        (((db)->dbOptFlags & (0x00002000)) == 0)) {
      if (pItem->fg.jointype & 0x08) {
        if (pItem->fg.jointype & 0x10) {
          pItem->fg.jointype &= ~0x08;
        } else {
          pItem->fg.jointype &= ~(0x08 | 0x20);
          unsetJoinExpr(p->pWhere, pItem->iCursor, 0);
        }
      }
      if (pItem->fg.jointype & 0x40) {
        for (j = i + 1; j < pTabList->nSrc; j++) {
          SrcItem *pI2 = &pTabList->a[j];
          if (pI2->fg.jointype & 0x10) {
            if (pI2->fg.jointype & 0x08) {
              pI2->fg.jointype &= ~0x10;
            } else {
              pI2->fg.jointype &= ~(0x10 | 0x20);
              unsetJoinExpr(p->pWhere, pI2->iCursor, 1);
            }
          }
        }
        for (j = pTabList->nSrc - 1; j >= 0; j--) {
          pTabList->a[j].fg.jointype &= ~0x40;
          if (pTabList->a[j].fg.jointype & 0x10)
            break;
        }
      }
    }

    if (pSub == 0)
      continue;

    if (pTab->nCol != pSub->pEList->nExpr) {
      sqlite3ErrorMsg(pParse, "expected %d columns for '%s' but got %d", pTab->nCol, pTab->zName, pSub->pEList->nExpr);
      goto select_end;
    }

    if (pItem->fg.isCte && pItem->u2.pCteUse->eM10d == 0) {
      continue;
    }

    if ((pSub->selFlags & 0x0000008) != 0)
      continue;

    if (pSub->pOrderBy != 0 && (p->pOrderBy != 0 || pTabList->nSrc > 1) && pSub->pLimit == 0 &&
        (pSub->selFlags & (0x8000000 | 0x0002000)) == 0 && (p->selFlags & 0x8000000) == 0 &&
        (((db)->dbOptFlags & (0x00040000)) == 0)) {
      sqlite3ParserAddCleanup(pParse, sqlite3ExprListDeleteGeneric, pSub->pOrderBy);
      pSub->pOrderBy = 0;
    }

    if (pSub->pOrderBy != 0 && i == 0 && (p->selFlags & 0x0040000) != 0 &&
        (pTabList->nSrc == 1 || (pTabList->a[1].fg.jointype & (0x20 | 0x02)) != 0)) {
      continue;
    }

    if (flattenSubquery(pParse, p, i, isAgg)) {
      if (pParse->nErr)
        goto select_end;

      i = -1;
    }
    pTabList = p->pSrc;
    if (db->mallocFailed)
      goto select_end;
    if (!((pDest->eDest) <= 6)) {
      sSort.pOrderBy = p->pOrderBy;
    }
  }

  if (p->pPrior) {
    rc = multiSelect(pParse, p, pDest);

    if (p->pNext == 0)
      sqlite3VdbeExplainPop(pParse);
    return rc;
  }

  if (pParse->bHasExists && (((db)->dbOptFlags & (0x40000000)) == 0)) {
    existsToJoin(pParse, p, p->pWhere);
    pTabList = p->pSrc;
  }

  if (p->pWhere != 0 && p->pWhere->op == 44 && (((db)->dbOptFlags & (0x00008000)) == 0) &&
      propagateConstants(pParse, p)) {
  } else {
  }

  if ((((db)->dbOptFlags & (0x00000001 | 0x00000200)) == 0) && countOfViewOptimization(pParse, p)) {
    if (db->mallocFailed)
      goto select_end;
    pTabList = p->pSrc;
  }

  for (i = 0; i < pTabList->nSrc; i++) {
    SrcItem *pItem = &pTabList->a[i];
    SrcItem *pPrior;
    SelectDest dest;
    Subquery *pSubq;
    Select *pSub;

    const char *zSavedAuthContext;

    if (pItem->colUsed == 0 && pItem->zName != 0) {
      const char *zDb;
      if (pItem->fg.fixedSchema) {
        int iDb = sqlite3SchemaToIndex(pParse->db, pItem->u4.pSchema);
        zDb = db->aDb[iDb].zDbSName;
      } else if (pItem->fg.isSubquery) {
        zDb = 0;
      } else {
        zDb = pItem->u4.zDatabase;
      }
      sqlite3AuthCheck(pParse, SQLITE_READ, pItem->zName, "", zDb);
    }

    if (pItem->fg.isSubquery == 0)
      continue;
    pSubq = pItem->u4.pSubq;

    pSub = pSubq->pSelect;

    if (pSubq->addrFillSub != 0)
      continue;

    pParse->nHeight += sqlite3SelectExprHeight(p);

    if ((((db)->dbOptFlags & (0x00001000)) == 0) &&
        (pItem->fg.isCte == 0 || (pItem->u2.pCteUse->eM10d != 0 && pItem->u2.pCteUse->nUse < 2)) &&
        pushDownWhereTerms(pParse, pSub, p->pWhere, pTabList, i)) {
    } else {
    }

    if ((((db)->dbOptFlags & (0x04000000)) == 0) && disableUnusedSubqueryResultColumns(pItem)) {
    }

    zSavedAuthContext = pParse->zAuthContext;
    pParse->zAuthContext = pItem->zName;

    if (fromClauseTermCanBeCoroutine(pParse, pTabList, i, p->selFlags)) {
      int addrTop = sqlite3VdbeCurrentAddr(v) + 1;

      pSubq->regReturn = ++pParse->nMem;
      sqlite3VdbeAddOp3(v, 11, pSubq->regReturn, 0, addrTop);
      pSubq->addrFillSub = addrTop;
      sqlite3SelectDestInit(&dest, 11, pSubq->regReturn);
      sqlite3VdbeExplain(pParse, 1, "CO-ROUTINE %!S", pItem);
      sqlite3Select(pParse, pSub, &dest);
      pItem->pSTab->nRowLogEst = pSub->nSelectRow;
      pItem->fg.viaCoroutine = 1;
      pSubq->regResult = dest.iSdst;
      sqlite3VdbeEndCoroutine(v, pSubq->regReturn);
      sqlite3VdbeJumpHere(v, addrTop - 1);
      sqlite3ClearTempRegCache(pParse);
    } else if (pItem->fg.isCte && pItem->u2.pCteUse->addrM9e > 0) {
      CteUse *pCteUse = pItem->u2.pCteUse;
      sqlite3VdbeAddOp2(v, 10, pCteUse->regRtn, pCteUse->addrM9e);
      if (pItem->iCursor != pCteUse->iCur) {
        sqlite3VdbeAddOp2(v, 117, pItem->iCursor, pCteUse->iCur);
      }
      pSub->nSelectRow = pCteUse->nRowEst;
    } else if ((pPrior = isSelfJoinView(pTabList, pItem, 0, i)) != 0) {
      Subquery *pPriorSubq;

      pPriorSubq = pPrior->u4.pSubq;

      if (pPriorSubq->addrFillSub) {
        sqlite3VdbeAddOp2(v, 10, pPriorSubq->regReturn, pPriorSubq->addrFillSub);
      }
      sqlite3VdbeAddOp2(v, 117, pItem->iCursor, pPrior->iCursor);
      pSub->nSelectRow = pPriorSubq->pSelect->nSelectRow;
    } else {
      int topAddr;
      int onceAddr = 0;

      pSubq->regReturn = ++pParse->nMem;
      topAddr = sqlite3VdbeAddOp0(v, 9);
      pSubq->addrFillSub = topAddr + 1;
      pItem->fg.isMaterialized = 1;
      if (pItem->fg.isCorrelated == 0) {
        onceAddr = sqlite3VdbeAddOp0(v, 15);
      } else {
      }
      sqlite3SelectDestInit(&dest, 10, pItem->iCursor);

      sqlite3VdbeExplain(pParse, 1, "MATERIALIZE %!S", pItem);
      sqlite3Select(pParse, pSub, &dest);
      pItem->pSTab->nRowLogEst = pSub->nSelectRow;
      if (onceAddr)
        sqlite3VdbeJumpHere(v, onceAddr);
      sqlite3VdbeAddOp2(v, 69, pSubq->regReturn, topAddr + 1);
      sqlite3VdbeJumpHere(v, topAddr);
      sqlite3ClearTempRegCache(pParse);
      if (pItem->fg.isCte && pItem->fg.isCorrelated == 0) {
        CteUse *pCteUse = pItem->u2.pCteUse;
        pCteUse->addrM9e = pSubq->addrFillSub;
        pCteUse->regRtn = pSubq->regReturn;
        pCteUse->iCur = pItem->iCursor;
        pCteUse->nRowEst = pSub->nSelectRow;
      }
    }
    if (db->mallocFailed)
      goto select_end;
    pParse->nHeight -= sqlite3SelectExprHeight(p);
    pParse->zAuthContext = zSavedAuthContext;
  }

  pEList = p->pEList;
  pWhere = p->pWhere;
  pGroupBy = p->pGroupBy;
  pHaving = p->pHaving;
  sDistinct.isTnct = (p->selFlags & 0x0000001) != 0;

  if ((p->selFlags & (0x0000001 | 0x0000008)) == 0x0000001 && sqlite3CopySortOrder(pEList, sSort.pOrderBy) &&
      sqlite3ExprListCompare(pEList, sSort.pOrderBy, -1) == 0 && (((db)->dbOptFlags & (0x00000004)) == 0) &&
      p->pWin == 0) {
    p->selFlags &= ~(u32)0x0000001;
    pGroupBy = p->pGroupBy = sqlite3ExprListDup(db, pEList, 0);
    if (pGroupBy) {
      for (i = 0; i < pGroupBy->nExpr; i++) {
        pGroupBy->a[i].u.x.iOrderByCol = i + 1;
      }
    }
    p->selFlags |= 0x0000008;

    sDistinct.isTnct = 2;
  }

  if (sSort.pOrderBy) {
    KeyInfo *pKeyInfo;
    pKeyInfo = sqlite3KeyInfoFromExprList(pParse, sSort.pOrderBy, 0, pEList->nExpr);
    sSort.iECursor = pParse->nTab++;
    sSort.addrSortIndex =
        sqlite3VdbeAddOp4(v, 120, sSort.iECursor, sSort.pOrderBy->nExpr + 1 + pEList->nExpr, 0, (char *)pKeyInfo, (-9));
  } else {
    sSort.addrSortIndex = -1;
  }

  if (pDest->eDest == 10) {
    sqlite3VdbeAddOp2(v, 120, pDest->iSDParm, pEList->nExpr);
    if (p->selFlags & 0x0000800) {
      int ii;
      for (ii = pEList->nExpr - 1; ii > 0 && pEList->a[ii].fg.bUsed == 0; ii--) {
        sqlite3ExprDelete(db, pEList->a[ii].pExpr);
        sqlite3DbFree(db, pEList->a[ii].zEName);
        pEList->nExpr--;
      }
      for (ii = 0; ii < pEList->nExpr; ii++) {
        if (pEList->a[ii].fg.bUsed == 0)
          pEList->a[ii].pExpr->op = 122;
      }
    }
  }

  iEnd = sqlite3VdbeMakeLabel(pParse);
  if ((p->selFlags & 0x0004000) == 0) {
    p->nSelectRow = 320;
  }
  if (p->pLimit)
    computeLimitRegisters(pParse, p, iEnd);
  if (p->iLimit == 0 && sSort.addrSortIndex >= 0) {
    sqlite3VdbeChangeOpcode(v, sSort.addrSortIndex, 121);
    sSort.sortFlags |= 0x01;
  }

  if (p->selFlags & 0x0000001) {
    sDistinct.tabTnct = pParse->nTab++;
    sDistinct.addrTnct = sqlite3VdbeAddOp4(v, 120, sDistinct.tabTnct, 0, 0,
                                           (char *)sqlite3KeyInfoFromExprList(pParse, p->pEList, 0, 0), (-9));
    sqlite3VdbeChangeP5(v, 8);
    sDistinct.eTnctType = 3;
  } else {
    sDistinct.eTnctType = 0;
  }

  if (!isAgg && pGroupBy == 0) {
    u16 wctrlFlags = (sDistinct.isTnct ? 0x0100 : 0) | (p->selFlags & 0x0004000);

    Window *pWin = p->pWin;
    if (pWin) {
      sqlite3WindowCodeInit(pParse, p);
    }

    pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, sSort.pOrderBy, p->pEList, p, wctrlFlags, p->nSelectRow);
    if (pWInfo == 0)
      goto select_end;
    if (sqlite3WhereOutputRowCount(pWInfo) < p->nSelectRow) {
      p->nSelectRow = sqlite3WhereOutputRowCount(pWInfo);
      if (pDest->eDest <= 4 && pDest->eDest >= 3) {
        p->nSelectRow -= 30;
      }
    }
    if (sDistinct.isTnct && sqlite3WhereIsDistinct(pWInfo)) {
      sDistinct.eTnctType = sqlite3WhereIsDistinct(pWInfo);
    }
    if (sSort.pOrderBy) {
      sSort.nOBSat = sqlite3WhereIsOrdered(pWInfo);
      sSort.labelOBLopt = sqlite3WhereOrderByLimitOptLabel(pWInfo);
      if (sSort.nOBSat == sSort.pOrderBy->nExpr) {
        sSort.pOrderBy = 0;
      }
    };

    if (sSort.addrSortIndex >= 0 && sSort.pOrderBy == 0) {
      sqlite3VdbeChangeToNoop(v, sSort.addrSortIndex);
    }

    if (pWin) {
      int addrGosub = sqlite3VdbeMakeLabel(pParse);
      int iCont = sqlite3VdbeMakeLabel(pParse);
      int iBreak = sqlite3VdbeMakeLabel(pParse);
      int regGosub = ++pParse->nMem;

      sqlite3WindowCodeStep(pParse, p, pWInfo, regGosub, addrGosub);

      sqlite3VdbeAddOp2(v, 9, 0, iBreak);
      sqlite3VdbeResolveLabel(v, addrGosub);
      sSort.labelOBLopt = 0;
      selectInnerLoop(pParse, p, -1, &sSort, &sDistinct, pDest, iCont, iBreak);
      sqlite3VdbeResolveLabel(v, iCont);
      sqlite3VdbeAddOp1(v, 69, regGosub);
      sqlite3VdbeResolveLabel(v, iBreak);
    } else {
      selectInnerLoop(pParse, p, -1, &sSort, &sDistinct, pDest, sqlite3WhereContinueLabel(pWInfo),
                      sqlite3WhereBreakLabel(pWInfo));

      sqlite3WhereEnd(pWInfo);
    }
  } else {
    NameContext sNC;
    int iAMem;
    int iBMem;
    int iUseFlag;

    int iAbortFlag;
    int groupBySort;
    int addrEnd;
    int sortPTab = 0;
    int sortOut = 0;
    int orderByGrp = 0;

    if (pGroupBy) {
      int k;
      struct ExprList_item *pItem;

      for (k = p->pEList->nExpr, pItem = p->pEList->a; k > 0; k--, pItem++) {
        pItem->u.x.iAlias = 0;
      }
      for (k = pGroupBy->nExpr, pItem = pGroupBy->a; k > 0; k--, pItem++) {
        pItem->u.x.iAlias = 0;
      }

      if (p->nSelectRow > 66)
        p->nSelectRow = 66;

      if (sqlite3CopySortOrder(pGroupBy, sSort.pOrderBy) && sqlite3ExprListCompare(pGroupBy, sSort.pOrderBy, -1) == 0) {
        orderByGrp = 1;
      }
    } else {
      p->nSelectRow = 0;
    }

    addrEnd = sqlite3VdbeMakeLabel(pParse);

    pAggInfo = (AggInfo*)(sqlite3DbMallocZero(db, sizeof(*pAggInfo)));
    if (pAggInfo) {
      sqlite3ParserAddCleanup(pParse, agginfoFree, pAggInfo);
    }
    if (db->mallocFailed) {
      goto select_end;
    }
    pAggInfo->selId = p->selId;

    memset(&sNC, 0, sizeof(sNC));
    sNC.pParse = pParse;
    sNC.pSrcList = pTabList;
    sNC.uNC.pAggInfo = pAggInfo;

    pAggInfo->nSortingColumn = pGroupBy ? pGroupBy->nExpr : 0;
    pAggInfo->pGroupBy = pGroupBy;
    sqlite3ExprAnalyzeAggList(&sNC, pEList);
    sqlite3ExprAnalyzeAggList(&sNC, sSort.pOrderBy);
    if (pHaving) {
      if (pGroupBy) {
        havingToWhere(pParse, p);
        pWhere = p->pWhere;
      }
      sqlite3ExprAnalyzeAggregates(&sNC, pHaving);
    }
    pAggInfo->nAccumulator = pAggInfo->nColumn;
    if (p->pGroupBy == 0 && p->pHaving == 0 && pAggInfo->nFunc == 1) {
      minMaxFlag = minMaxQuery(db, pAggInfo->aFunc[0].pFExpr, &pMinMaxOrderBy);
    } else {
      minMaxFlag = 0x0000;
    }
    analyzeAggFuncArgs(pAggInfo, &sNC);
    if (db->mallocFailed)
      goto select_end;

    if (pGroupBy) {
      KeyInfo *pKeyInfo;
      int addr1;
      int addrOutputRow;
      int regOutputRow;
      int addrSetAbort;
      int addrTopOfLoop;
      int addrSortingIdx;
      int addrReset;
      int regReset;
      ExprList *pDistinct = 0;
      u16 distFlag = 0;
      int eDist = 0;

      if (pAggInfo->nFunc == 1 && pAggInfo->aFunc[0].iDistinct >= 0 && (pAggInfo->aFunc[0].pFExpr != 0) &&
          ((((pAggInfo->aFunc[0].pFExpr)->flags & 0x001000) == 0)) && pAggInfo->aFunc[0].pFExpr->x.pList != 0) {
        Expr *pExpr = pAggInfo->aFunc[0].pFExpr->x.pList->a[0].pExpr;
        pExpr = sqlite3ExprDup(db, pExpr, 0);
        pDistinct = sqlite3ExprListDup(db, pGroupBy, 0);
        pDistinct = sqlite3ExprListAppend(pParse, pDistinct, pExpr);
        distFlag = pDistinct ? (0x0100 | 0x0400) : 0;
      }

      pAggInfo->sortingIdx = pParse->nTab++;
      pKeyInfo = sqlite3KeyInfoFromExprList(pParse, pGroupBy, 0, pAggInfo->nColumn);
      addrSortingIdx =
          sqlite3VdbeAddOp4(v, 121, pAggInfo->sortingIdx, pAggInfo->nSortingColumn, 0, (char *)pKeyInfo, (-9));

      iUseFlag = ++pParse->nMem;
      iAbortFlag = ++pParse->nMem;
      regOutputRow = ++pParse->nMem;
      addrOutputRow = sqlite3VdbeMakeLabel(pParse);
      regReset = ++pParse->nMem;
      addrReset = sqlite3VdbeMakeLabel(pParse);
      iAMem = pParse->nMem + 1;
      pParse->nMem += pGroupBy->nExpr;
      iBMem = pParse->nMem + 1;
      pParse->nMem += pGroupBy->nExpr;
      sqlite3VdbeAddOp2(v, 73, 0, iAbortFlag);
      sqlite3VdbeAddOp3(v, 77, 0, iAMem, iAMem + pGroupBy->nExpr - 1);
      sqlite3ExprNullRegisterRange(pParse, iAMem, pGroupBy->nExpr);

      sqlite3VdbeAddOp2(v, 10, regReset, addrReset);
      pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, pGroupBy, pDistinct, p,
                                 (sDistinct.isTnct == 2 ? 0x0080 : 0x0040) | (orderByGrp ? 0x0200 : 0) | distFlag, 0);
      if (pWInfo == 0) {
        sqlite3ExprListDelete(db, pDistinct);
        goto select_end;
      }
      if (pParse->pIdxEpr) {
        optimizeAggregateUseOfIndexedExpr(pParse, p, pAggInfo, &sNC);
      }
      assignAggregateRegisters(pParse, pAggInfo);
      eDist = sqlite3WhereIsDistinct(pWInfo);
      if (sqlite3WhereIsOrdered(pWInfo) == pGroupBy->nExpr) {
        groupBySort = 0;
      } else {
        int regBase;
        int regRecord;
        int nCol;
        int nGroupBy;

        sqlite3VdbeExplain(pParse, 0, "USE TEMP B-TREE FOR %s",
                           (sDistinct.isTnct && (p->selFlags & 0x0000001) == 0) ? "DISTINCT" : "GROUP BY");

        groupBySort = 1;
        nGroupBy = pGroupBy->nExpr;
        nCol = nGroupBy;
        j = nGroupBy;
        for (i = 0; i < pAggInfo->nColumn; i++) {
          if (pAggInfo->aCol[i].iSorterColumn >= j) {
            nCol++;
            j++;
          }
        }
        regBase = sqlite3GetTempRange(pParse, nCol);
        sqlite3ExprCodeExprList(pParse, pGroupBy, regBase, 0, 0);
        j = nGroupBy;
        pAggInfo->directMode = 1;
        for (i = 0; i < pAggInfo->nColumn; i++) {
          struct AggInfo_col *pCol = &pAggInfo->aCol[i];
          if (pCol->iSorterColumn >= j) {
            sqlite3ExprCode(pParse, pCol->pCExpr, j + regBase);
            j++;
          }
        }
        pAggInfo->directMode = 0;
        regRecord = sqlite3GetTempReg(pParse);
        sqlite3VdbeAddOp3(v, 99, regBase, nCol, regRecord);
        sqlite3VdbeAddOp2(v, 141, pAggInfo->sortingIdx, regRecord);
        sqlite3ReleaseTempReg(pParse, regRecord);
        sqlite3ReleaseTempRange(pParse, regBase, nCol);
        sqlite3WhereEnd(pWInfo);
        pAggInfo->sortingIdxPTab = sortPTab = pParse->nTab++;
        sortOut = sqlite3GetTempReg(pParse);
        sqlite3VdbeAddOp3(v, 123, sortPTab, sortOut, nCol);
        sqlite3VdbeAddOp2(v, 34, pAggInfo->sortingIdx, addrEnd);
        pAggInfo->useSortingIdx = 1;
      }

      if (pParse->pIdxEpr) {
        aggregateConvertIndexedExprRefToColumn(pAggInfo);
      }

      if (orderByGrp && (((db)->dbOptFlags & (0x00000004)) == 0) && (groupBySort || sqlite3WhereIsSorted(pWInfo))) {
        sSort.pOrderBy = 0;
        sqlite3VdbeChangeToNoop(v, sSort.addrSortIndex);
      }

      addrTopOfLoop = sqlite3VdbeCurrentAddr(v);
      if (groupBySort) {
        sqlite3VdbeAddOp3(v, 135, pAggInfo->sortingIdx, sortOut, sortPTab);
      }
      for (j = 0; j < pGroupBy->nExpr; j++) {
        int iOrderByCol = pGroupBy->a[j].u.x.iOrderByCol;

        if (groupBySort) {
          sqlite3VdbeAddOp3(v, 96, sortPTab, j, iBMem + j);
        } else {
          pAggInfo->directMode = 1;
          sqlite3ExprCode(pParse, pGroupBy->a[j].pExpr, iBMem + j);
        }

        if (iOrderByCol) {
          Expr *pX = p->pEList->a[iOrderByCol - 1].pExpr;
          Expr *pBase = sqlite3ExprSkipCollateAndLikely(pX);
          while ((pBase != 0) && pBase->op == 179) {
            pX = pBase->pLeft;
            pBase = sqlite3ExprSkipCollateAndLikely(pX);
          }
          if ((pBase != 0) && pBase->op != 170 && pBase->op != 176) {
            sqlite3ExprToRegister(pX, iAMem + j);
          }
        }
      }
      sqlite3VdbeAddOp4(v, 92, iAMem, iBMem, pGroupBy->nExpr, (char *)sqlite3KeyInfoRef(pKeyInfo), (-9));
      addr1 = sqlite3VdbeCurrentAddr(v);
      sqlite3VdbeAddOp3(v, 14, addr1 + 1, 0, addr1 + 1);

      sqlite3VdbeAddOp2(v, 10, regOutputRow, addrOutputRow);
      sqlite3ExprCodeMove(pParse, iBMem, iAMem, pGroupBy->nExpr);
      sqlite3VdbeAddOp2(v, 61, iAbortFlag, addrEnd);
      sqlite3VdbeAddOp2(v, 10, regReset, addrReset);

      sqlite3VdbeJumpHere(v, addr1);
      updateAccumulator(pParse, iUseFlag, pAggInfo, eDist);
      sqlite3VdbeAddOp2(v, 73, 1, iUseFlag);

      if (groupBySort) {
        sqlite3VdbeAddOp2(v, 38, pAggInfo->sortingIdx, addrTopOfLoop);
      } else {
        sqlite3WhereEnd(pWInfo);
        sqlite3VdbeChangeToNoop(v, addrSortingIdx);
      }
      sqlite3ExprListDelete(db, pDistinct);

      sqlite3VdbeAddOp2(v, 10, regOutputRow, addrOutputRow);

      sqlite3VdbeGoto(v, addrEnd);

      addrSetAbort = sqlite3VdbeCurrentAddr(v);
      sqlite3VdbeAddOp2(v, 73, 1, iAbortFlag);
      sqlite3VdbeAddOp1(v, 69, regOutputRow);
      sqlite3VdbeResolveLabel(v, addrOutputRow);
      addrOutputRow = sqlite3VdbeCurrentAddr(v);
      sqlite3VdbeAddOp2(v, 61, iUseFlag, addrOutputRow + 2);
      sqlite3VdbeAddOp1(v, 69, regOutputRow);
      finalizeAggFunctions(pParse, pAggInfo);
      sqlite3ExprIfFalse(pParse, pHaving, addrOutputRow + 1, 0x10);
      selectInnerLoop(pParse, p, -1, &sSort, &sDistinct, pDest, addrOutputRow + 1, addrSetAbort);
      sqlite3VdbeAddOp1(v, 69, regOutputRow);

      sqlite3VdbeResolveLabel(v, addrReset);
      resetAccumulator(pParse, pAggInfo);
      sqlite3VdbeAddOp2(v, 73, 0, iUseFlag);
      sqlite3VdbeAddOp1(v, 69, regReset);

      if (distFlag != 0 && eDist != 0) {
        struct AggInfo_func *pF = &pAggInfo->aFunc[0];
        fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
      }
    } else {
      Table *pTab;
      if ((pTab = isSimpleCount(p, pAggInfo)) != 0) {
        const int iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
        const int iCsr = pParse->nTab++;
        Index *pIdx;
        KeyInfo *pKeyInfo = 0;
        Index *pBest = 0;
        Pgno iRoot = pTab->tnum;

        sqlite3CodeVerifySchema(pParse, iDb);
        sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);

        if (!(((pTab)->tabFlags & 0x00000080) == 0))
          pBest = sqlite3PrimaryKeyIndex(pTab);
        if (!p->pSrc->a[0].fg.notIndexed) {
          for (pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext) {
            if (pIdx->bUnordered == 0 && pIdx->szIdxRow < pTab->szTabRow && pIdx->pPartIdxWhere == 0 &&
                (!pBest || pIdx->szIdxRow < pBest->szIdxRow)) {
              pBest = pIdx;
            }
          }
        }
        if (pBest) {
          iRoot = pBest->tnum;
          pKeyInfo = sqlite3KeyInfoOfIndex(pParse, pBest);
        }

        sqlite3VdbeAddOp4Int(v, 114, iCsr, (int)iRoot, iDb, 1);
        if (pKeyInfo) {
          sqlite3VdbeChangeP4(v, -1, (char *)pKeyInfo, (-9));
        }
        assignAggregateRegisters(pParse, pAggInfo);
        sqlite3VdbeAddOp2(v, 100, iCsr, ((pAggInfo)->iFirstReg + (pAggInfo)->nColumn + (0)));
        sqlite3VdbeAddOp1(v, 124, iCsr);
        explainSimpleCount(pParse, pTab, pBest);
      } else {
        int regAcc = 0;
        ExprList *pDistinct = 0;
        u16 distFlag = 0;
        int eDist;

        if (pAggInfo->nAccumulator) {
          for (i = 0; i < pAggInfo->nFunc; i++) {
            if ((((pAggInfo->aFunc[i].pFExpr)->flags & (u32)(0x1000000)) != 0)) {
              continue;
            }
            if (pAggInfo->aFunc[i].pFunc->funcFlags & 0x0020) {
              break;
            }
          }
          if (i == pAggInfo->nFunc) {
            regAcc = ++pParse->nMem;
            sqlite3VdbeAddOp2(v, 73, 0, regAcc);
          }
        } else if (pAggInfo->nFunc == 1 && pAggInfo->aFunc[0].iDistinct >= 0) {
          pDistinct = pAggInfo->aFunc[0].pFExpr->x.pList;
          distFlag = pDistinct ? (0x0100 | 0x0400) : 0;
        }
        assignAggregateRegisters(pParse, pAggInfo);

        resetAccumulator(pParse, pAggInfo);

        pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, pMinMaxOrderBy, pDistinct, p, minMaxFlag | distFlag, 0);
        if (pWInfo == 0) {
          goto select_end;
        };
        eDist = sqlite3WhereIsDistinct(pWInfo);
        updateAccumulator(pParse, regAcc, pAggInfo, eDist);
        if (eDist != 0) {
          struct AggInfo_func *pF = pAggInfo->aFunc;
          if (pF) {
            fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
          }
        }

        if (regAcc)
          sqlite3VdbeAddOp2(v, 73, 1, regAcc);
        if (minMaxFlag) {
          sqlite3WhereMinMaxOptEarlyOut(v, pWInfo);
        };
        sqlite3WhereEnd(pWInfo);
        finalizeAggFunctions(pParse, pAggInfo);
      }

      sSort.pOrderBy = 0;
      sqlite3ExprIfFalse(pParse, pHaving, addrEnd, 0x10);
      selectInnerLoop(pParse, p, -1, 0, 0, pDest, addrEnd, addrEnd);
    }
    sqlite3VdbeResolveLabel(v, addrEnd);
  }

  if (sDistinct.eTnctType == 3) {
    explainTempTable(pParse, "DISTINCT");
  }

  if (sSort.pOrderBy) {
    generateSortTail(pParse, p, &sSort, pEList->nExpr, pDest);
  }

  sqlite3VdbeResolveLabel(v, iEnd);

  rc = (pParse->nErr > 0);

select_end:
  sqlite3ExprListDelete(db, pMinMaxOrderBy);

  sqlite3VdbeExplainPop(pParse);
  return rc;
}

Trigger *sqlite3TriggerList(Parse *pParse, Table *pTab) {
  Schema *pTmpSchema;
  Trigger *pList;
  HashElem *p;

  pTmpSchema = pParse->db->aDb[1].pSchema;
  p = ((&pTmpSchema->trigHash)->first);
  pList = pTab->pTrigger;
  while (p) {
    Trigger *pTrig = (Trigger *)((p)->data);
    if (pTrig->pTabSchema == pTab->pSchema && pTrig->table && 0 == sqlite3StrICmp(pTrig->table, pTab->zName) &&
        (pTrig->pTabSchema != pTmpSchema || pTrig->bReturning)) {
      pTrig->pNext = pList;
      pList = pTrig;
    } else if (pTrig->op == 151) {
      pTrig->table = pTab->zName;
      pTrig->pTabSchema = pTab->pSchema;
      pTrig->pNext = pList;
      pList = pTrig;
    }
    p = ((p)->next);
  }

  return pList;
}

void sqlite3BeginTrigger(Parse *pParse, Token *pName1, Token *pName2, int tr_tm, int op, IdList *pColumns,
                         SrcList *pTableName, Expr *pWhen, int isTemp, int noErr) {
  Trigger *pTrigger = 0;
  Table *pTab;
  char *zName = 0;
  sqlite3 *db = pParse->db;
  int iDb;
  Token *pName;
  DbFixer sFix;

  if (isTemp) {
    if (pName2->n > 0) {
      sqlite3ErrorMsg(pParse, "temporary trigger may not have qualified name");
      goto trigger_cleanup;
    }
    iDb = 1;
    pName = pName1;
  } else {
    iDb = sqlite3TwoPartName(pParse, pName1, pName2, &pName);
    if (iDb < 0) {
      goto trigger_cleanup;
    }
  }
  if (!pTableName || db->mallocFailed) {
    goto trigger_cleanup;
  }

  if (db->init.busy && iDb != 1) {
    sqlite3DbFree(db, pTableName->a[0].u4.zDatabase);
    pTableName->a[0].u4.zDatabase = 0;
  }

  pTab = sqlite3SrcListLookup(pParse, pTableName);
  if (db->init.busy == 0 && pName2->n == 0 && pTab && pTab->pSchema == db->aDb[1].pSchema) {
    iDb = 1;
  }

  if (db->mallocFailed)
    goto trigger_cleanup;

  sqlite3FixInit(&sFix, pParse, iDb, "trigger", pName);
  if (sqlite3FixSrcList(&sFix, pTableName)) {
    goto trigger_cleanup;
  }
  pTab = sqlite3SrcListLookup(pParse, pTableName);
  if (!pTab) {
    goto trigger_orphan_error;
  }
  if ((pTab)->eTabType == 1) {
    sqlite3ErrorMsg(pParse, "cannot create triggers on virtual tables");
    goto trigger_orphan_error;
  }
  if ((pTab->tabFlags & 0x00001000) != 0 && sqlite3ReadOnlyShadowTables(db)) {
    sqlite3ErrorMsg(pParse, "cannot create triggers on shadow tables");
    goto trigger_orphan_error;
  }

  zName = sqlite3NameFromToken(db, pName);
  if (zName == 0) {
    goto trigger_cleanup;
  }
  if (sqlite3CheckObjectName(pParse, zName, "trigger", pTab->zName)) {
    goto trigger_cleanup;
  }

  if (!(pParse->eParseMode >= 2)) {
    if (sqlite3HashFind(&(db->aDb[iDb].pSchema->trigHash), zName)) {
      if (!noErr) {
        sqlite3ErrorMsg(pParse, "trigger %T already exists", pName);
      } else {
        sqlite3CodeVerifySchema(pParse, iDb);
      }
      goto trigger_cleanup;
    }
  }

  if (sqlite3_strnicmp(pTab->zName, "sqlite_", 7) == 0) {
    sqlite3ErrorMsg(pParse, "cannot create trigger on system table");
    goto trigger_cleanup;
  }

  if (((pTab)->eTabType == 2) && tr_tm != 66) {
    sqlite3ErrorMsg(pParse, "cannot create %s trigger on view: %S", (tr_tm == 33) ? "BEFORE" : "AFTER", pTableName->a);
    goto trigger_orphan_error;
  }
  if (!((pTab)->eTabType == 2) && tr_tm == 66) {
    sqlite3ErrorMsg(pParse,
                    "cannot create INSTEAD OF"
                    " trigger on table: %S",
                    pTableName->a);
    goto trigger_orphan_error;
  }

  if (!(pParse->eParseMode >= 2)) {
    int iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    int code = SQLITE_CREATE_TRIGGER;
    const char *zDb = db->aDb[iTabDb].zDbSName;
    const char *zDbTrig = isTemp ? db->aDb[1].zDbSName : zDb;
    if (iTabDb == 1 || isTemp)
      code = SQLITE_CREATE_TEMP_TRIGGER;
    if (sqlite3AuthCheck(pParse, code, zName, pTab->zName, zDbTrig)) {
      goto trigger_cleanup;
    }
    if (sqlite3AuthCheck(pParse, SQLITE_INSERT, ((!0) && (iTabDb == 1) ? "sqlite_temp_master" : "sqlite_master"), 0,
                         zDb)) {
      goto trigger_cleanup;
    }
  }

  if (tr_tm == 66) {
    tr_tm = 33;
  }

  pTrigger = (Trigger *)sqlite3DbMallocZero(db, sizeof(Trigger));
  if (pTrigger == 0)
    goto trigger_cleanup;
  pTrigger->zName = zName;
  zName = 0;
  pTrigger->table = sqlite3DbStrDup(db, pTableName->a[0].zName);
  pTrigger->pSchema = db->aDb[iDb].pSchema;
  pTrigger->pTabSchema = pTab->pSchema;
  pTrigger->op = (u8)op;
  pTrigger->tr_tm = tr_tm == 33 ? 1 : 2;
  if ((pParse->eParseMode >= 2)) {
    sqlite3RenameTokenRemap(pParse, pTrigger->table, pTableName->a[0].zName);
    pTrigger->pWhen = pWhen;
    pWhen = 0;
  } else {
    pTrigger->pWhen = sqlite3ExprDup(db, pWhen, 0x0001);
  }
  pTrigger->pColumns = pColumns;
  pColumns = 0;

  pParse->pNewTrigger = pTrigger;

trigger_cleanup:
  sqlite3DbFree(db, zName);
  sqlite3SrcListDelete(db, pTableName);
  sqlite3IdListDelete(db, pColumns);
  sqlite3ExprDelete(db, pWhen);
  if (!pParse->pNewTrigger) {
    sqlite3DeleteTrigger(db, pTrigger);
  } else {
  }
  return;

trigger_orphan_error:
  if (db->init.iDb == 1) {
    db->init.orphanTrigger = 1;
  }
  goto trigger_cleanup;
}

void sqlite3FinishTrigger(Parse *pParse, TriggerStep *pStepList, Token *pAll) {
  Trigger *pTrig = pParse->pNewTrigger;
  char *zName;
  sqlite3 *db = pParse->db;
  DbFixer sFix;
  int iDb;
  Token nameToken;

  pParse->pNewTrigger = 0;
  if ((pParse->nErr) || !pTrig)
    goto triggerfinish_cleanup;
  zName = pTrig->zName;
  iDb = sqlite3SchemaToIndex(pParse->db, pTrig->pSchema);

  pTrig->step_list = pStepList;
  while (pStepList) {
    pStepList->pTrig = pTrig;
    pStepList = pStepList->pNext;
  }
  sqlite3TokenInit(&nameToken, pTrig->zName);
  sqlite3FixInit(&sFix, pParse, iDb, "trigger", &nameToken);
  if (sqlite3FixTriggerStep(&sFix, pTrig->step_list) || sqlite3FixExpr(&sFix, pTrig->pWhen)) {
    goto triggerfinish_cleanup;
  }

  if ((pParse->eParseMode >= 2)) {
    pParse->pNewTrigger = pTrig;
    pTrig = 0;
  } else if (!db->init.busy) {
    Vdbe *v;
    char *z;

    if (sqlite3ReadOnlyShadowTables(db)) {
      TriggerStep *pStep;
      for (pStep = pTrig->step_list; pStep; pStep = pStep->pNext) {
        if (pStep->pSrc != 0 && sqlite3ShadowTableName(db, pStep->pSrc->a[0].zName)) {
          sqlite3ErrorMsg(pParse, "trigger \"%s\" may not write to shadow table \"%s\"", pTrig->zName,
                          pStep->pSrc->a[0].zName);
          goto triggerfinish_cleanup;
        }
      }
    }

    v = sqlite3GetVdbe(pParse);
    if (v == 0)
      goto triggerfinish_cleanup;
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    z = sqlite3DbStrNDup(db, (char *)pAll->z, pAll->n);
    sqlite3NestedParse(pParse,
                       "INSERT INTO %Q."
                       "sqlite_master"
                       " VALUES('trigger',%Q,%Q,0,'CREATE TRIGGER %q')",
                       db->aDb[iDb].zDbSName, zName, pTrig->table, z);
    sqlite3DbFree(db, z);
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3VdbeAddParseSchemaOp(v, iDb, sqlite3MPrintf(db, "type='trigger' AND name='%q'", zName), 0);
  }

  if (db->init.busy) {
    Trigger *pLink = pTrig;
    Hash *pHash = &db->aDb[iDb].pSchema->trigHash;

    pTrig = (Trigger*)(sqlite3HashInsert(pHash, zName, pTrig));
    if (pTrig) {
      sqlite3OomFault(db);
    } else if (pLink->pSchema == pLink->pTabSchema) {
      Table *pTab;
      pTab = (Table*)(sqlite3HashFind(&pLink->pTabSchema->tblHash, pLink->table));

      pLink->pNext = pTab->pTrigger;
      pTab->pTrigger = pLink;
    }
  }

triggerfinish_cleanup:
  sqlite3DeleteTrigger(db, pTrig);

  sqlite3DeleteTriggerStep(db, pStepList);
}

TriggerStep *triggerStepAllocate(Parse *pParse, u8 op, SrcList *pTabList, const char *zStart, const char *zEnd) {
  Trigger *pNew = pParse->pNewTrigger;
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep = 0;

  if (pParse->nErr == 0) {
    if (pNew && pNew->pSchema != db->aDb[1].pSchema && pTabList->a[0].u4.zDatabase) {
      sqlite3ErrorMsg(pParse,
                      "qualified table names are not allowed on INSERT, UPDATE, and DELETE "
                      "statements within triggers");
    } else {
      pTriggerStep = (TriggerStep*)(sqlite3DbMallocZero(db, sizeof(TriggerStep)));
      if (pTriggerStep) {
        pTriggerStep->pSrc = sqlite3SrcListDup(db, pTabList, 0x0001);
        pTriggerStep->op = op;
        pTriggerStep->zSpan = triggerSpanDup(db, zStart, zEnd);
        if (pTriggerStep->pSrc && (pParse->eParseMode >= 2)) {
          sqlite3RenameTokenRemap(pParse, pTriggerStep->pSrc->a[0].zName, pTabList->a[0].zName);
        }
      }
    }
  }

  sqlite3SrcListDelete(db, pTabList);
  return pTriggerStep;
}

TriggerStep *sqlite3TriggerInsertStep(Parse *pParse, SrcList *pTabList, IdList *pColumn, Select *pSelect, u8 orconf,
                                      Upsert *pUpsert, const char *zStart, const char *zEnd) {
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  pTriggerStep = triggerStepAllocate(pParse, 128, pTabList, zStart, zEnd);
  if (pTriggerStep) {
    if ((pParse->eParseMode >= 2)) {
      pTriggerStep->pSelect = pSelect;
      pSelect = 0;
    } else {
      pTriggerStep->pSelect = sqlite3SelectDup(db, pSelect, 0x0001);
    }
    pTriggerStep->pIdList = pColumn;
    pTriggerStep->pUpsert = pUpsert;
    pTriggerStep->orconf = orconf;
    if (pUpsert) {
      sqlite3HasExplicitNulls(pParse, pUpsert->pUpsertTarget);
    }
  } else {
    sqlite3IdListDelete(db, pColumn);
    sqlite3UpsertDelete(db, pUpsert);
  }
  sqlite3SelectDelete(db, pSelect);

  return pTriggerStep;
}

TriggerStep *sqlite3TriggerUpdateStep(Parse *pParse, SrcList *pTabList, SrcList *pFrom, ExprList *pEList, Expr *pWhere,
                                      u8 orconf, const char *zStart, const char *zEnd) {
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  pTriggerStep = triggerStepAllocate(pParse, 130, pTabList, zStart, zEnd);
  if (pTriggerStep) {
    SrcList *pFromDup = 0;
    if ((pParse->eParseMode >= 2)) {
      pTriggerStep->pExprList = pEList;
      pTriggerStep->pWhere = pWhere;
      pFromDup = pFrom;
      pEList = 0;
      pWhere = 0;
      pFrom = 0;
    } else {
      pTriggerStep->pExprList = sqlite3ExprListDup(db, pEList, 0x0001);
      pTriggerStep->pWhere = sqlite3ExprDup(db, pWhere, 0x0001);
      pFromDup = sqlite3SrcListDup(db, pFrom, 0x0001);
    }
    pTriggerStep->orconf = orconf;

    if (pFromDup && !(pParse->eParseMode >= 2)) {
      Select *pSub;
      Token as = {0, 0};
      pSub = sqlite3SelectNew(pParse, 0, pFromDup, 0, 0, 0, 0, 0x0000800, 0);
      pFromDup = sqlite3SrcListAppendFromTerm(pParse, 0, 0, 0, &as, pSub, 0);
    }
    if (pFromDup && pTriggerStep->pSrc) {
      pTriggerStep->pSrc = sqlite3SrcListAppendList(pParse, pTriggerStep->pSrc, pFromDup);
    } else {
      sqlite3SrcListDelete(db, pFromDup);
    }
  }
  sqlite3ExprListDelete(db, pEList);
  sqlite3ExprDelete(db, pWhere);
  sqlite3SrcListDelete(db, pFrom);
  return pTriggerStep;
}

TriggerStep *sqlite3TriggerDeleteStep(Parse *pParse, SrcList *pTabList, Expr *pWhere, const char *zStart,
                                      const char *zEnd) {
  sqlite3 *db = pParse->db;
  TriggerStep *pTriggerStep;

  pTriggerStep = triggerStepAllocate(pParse, 129, pTabList, zStart, zEnd);
  if (pTriggerStep) {
    if ((pParse->eParseMode >= 2)) {
      pTriggerStep->pWhere = pWhere;
      pWhere = 0;
    } else {
      pTriggerStep->pWhere = sqlite3ExprDup(db, pWhere, 0x0001);
    }
    pTriggerStep->orconf = 11;
  }
  sqlite3ExprDelete(db, pWhere);
  return pTriggerStep;
}

void sqlite3DropTrigger(Parse *pParse, SrcList *pName, int noErr) {
  Trigger *pTrigger = 0;
  int i;
  const char *zDb;
  const char *zName;
  sqlite3 *db = pParse->db;

  if (db->mallocFailed)
    goto drop_trigger_cleanup;
  if (SQLITE_OK != sqlite3ReadSchema(pParse)) {
    goto drop_trigger_cleanup;
  }

  zDb = pName->a[0].u4.zDatabase;
  zName = pName->a[0].zName;

  for (i = 0; i < db->nDb; i++) {
    int j = (i < 2) ? i ^ 1 : i;
    if (zDb && sqlite3DbIsNamed(db, j, zDb) == 0)
      continue;

    pTrigger = (Trigger*)(sqlite3HashFind(&(db->aDb[j].pSchema->trigHash), zName));
    if (pTrigger)
      break;
  }
  if (!pTrigger) {
    if (!noErr) {
      sqlite3ErrorMsg(pParse, "no such trigger: %S", pName->a);
    } else {
      sqlite3CodeVerifyNamedSchema(pParse, zDb);
    }
    pParse->checkSchema = 1;
    goto drop_trigger_cleanup;
  }
  sqlite3DropTriggerPtr(pParse, pTrigger);

drop_trigger_cleanup:
  sqlite3SrcListDelete(db, pName);
}

void sqlite3DropTriggerPtr(Parse *pParse, Trigger *pTrigger) {
  Table *pTable;
  Vdbe *v;
  sqlite3 *db = pParse->db;
  int iDb;

  iDb = sqlite3SchemaToIndex(pParse->db, pTrigger->pSchema);

  pTable = tableOfTrigger(pTrigger);

  if (pTable) {
    int code = SQLITE_DROP_TRIGGER;
    const char *zDb = db->aDb[iDb].zDbSName;
    const char *zTab = ((!0) && (iDb == 1) ? "sqlite_temp_master" : "sqlite_master");
    if (iDb == 1)
      code = SQLITE_DROP_TEMP_TRIGGER;
    if (sqlite3AuthCheck(pParse, code, pTrigger->zName, pTable->zName, zDb) ||
        sqlite3AuthCheck(pParse, SQLITE_DELETE, zTab, 0, zDb)) {
      return;
    }
  }

  if ((v = sqlite3GetVdbe(pParse)) != 0) {
    sqlite3NestedParse(pParse,
                       "DELETE FROM %Q."
                       "sqlite_master"
                       " WHERE name=%Q AND type='trigger'",
                       db->aDb[iDb].zDbSName, pTrigger->zName);
    sqlite3ChangeCookie(pParse, iDb);
    sqlite3VdbeAddOp4(v, 156, iDb, 0, 0, pTrigger->zName, 0);
  }
}

__attribute__((noinline)) Trigger *triggersReallyExist(Parse *pParse, Table *pTab, int op, ExprList *pChanges,
                                                       int *pMask) {
  int mask = 0;
  Trigger *pList = 0;
  Trigger *p;

  pList = sqlite3TriggerList(pParse, pTab);

  if (pList != 0) {
    p = pList;
    if ((pParse->db->flags & 0x00040000) == 0 && pTab->pTrigger != 0 &&
        sqlite3SchemaToIndex(pParse->db, pTab->pTrigger->pSchema) != 1) {
      if (pList == pTab->pTrigger) {
        pList = 0;
        goto exit_triggers_exist;
      }
      while ((p->pNext) && p->pNext != pTab->pTrigger)
        p = p->pNext;
      p->pNext = 0;
      p = pList;
    }
    do {
      if (p->op == op && checkColumnOverlap(p->pColumns, pChanges)) {
        mask |= p->tr_tm;
      } else if (p->op == 151) {
        p->op = op;
        if ((pTab)->eTabType == 1) {
          if (op != 128) {
            sqlite3ErrorMsg(pParse, "%s RETURNING is not available on virtual tables", op == 129 ? "DELETE" : "UPDATE");
          }
          p->tr_tm = 1;
        } else {
          p->tr_tm = 2;
        }
        mask |= p->tr_tm;
      } else if (p->bReturning && p->op == 128 && op == 130 && ((pParse)->pToplevel == 0)) {
        mask |= p->tr_tm;
      }
      p = p->pNext;
    } while (p);
  }
exit_triggers_exist:
  if (pMask) {
    *pMask = mask;
  }
  return (mask ? pList : 0);
}

Trigger *sqlite3TriggersExist(Parse *pParse, Table *pTab, int op, ExprList *pChanges, int *pMask) {
  if ((pTab->pTrigger == 0 && !tempTriggersExist(pParse->db)) || pParse->disableTriggers) {
    if (pMask)
      *pMask = 0;
    return 0;
  }
  return triggersReallyExist(pParse, pTab, op, pChanges, pMask);
}

int isAsteriskTerm(Parse *pParse, Expr *pTerm) {
  if (pTerm->op == 180)
    return 1;
  if (pTerm->op != 142)
    return 0;

  if (pTerm->pRight->op != 180)
    return 0;
  sqlite3ErrorMsg(pParse, "RETURNING may not use \"TABLE.*\" wildcards");
  return 1;
}

ExprList *sqlite3ExpandReturning(Parse *pParse, ExprList *pList, Table *pTab) {
  ExprList *pNew = 0;
  sqlite3 *db = pParse->db;
  int i;

  for (i = 0; i < pList->nExpr; i++) {
    Expr *pOldExpr = pList->a[i].pExpr;
    if (pOldExpr == 0)
      continue;
    if (isAsteriskTerm(pParse, pOldExpr)) {
      int jj;
      for (jj = 0; jj < pTab->nCol; jj++) {
        Expr *pNewExpr;
        if ((((pTab->aCol + jj)->colFlags & 0x0002) != 0))
          continue;
        pNewExpr = sqlite3Expr(db, 60, pTab->aCol[jj].zCnName);
        pNew = sqlite3ExprListAppend(pParse, pNew, pNewExpr);
        if (!db->mallocFailed) {
          struct ExprList_item *pItem = &pNew->a[pNew->nExpr - 1];
          pItem->zEName = sqlite3DbStrDup(db, pTab->aCol[jj].zCnName);
          pItem->fg.eEName = 0;
        }
      }
    } else {
      Expr *pNewExpr = sqlite3ExprDup(db, pOldExpr, 0);
      pNew = sqlite3ExprListAppend(pParse, pNew, pNewExpr);
      if (!db->mallocFailed && (pList->a[i].zEName != 0)) {
        struct ExprList_item *pItem = &pNew->a[pNew->nExpr - 1];
        pItem->zEName = sqlite3DbStrDup(db, pList->a[i].zEName);
        pItem->fg.eEName = pList->a[i].fg.eEName;
      }
    }
  }
  return pNew;
}

void codeReturningTrigger(Parse *pParse, Trigger *pTrigger, Table *pTab, int regIn) {
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  ExprList *pNew;
  Returning *pReturning;
  Select sSelect;
  SrcList *pFrom;
  union {
    SrcList sSrc;
    u8 fromSpace[(offsetof(SrcList, a) + sizeof(SrcItem))];
  } uSrc;

  if (!pParse->bReturning) {
    return;
  }

  pReturning = pParse->u1.d.pReturning;
  if (pTrigger != &(pReturning->retTrig)) {
    return;
  }
  memset(&sSelect, 0, sizeof(sSelect));
  memset(&uSrc, 0, sizeof(uSrc));
  pFrom = &uSrc.sSrc;
  sSelect.pEList = sqlite3ExprListDup(db, pReturning->pReturnEL, 0);
  sSelect.pSrc = pFrom;
  pFrom->nSrc = 1;
  pFrom->a[0].pSTab = pTab;
  pFrom->a[0].zName = pTab->zName;
  pFrom->a[0].iCursor = -1;
  sqlite3SelectPrep(pParse, &sSelect, 0);
  if (pParse->nErr == 0) {
    sqlite3GenerateColumnNames(pParse, &sSelect);
  }
  sqlite3ExprListDelete(db, sSelect.pEList);
  pNew = sqlite3ExpandReturning(pParse, pReturning->pReturnEL, pTab);
  if (pParse->nErr == 0) {
    NameContext sNC;
    memset(&sNC, 0, sizeof(sNC));
    if (pReturning->nRetCol == 0) {
      pReturning->nRetCol = pNew->nExpr;
      pReturning->iRetCur = pParse->nTab++;
    }
    sNC.pParse = pParse;
    sNC.uNC.iBaseReg = regIn;
    sNC.ncFlags = 0x000400;
    pParse->eTriggerOp = pTrigger->op;
    pParse->pTriggerTab = pTab;
    if (sqlite3ResolveExprListNames(&sNC, pNew) == SQLITE_OK && (!db->mallocFailed)) {
      int i;
      int nCol = pNew->nExpr;
      int reg = pParse->nMem + 1;
      sqlite3ProcessReturningSubqueries(pNew, pTab);
      pParse->nMem += nCol + 2;
      pReturning->iRetReg = reg;
      for (i = 0; i < nCol; i++) {
        Expr *pCol = pNew->a[i].pExpr;

        sqlite3ExprCodeFactorable(pParse, pCol, reg + i);
        if (sqlite3ExprAffinity(pCol) == 0x45) {
          sqlite3VdbeAddOp1(v, 89, reg + i);
        }
      }
      sqlite3VdbeAddOp3(v, 99, reg, i, reg + i);
      sqlite3VdbeAddOp2(v, 129, pReturning->iRetCur, reg + i + 1);
      sqlite3VdbeAddOp3(v, 130, pReturning->iRetCur, reg + i, reg + i + 1);
    }
  }
  sqlite3ExprListDelete(db, pNew);
  pParse->eTriggerOp = 0;
  pParse->pTriggerTab = 0;
}

int codeTriggerProgram(Parse *pParse, TriggerStep *pStepList, int orconf) {
  TriggerStep *pStep;
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;

  for (pStep = pStepList; pStep; pStep = pStep->pNext) {
    pParse->eOrconf = (orconf == 11) ? pStep->orconf : (u8)orconf;

    if (pStep->zSpan) {
      sqlite3VdbeAddOp4(v, 186, 0x7fffffff, 1, 0, sqlite3MPrintf(db, "-- %s", pStep->zSpan), (-7));
    }

    switch (pStep->op) {
      case 130: {
        sqlite3Update(pParse, sqlite3SrcListDup(db, pStep->pSrc, 0), sqlite3ExprListDup(db, pStep->pExprList, 0),
                      sqlite3ExprDup(db, pStep->pWhere, 0), pParse->eOrconf, 0, 0, 0);
        sqlite3VdbeAddOp0(v, 133);
        break;
      }
      case 128: {
        sqlite3Insert(pParse, sqlite3SrcListDup(db, pStep->pSrc, 0), sqlite3SelectDup(db, pStep->pSelect, 0),
                      sqlite3IdListDup(db, pStep->pIdList), pParse->eOrconf, sqlite3UpsertDup(db, pStep->pUpsert));
        sqlite3VdbeAddOp0(v, 133);
        break;
      }
      case 129: {
        sqlite3DeleteFrom(pParse, sqlite3SrcListDup(db, pStep->pSrc, 0), sqlite3ExprDup(db, pStep->pWhere, 0), 0, 0);
        sqlite3VdbeAddOp0(v, 133);
        break;
      }
      default: {
        SelectDest sDest;
        Select *pSelect = sqlite3SelectDup(db, pStep->pSelect, 0);
        sqlite3SelectDestInit(&sDest, 2, 0);
        sqlite3Select(pParse, pSelect, &sDest);
        sqlite3SelectDelete(db, pSelect);
        break;
      }
    }
  }

  return 0;
}

void transferParseError(Parse *pTo, Parse *pFrom) {
  if (pTo->nErr == 0) {
    pTo->zErrMsg = pFrom->zErrMsg;
    pTo->nErr = pFrom->nErr;
    pTo->rc = pFrom->rc;
  } else {
    sqlite3DbFree(pFrom->db, pFrom->zErrMsg);
  }
}

TriggerPrg *codeRowTrigger(Parse *pParse, Trigger *pTrigger, Table *pTab, int orconf) {
  Parse *pTop;
  sqlite3 *db = pParse->db;
  TriggerPrg *pPrg;
  Expr *pWhen = 0;
  Vdbe *v;
  NameContext sNC;
  SubProgram *pProgram = 0;
  int iEndTrigger = 0;
  Parse sSubParse;
  int nDepth;

  pTop = pParse;
  for (nDepth = 0; pTop->pOuterParse; pTop = pTop->pOuterParse, nDepth++) {
  }
  if (nDepth >= db->aLimit[SQLITE_LIMIT_TRIGGER_DEPTH]) {
    sqlite3ErrorMsg(pParse, "triggers nested too deep");
    return 0;
  }

  pTop = ((pParse)->pToplevel ? (pParse)->pToplevel : (pParse));

  pPrg = (TriggerPrg*)(sqlite3DbMallocZero(db, sizeof(TriggerPrg)));
  if (!pPrg)
    return 0;
  pPrg->pNext = pTop->pTriggerPrg;
  pTop->pTriggerPrg = pPrg;
  pPrg->pProgram = pProgram = (SubProgram*)(sqlite3DbMallocZero(db, sizeof(SubProgram)));
  if (!pProgram)
    return 0;
  sqlite3VdbeLinkSubProgram(pTop->pVdbe, pProgram);
  pPrg->pTrigger = pTrigger;
  pPrg->orconf = orconf;
  pPrg->aColmask[0] = 0xffffffff;
  pPrg->aColmask[1] = 0xffffffff;

  sqlite3ParseObjectInit(&sSubParse, db);
  memset(&sNC, 0, sizeof(sNC));
  sNC.pParse = &sSubParse;
  sSubParse.pTriggerTab = pTab;
  sSubParse.pToplevel = pTop;
  sSubParse.zAuthContext = pTrigger->zName;
  sSubParse.eTriggerOp = pTrigger->op;
  sSubParse.nQueryLoop = pParse->nQueryLoop;
  sSubParse.prepFlags = pParse->prepFlags;
  sSubParse.oldmask = 0;
  sSubParse.newmask = 0;

  v = sqlite3GetVdbe(&sSubParse);
  if (v) {
    if (pTrigger->zName) {
      sqlite3VdbeChangeP4(v, -1, sqlite3MPrintf(db, "-- TRIGGER %s", pTrigger->zName), (-7));
    }

    if (pTrigger->pWhen) {
      pWhen = sqlite3ExprDup(db, pTrigger->pWhen, 0);
      if (db->mallocFailed == 0 && SQLITE_OK == sqlite3ResolveExprNames(&sNC, pWhen)) {
        iEndTrigger = sqlite3VdbeMakeLabel(&sSubParse);
        sqlite3ExprIfFalse(&sSubParse, pWhen, iEndTrigger, 0x10);
      }
      sqlite3ExprDelete(db, pWhen);
    }

    codeTriggerProgram(&sSubParse, pTrigger->step_list, orconf);

    if (iEndTrigger) {
      sqlite3VdbeResolveLabel(v, iEndTrigger);
    }
    sqlite3VdbeAddOp0(v, 72);
    transferParseError(pParse, &sSubParse);

    if (pParse->nErr == 0) {
      pProgram->aOp = sqlite3VdbeTakeOpArray(v, &pProgram->nOp, &pTop->nMaxArg);
    }
    pProgram->nMem = sSubParse.nMem;
    pProgram->nCsr = sSubParse.nTab;
    pProgram->token = (void *)pTrigger;
    pPrg->aColmask[0] = sSubParse.oldmask;
    pPrg->aColmask[1] = sSubParse.newmask;
    sqlite3VdbeDelete(v);
  } else {
    transferParseError(pParse, &sSubParse);
  }

  sqlite3ParseObjectReset(&sSubParse);
  return pPrg;
}

TriggerPrg *getRowTrigger(Parse *pParse, Trigger *pTrigger, Table *pTab, int orconf) {
  Parse *pRoot = ((pParse)->pToplevel ? (pParse)->pToplevel : (pParse));
  TriggerPrg *pPrg;

  for (pPrg = pRoot->pTriggerPrg; pPrg && (pPrg->pTrigger != pTrigger || pPrg->orconf != orconf); pPrg = pPrg->pNext)
    ;

  if (!pPrg) {
    pPrg = codeRowTrigger(pParse, pTrigger, pTab, orconf);
    pParse->db->errByteOffset = -1;
  }

  return pPrg;
}

void sqlite3CodeRowTriggerDirect(Parse *pParse, Trigger *p, Table *pTab, int reg, int orconf, int ignoreJump) {
  Vdbe *v = sqlite3GetVdbe(pParse);
  TriggerPrg *pPrg;
  pPrg = getRowTrigger(pParse, p, pTab, orconf);

  if (pPrg) {
    int bRecursive = (p->zName && 0 == (pParse->db->flags & 0x00002000));

    sqlite3VdbeAddOp4(v, 50, reg, ignoreJump, ++pParse->nMem, (const char *)pPrg->pProgram, (-4));

    sqlite3VdbeChangeP5(v, (u16)bRecursive);
  }
}

void sqlite3CodeRowTrigger(Parse *pParse, Trigger *pTrigger, int op, ExprList *pChanges, int tr_tm, Table *pTab,
                           int reg, int orconf, int ignoreJump) {
  Trigger *p;

  for (p = pTrigger; p; p = p->pNext) {
    if ((p->op == op || (p->bReturning && p->op == 128 && op == 130)) && p->tr_tm == tr_tm &&
        checkColumnOverlap(p->pColumns, pChanges)) {
      if (!p->bReturning) {
        sqlite3CodeRowTriggerDirect(pParse, p, pTab, reg, orconf, ignoreJump);
      } else if ((pParse)->pToplevel == 0) {
        codeReturningTrigger(pParse, p, pTab, reg);
      }
    }
  }
}

u32 sqlite3TriggerColmask(Parse *pParse, Trigger *pTrigger, ExprList *pChanges, int isNew, int tr_tm, Table *pTab,
                          int orconf) {
  const int op = pChanges ? 130 : 129;
  u32 mask = 0;
  Trigger *p;

  if ((pTab)->eTabType == 2) {
    return 0xffffffff;
  }
  for (p = pTrigger; p; p = p->pNext) {
    if (p->op == op && (tr_tm & p->tr_tm) && checkColumnOverlap(p->pColumns, pChanges)) {
      if (p->bReturning) {
        mask = 0xffffffff;
      } else {
        TriggerPrg *pPrg;
        pPrg = getRowTrigger(pParse, p, pTab, orconf);
        if (pPrg) {
          mask |= pPrg->aColmask[isNew];
        }
      }
    }
  }

  return mask;
}

Expr *exprRowColumn(Parse *pParse, int iCol) {
  Expr *pRet = sqlite3PExpr(pParse, 76, 0, 0);
  if (pRet)
    pRet->iColumn = iCol + 1;
  return pRet;
}

void updateFromSelect(Parse *pParse, int iEph, Index *pPk, ExprList *pChanges, SrcList *pTabList, Expr *pWhere,
                      ExprList *pOrderBy, Expr *pLimit) {
  int i;
  SelectDest dest;
  Select *pSelect = 0;
  ExprList *pList = 0;
  ExprList *pGrp = 0;
  Expr *pLimit2 = 0;
  ExprList *pOrderBy2 = 0;
  sqlite3 *db = pParse->db;
  Table *pTab = pTabList->a[0].pSTab;
  SrcList *pSrc;
  Expr *pWhere2;
  int eDest;

  (void)(pOrderBy);
  (void)(pLimit);

  pSrc = sqlite3SrcListDup(db, pTabList, 0);
  pWhere2 = sqlite3ExprDup(db, pWhere, 0);

  if (pSrc) {
    pSrc->a[0].iCursor = -1;
    pSrc->a[0].pSTab->nTabRef--;
    pSrc->a[0].pSTab = 0;
  }
  if (pPk) {
    for (i = 0; i < pPk->nKeyCol; i++) {
      Expr *pNew = exprRowColumn(pParse, pPk->aiColumn[i]);

      pList = sqlite3ExprListAppend(pParse, pList, pNew);
    }
    eDest = ((pTab)->eTabType == 1) ? 12 : 13;
  } else if ((pTab)->eTabType == 2) {
    for (i = 0; i < pTab->nCol; i++) {
      pList = sqlite3ExprListAppend(pParse, pList, exprRowColumn(pParse, i));
    }
    eDest = 12;
  } else {
    eDest = ((pTab)->eTabType == 1) ? 12 : 13;
    pList = sqlite3ExprListAppend(pParse, 0, sqlite3PExpr(pParse, 76, 0, 0));
  }

  if (pChanges) {
    for (i = 0; i < pChanges->nExpr; i++) {
      pList = sqlite3ExprListAppend(pParse, pList, sqlite3ExprDup(db, pChanges->a[i].pExpr, 0));
    }
  }
  pSelect =
      sqlite3SelectNew(pParse, pList, pSrc, pWhere2, pGrp, 0, pOrderBy2, 0x0800000 | 0x0020000 | 0x10000000, pLimit2);
  if (pSelect)
    pSelect->selFlags |= 0x8000000;
  sqlite3SelectDestInit(&dest, eDest, iEph);
  dest.iSDParm2 = (pPk ? pPk->nKeyCol : -1);
  sqlite3Select(pParse, pSelect, &dest);
  sqlite3SelectDelete(db, pSelect);
}

void sqlite3Update(Parse *pParse, SrcList *pTabList, ExprList *pChanges, Expr *pWhere, int onError, ExprList *pOrderBy,
                   Expr *pLimit, Upsert *pUpsert) {
  int i, j, k;
  Table *pTab;
  int addrTop = 0;
  WhereInfo *pWInfo = 0;
  Vdbe *v;
  Index *pIdx;
  Index *pPk;
  int nIdx;
  int nAllIdx;
  int iBaseCur;
  int iDataCur;
  int iIdxCur;
  sqlite3 *db;
  int *aRegIdx = 0;
  int *aXRef = 0;

  u8 *aToOpen;
  u8 chngPk;
  u8 chngRowid;
  u8 chngKey;
  Expr *pRowidExpr = 0;
  int iRowidExpr = -1;
  AuthContext sContext;
  NameContext sNC;
  int iDb;
  int eOnePass;
  int hasFK;
  int labelBreak;
  int labelContinue;
  int flags;

  int isView;
  Trigger *pTrigger;
  int tmask;

  int newmask;
  int iEph = 0;
  int nKey = 0;
  int aiCurOnePass[2];
  int addrOpen = 0;
  int iPk = 0;
  i16 nPk = 0;
  int bReplace = 0;
  int bFinishSeek = 1;
  int nChangeFrom = 0;

  int regRowCount = 0;
  int regOldRowid = 0;
  int regNewRowid = 0;
  int regNew = 0;
  int regOld = 0;
  int regRowSet = 0;
  int regKey = 0;

  memset(&sContext, 0, sizeof(sContext));
  db = pParse->db;

  if (pParse->nErr) {
    goto update_cleanup;
  }

  pTab = sqlite3SrcListLookup(pParse, pTabList);
  if (pTab == 0)
    goto update_cleanup;
  iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);

  pTrigger = sqlite3TriggersExist(pParse, pTab, 130, pChanges, &tmask);
  isView = ((pTab)->eTabType == 2);

  nChangeFrom = (pTabList->nSrc > 1) ? pChanges->nExpr : 0;

  if (sqlite3ViewGetColumnNames(pParse, pTab)) {
    goto update_cleanup;
  }
  if (sqlite3IsReadOnly(pParse, pTab, pTrigger)) {
    goto update_cleanup;
  }

  iBaseCur = iDataCur = pParse->nTab++;
  iIdxCur = iDataCur + 1;
  pPk = (((pTab)->tabFlags & 0x00000080) == 0) ? 0 : sqlite3PrimaryKeyIndex(pTab);
  for (nIdx = 0, pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext, nIdx++) {
    if (pPk == pIdx) {
      iDataCur = pParse->nTab;
    }
    pParse->nTab++;
  }
  if (pUpsert) {
    iDataCur = pUpsert->iDataCur;
    iIdxCur = pUpsert->iIdxCur;
    pParse->nTab = iBaseCur;
  }
  pTabList->a[0].iCursor = iDataCur;

  aXRef = (int*)(sqlite3DbMallocRawNN(db, sizeof(int) * (pTab->nCol + nIdx + 1) + nIdx + 2));
  if (aXRef == 0)
    goto update_cleanup;
  aRegIdx = aXRef + pTab->nCol;
  aToOpen = (u8 *)(aRegIdx + nIdx + 1);
  memset(aToOpen, 1, nIdx + 1);
  aToOpen[nIdx + 1] = 0;
  for (i = 0; i < pTab->nCol; i++)
    aXRef[i] = -1;

  memset(&sNC, 0, sizeof(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  sNC.uNC.pUpsert = pUpsert;
  sNC.ncFlags = 0x000200;

  v = sqlite3GetVdbe(pParse);
  if (v == 0)
    goto update_cleanup;

  chngRowid = chngPk = 0;
  for (i = 0; i < pChanges->nExpr; i++) {
    if (nChangeFrom == 0 && sqlite3ResolveExprNames(&sNC, pChanges->a[i].pExpr)) {
      goto update_cleanup;
    }
    j = sqlite3ColumnIndex(pTab, pChanges->a[i].zEName);
    if (j >= 0) {
      if (j == pTab->iPKey) {
        chngRowid = 1;
        pRowidExpr = pChanges->a[i].pExpr;
        iRowidExpr = i;
      } else if (pPk && (pTab->aCol[j].colFlags & 0x0001) != 0) {
        chngPk = 1;
      }

      else if (pTab->aCol[j].colFlags & 0x0060) {
        sqlite3ErrorMsg(pParse, "cannot UPDATE generated column \"%s\"", pTab->aCol[j].zCnName);
        goto update_cleanup;
      }

      aXRef[j] = i;
    } else {
      if (pPk == 0 && sqlite3IsRowid(pChanges->a[i].zEName)) {
        j = -1;
        chngRowid = 1;
        pRowidExpr = pChanges->a[i].pExpr;
        iRowidExpr = i;
      } else {
        sqlite3ErrorMsg(pParse, "no such column: %s", pChanges->a[i].zEName);
        pParse->checkSchema = 1;
        goto update_cleanup;
      }
    }

    {
      int rc;
      rc = sqlite3AuthCheck(pParse, SQLITE_UPDATE, pTab->zName, j < 0 ? "ROWID" : pTab->aCol[j].zCnName,
                            db->aDb[iDb].zDbSName);
      if (rc == SQLITE_DENY) {
        goto update_cleanup;
      } else if (rc == SQLITE_IGNORE) {
        aXRef[j] = -1;
      }
    }
  }

  chngKey = chngRowid + chngPk;

  if (pTab->tabFlags & 0x00000060) {
    int bProgress;
    do {
      bProgress = 0;
      for (i = 0; i < pTab->nCol; i++) {
        if (aXRef[i] >= 0)
          continue;
        if ((pTab->aCol[i].colFlags & 0x0060) == 0)
          continue;
        if (sqlite3ExprReferencesUpdatedColumn(sqlite3ColumnExpr(pTab, &pTab->aCol[i]), aXRef, chngRowid)) {
          aXRef[i] = 99999;
          bProgress = 1;
        }
      }
    } while (bProgress);
  }

  pTabList->a[0].colUsed = ((pTab)->eTabType == 1) ? ((Bitmask)-1) : 0;

  hasFK = sqlite3FkRequired(pParse, pTab, aXRef, chngKey);

  if (onError == 5)
    bReplace = 1;
  for (nAllIdx = 0, pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext, nAllIdx++) {
    int reg;
    if (chngKey || hasFK > 1 || pIdx == pPk || indexWhereClauseMightChange(pIdx, aXRef, chngRowid)) {
      reg = ++pParse->nMem;
      pParse->nMem += pIdx->nColumn;
    } else {
      reg = 0;
      for (i = 0; i < pIdx->nKeyCol; i++) {
        if (indexColumnIsBeingUpdated(pIdx, i, aXRef, chngRowid)) {
          reg = ++pParse->nMem;
          pParse->nMem += pIdx->nColumn;
          if (onError == 11 && pIdx->onError == 5) {
            bReplace = 1;
          }
          break;
        }
      }
    }
    if (reg == 0)
      aToOpen[nAllIdx + 1] = 0;
    aRegIdx[nAllIdx] = reg;
  }
  aRegIdx[nAllIdx] = ++pParse->nMem;
  if (bReplace) {
    memset(aToOpen, 1, nIdx + 1);
  }

  if (pParse->nested == 0)
    sqlite3VdbeCountChanges(v);
  sqlite3BeginWriteOperation(pParse, pTrigger || hasFK, iDb);

  if (!((pTab)->eTabType == 1)) {
    regRowSet = aRegIdx[nAllIdx];
    regOldRowid = regNewRowid = ++pParse->nMem;
    if (chngPk || pTrigger || hasFK) {
      regOld = pParse->nMem + 1;
      pParse->nMem += pTab->nCol;
    }
    if (chngKey || pTrigger || hasFK) {
      regNewRowid = ++pParse->nMem;
    }
    regNew = pParse->nMem + 1;
    pParse->nMem += pTab->nCol;
  }

  if (isView) {
    sqlite3AuthContextPush(pParse, &sContext, pTab->zName);
  }

  if (nChangeFrom == 0 && isView) {
    sqlite3MaterializeView(pParse, pTab, pWhere, pOrderBy, pLimit, iDataCur);
    pOrderBy = 0;
    pLimit = 0;
  }

  if (nChangeFrom == 0 && sqlite3ResolveExprNames(&sNC, pWhere)) {
    goto update_cleanup;
  }

  if ((pTab)->eTabType == 1) {
    updateVirtualTable(pParse, pTabList, pTab, pChanges, pRowidExpr, aXRef, pWhere, onError);
    goto update_cleanup;
  }

  labelContinue = labelBreak = sqlite3VdbeMakeLabel(pParse);

  if ((db->flags & ((u64)(0x00001) << 32)) != 0 && !pParse->pTriggerTab && !pParse->nested && !pParse->bReturning &&
      pUpsert == 0) {
    regRowCount = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, 73, 0, regRowCount);
  }

  if (nChangeFrom == 0 && (((pTab)->tabFlags & 0x00000080) == 0)) {
    sqlite3VdbeAddOp3(v, 77, 0, regRowSet, regOldRowid);
    iEph = pParse->nTab++;
    addrOpen = sqlite3VdbeAddOp3(v, 120, iEph, 0, regRowSet);
  } else {
    nPk = pPk ? pPk->nKeyCol : 0;
    iPk = pParse->nMem + 1;
    pParse->nMem += nPk;
    pParse->nMem += nChangeFrom;
    regKey = ++pParse->nMem;
    if (pUpsert == 0) {
      int nEphCol = nPk + nChangeFrom + (isView ? pTab->nCol : 0);
      iEph = pParse->nTab++;
      if (pPk)
        sqlite3VdbeAddOp3(v, 77, 0, iPk, iPk + nPk - 1);
      addrOpen = sqlite3VdbeAddOp2(v, 120, iEph, nEphCol);
      if (pPk) {
        KeyInfo *pKeyInfo = sqlite3KeyInfoOfIndex(pParse, pPk);
        if (pKeyInfo) {
          pKeyInfo->nAllField = nEphCol;
          sqlite3VdbeAppendP4(v, pKeyInfo, (-9));
        }
      }
      if (nChangeFrom) {
        updateFromSelect(pParse, iEph, pPk, pChanges, pTabList, pWhere, pOrderBy, pLimit);

        if (isView)
          iDataCur = iEph;
      }
    }
  }

  if (nChangeFrom) {
    sqlite3MultiWrite(pParse);
    eOnePass = 0;
    nKey = nPk;
    regKey = iPk;
  } else {
    if (pUpsert) {
      pWInfo = 0;
      eOnePass = 1;
      sqlite3ExprIfFalse(pParse, pWhere, labelBreak, 0x10);
      bFinishSeek = 0;
    } else {
      flags = 0x0004;
      if (!pParse->nested && !pTrigger && !hasFK && !chngKey && !bReplace &&
          (pWhere == 0 || !(((pWhere)->flags & (u32)(0x400000)) != 0))) {
        flags |= 0x0008;
      }
      pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, 0, 0, 0, flags, iIdxCur);
      if (pWInfo == 0)
        goto update_cleanup;

      eOnePass = sqlite3WhereOkOnePass(pWInfo, aiCurOnePass);
      bFinishSeek = sqlite3WhereUsesDeferredSeek(pWInfo);
      if (eOnePass != 1) {
        sqlite3MultiWrite(pParse);
        if (eOnePass == 2) {
          int iCur = aiCurOnePass[1];
          if (iCur >= 0 && iCur != iDataCur && aToOpen[iCur - iBaseCur]) {
            eOnePass = 0;
          }
        }
      }
    }

    if ((((pTab)->tabFlags & 0x00000080) == 0)) {
      sqlite3VdbeAddOp2(v, 137, iDataCur, regOldRowid);
      if (eOnePass == 0) {
        aRegIdx[nAllIdx] = ++pParse->nMem;
        sqlite3VdbeAddOp3(v, 130, iEph, regRowSet, regOldRowid);
      } else {
        if ((addrOpen))
          sqlite3VdbeChangeToNoop(v, addrOpen);
      }
    } else {
      for (i = 0; i < nPk; i++) {
        sqlite3ExprCodeGetColumnOfTable(v, pTab, iDataCur, pPk->aiColumn[i], iPk + i);
      }
      if (eOnePass) {
        if (addrOpen)
          sqlite3VdbeChangeToNoop(v, addrOpen);
        nKey = nPk;
        regKey = iPk;
      } else {
        sqlite3VdbeAddOp4(v, 99, iPk, nPk, regKey, sqlite3IndexAffinityStr(db, pPk), nPk);
        sqlite3VdbeAddOp4Int(v, 140, iEph, regKey, iPk, nPk);
      }
    }
  }

  if (pUpsert == 0) {
    if (nChangeFrom == 0 && eOnePass != 2) {
      sqlite3WhereEnd(pWInfo);
    }

    if (!isView) {
      int addrOnce = 0;
      int iNotUsed1 = 0;
      int iNotUsed2 = 0;

      if (eOnePass != 0) {
        if (aiCurOnePass[0] >= 0)
          aToOpen[aiCurOnePass[0] - iBaseCur] = 0;
        if (aiCurOnePass[1] >= 0)
          aToOpen[aiCurOnePass[1] - iBaseCur] = 0;
      }

      if (eOnePass == 2 && (nIdx - (aiCurOnePass[1] >= 0)) > 0) {
        addrOnce = sqlite3VdbeAddOp0(v, 15);
      }
      sqlite3OpenTableAndIndices(pParse, pTab, 116, 0, iBaseCur, aToOpen, &iNotUsed1, &iNotUsed2);
      if (addrOnce) {
        sqlite3VdbeJumpHereOrPopInst(v, addrOnce);
      }
    }

    if (eOnePass != 0) {
      if (aiCurOnePass[0] != iDataCur && aiCurOnePass[1] != iDataCur) {
        sqlite3VdbeAddOp4Int(v, 28, iDataCur, labelBreak, regKey, nKey);
      }
      if (eOnePass != 1) {
        labelContinue = sqlite3VdbeMakeLabel(pParse);
      }
      sqlite3VdbeAddOp2(v, 51, pPk ? regKey : regOldRowid, labelBreak);
    } else if (pPk || nChangeFrom) {
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      sqlite3VdbeAddOp2(v, 36, iEph, labelBreak);
      addrTop = sqlite3VdbeCurrentAddr(v);
      if (nChangeFrom) {
        if (!isView) {
          if (pPk) {
            for (i = 0; i < nPk; i++) {
              sqlite3VdbeAddOp3(v, 96, iEph, i, iPk + i);
            }
            sqlite3VdbeAddOp4Int(v, 28, iDataCur, labelContinue, iPk, nPk);
          } else {
            sqlite3VdbeAddOp2(v, 137, iEph, regOldRowid);
            sqlite3VdbeAddOp3(v, 31, iDataCur, labelContinue, regOldRowid);
          }
        }
      } else {
        sqlite3VdbeAddOp2(v, 136, iEph, regKey);
        sqlite3VdbeAddOp4Int(v, 28, iDataCur, labelContinue, regKey, 0);
      }
    } else {
      sqlite3VdbeAddOp2(v, 36, iEph, labelBreak);
      labelContinue = sqlite3VdbeMakeLabel(pParse);
      addrTop = sqlite3VdbeAddOp2(v, 137, iEph, regOldRowid);
      sqlite3VdbeAddOp3(v, 31, iDataCur, labelContinue, regOldRowid);
    }
  }

  if (chngRowid) {
    if (nChangeFrom == 0) {
      sqlite3ExprCode(pParse, pRowidExpr, regNewRowid);
    } else {
      sqlite3VdbeAddOp3(v, 96, iEph, iRowidExpr, regNewRowid);
    }
    sqlite3VdbeAddOp1(v, 13, regNewRowid);
  }

  if (chngPk || hasFK || pTrigger) {
    u32 oldmask = (hasFK ? sqlite3FkOldmask(pParse, pTab) : 0);
    oldmask |= sqlite3TriggerColmask(pParse, pTrigger, pChanges, 0, 1 | 2, pTab, onError);
    for (i = 0; i < pTab->nCol; i++) {
      u32 colFlags = pTab->aCol[i].colFlags;
      k = sqlite3TableColumnToStorage(pTab, i) + regOld;
      if (oldmask == 0xffffffff || (i < 32 && (oldmask & (((unsigned int)1) << (i))) != 0) ||
          (colFlags & 0x0001) != 0) {
        sqlite3ExprCodeGetColumnOfTable(v, pTab, iDataCur, i, k);
      } else {
        sqlite3VdbeAddOp2(v, 77, 0, k);
      }
    }
    if (chngRowid == 0 && pPk == 0) {
      sqlite3VdbeAddOp2(v, 82, regOldRowid, regNewRowid);
    }
  }

  newmask = sqlite3TriggerColmask(pParse, pTrigger, pChanges, 1, 1, pTab, onError);
  for (i = 0, k = regNew; i < pTab->nCol; i++, k++) {
    if (i == pTab->iPKey) {
      sqlite3VdbeAddOp2(v, 77, 0, k);
    } else if ((pTab->aCol[i].colFlags & 0x0060) != 0) {
      if (pTab->aCol[i].colFlags & 0x0020)
        k--;
    } else {
      j = aXRef[i];
      if (j >= 0) {
        if (nChangeFrom) {
          int nOff = (isView ? pTab->nCol : nPk);

          sqlite3VdbeAddOp3(v, 96, iEph, nOff + j, k);
        } else {
          sqlite3ExprCode(pParse, pChanges->a[j].pExpr, k);
        }
      } else if (0 == (tmask & 1) || i > 31 || (newmask & (((unsigned int)1) << (i)))) {
        sqlite3ExprCodeGetColumnOfTable(v, pTab, iDataCur, i, k);
        bFinishSeek = 0;
      } else {
        sqlite3VdbeAddOp2(v, 77, 0, k);
      }
    }
  }

  if (pTab->tabFlags & 0x00000060) {
    sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
  }

  if (tmask & 1) {
    sqlite3TableAffinity(v, pTab, regNew);
    sqlite3CodeRowTrigger(pParse, pTrigger, 130, pChanges, 1, pTab, regOldRowid, onError, labelContinue);

    if (!isView) {
      if (pPk) {
        sqlite3VdbeAddOp4Int(v, 28, iDataCur, labelContinue, regKey, nKey);
      } else {
        sqlite3VdbeAddOp3(v, 31, iDataCur, labelContinue, regOldRowid);
      }

      for (i = 0, k = regNew; i < pTab->nCol; i++, k++) {
        if (pTab->aCol[i].colFlags & 0x0060) {
          if (pTab->aCol[i].colFlags & 0x0020)
            k--;
        } else if (aXRef[i] < 0 && i != pTab->iPKey) {
          sqlite3ExprCodeGetColumnOfTable(v, pTab, iDataCur, i, k);
        }
      }

      if (pTab->tabFlags & 0x00000060) {
        sqlite3ComputeGeneratedColumns(pParse, regNew, pTab);
      }
    }
  }

  if (!isView) {
    sqlite3GenerateConstraintChecks(pParse, pTab, aRegIdx, iDataCur, iIdxCur, regNewRowid, regOldRowid, chngKey,
                                    onError, labelContinue, &bReplace, aXRef, 0);

    if (bReplace || chngKey) {
      if (pPk) {
        sqlite3VdbeAddOp4Int(v, 28, iDataCur, labelContinue, regKey, nKey);
      } else {
        sqlite3VdbeAddOp3(v, 31, iDataCur, labelContinue, regOldRowid);
      };
    }

    if (hasFK) {
      sqlite3FkCheck(pParse, pTab, regOldRowid, 0, aXRef, chngKey);
    }

    sqlite3GenerateRowIndexDelete(pParse, pTab, iDataCur, iIdxCur, aRegIdx, -1);

    if (bFinishSeek) {
      sqlite3VdbeAddOp1(v, 145, iDataCur);
    }

    if (hasFK > 1 || chngKey) {
      sqlite3VdbeAddOp2(v, 132, iDataCur, 0);
    }

    if (hasFK) {
      sqlite3FkCheck(pParse, pTab, 0, regNewRowid, aXRef, chngKey);
    }

    sqlite3CompleteInsertion(pParse, pTab, iDataCur, iIdxCur, regNewRowid, aRegIdx, 0x04 | (eOnePass == 2 ? 0x02 : 0),
                             0, 0);

    if (hasFK) {
      sqlite3FkActions(pParse, pTab, pChanges, regOldRowid, aXRef, chngKey);
    }
  }

  if (regRowCount) {
    sqlite3VdbeAddOp2(v, 88, regRowCount, 1);
  }

  if (pTrigger) {
    sqlite3CodeRowTrigger(pParse, pTrigger, 130, pChanges, 2, pTab, regOldRowid, onError, labelContinue);
  }

  if (eOnePass == 1) {
  } else if (eOnePass == 2) {
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3WhereEnd(pWInfo);
  } else {
    sqlite3VdbeResolveLabel(v, labelContinue);
    sqlite3VdbeAddOp2(v, 40, iEph, addrTop);
  }
  sqlite3VdbeResolveLabel(v, labelBreak);

  if (pParse->nested == 0 && pParse->pTriggerTab == 0 && pUpsert == 0) {
    sqlite3AutoincrementEnd(pParse);
  }

  if (regRowCount) {
    sqlite3CodeChangeCount(v, regRowCount, "rows updated");
  }

update_cleanup:
  sqlite3AuthContextPop(&sContext);
  sqlite3DbFree(db, aXRef);
  sqlite3SrcListDelete(db, pTabList);
  sqlite3ExprListDelete(db, pChanges);
  sqlite3ExprDelete(db, pWhere);

  return;
}

void updateVirtualTable(Parse *pParse, SrcList *pSrc, Table *pTab, ExprList *pChanges, Expr *pRowid, int *aXRef,
                        Expr *pWhere, int onError) {
  Vdbe *v = pParse->pVdbe;
  int ephemTab;
  int i;
  sqlite3 *db = pParse->db;
  const char *pVTab = (const char *)sqlite3GetVTable(db, pTab);
  WhereInfo *pWInfo = 0;
  int nArg = 2 + pTab->nCol;
  int regArg;
  int regRec;
  int regRowid;
  int iCsr = pSrc->a[0].iCursor;
  int aDummy[2];
  int eOnePass;
  int addr;

  ephemTab = pParse->nTab++;
  addr = sqlite3VdbeAddOp2(v, 120, ephemTab, nArg);
  regArg = pParse->nMem + 1;
  pParse->nMem += nArg;
  if (pSrc->nSrc > 1) {
    Index *pPk = 0;
    Expr *pRow;
    ExprList *pList;
    if ((((pTab)->tabFlags & 0x00000080) == 0)) {
      if (pRowid) {
        pRow = sqlite3ExprDup(db, pRowid, 0);
      } else {
        pRow = sqlite3PExpr(pParse, 76, 0, 0);
      }
    } else {
      i16 iPk;
      pPk = sqlite3PrimaryKeyIndex(pTab);

      iPk = pPk->aiColumn[0];
      if (aXRef[iPk] >= 0) {
        pRow = sqlite3ExprDup(db, pChanges->a[aXRef[iPk]].pExpr, 0);
      } else {
        pRow = exprRowColumn(pParse, iPk);
      }
    }
    pList = sqlite3ExprListAppend(pParse, 0, pRow);

    for (i = 0; i < pTab->nCol; i++) {
      if (aXRef[i] >= 0) {
        pList = sqlite3ExprListAppend(pParse, pList, sqlite3ExprDup(db, pChanges->a[aXRef[i]].pExpr, 0));
      } else {
        Expr *pRowExpr = exprRowColumn(pParse, i);
        if (pRowExpr)
          pRowExpr->op2 = 0x01;
        pList = sqlite3ExprListAppend(pParse, pList, pRowExpr);
      }
    }

    updateFromSelect(pParse, ephemTab, pPk, pList, pSrc, pWhere, 0, 0);
    sqlite3ExprListDelete(db, pList);
    eOnePass = 0;
  } else {
    regRec = ++pParse->nMem;
    regRowid = ++pParse->nMem;

    pWInfo = sqlite3WhereBegin(pParse, pSrc, pWhere, 0, 0, 0, 0x0004, 0);
    if (pWInfo == 0)
      return;

    for (i = 0; i < pTab->nCol; i++) {
      if (aXRef[i] >= 0) {
        sqlite3ExprCode(pParse, pChanges->a[aXRef[i]].pExpr, regArg + 2 + i);
      } else {
        sqlite3VdbeAddOp3(v, 178, iCsr, i, regArg + 2 + i);
        sqlite3VdbeChangeP5(v, 0x01);
      }
    }
    if ((((pTab)->tabFlags & 0x00000080) == 0)) {
      sqlite3VdbeAddOp2(v, 137, iCsr, regArg);
      if (pRowid) {
        sqlite3ExprCode(pParse, pRowid, regArg + 1);
      } else {
        sqlite3VdbeAddOp2(v, 137, iCsr, regArg + 1);
      }
    } else {
      Index *pPk;
      i16 iPk;
      pPk = sqlite3PrimaryKeyIndex(pTab);

      iPk = pPk->aiColumn[0];
      sqlite3VdbeAddOp3(v, 178, iCsr, iPk, regArg);
      sqlite3VdbeAddOp2(v, 83, regArg + 2 + iPk, regArg + 1);
    }

    eOnePass = sqlite3WhereOkOnePass(pWInfo, aDummy);

    if (eOnePass) {
      sqlite3VdbeChangeToNoop(v, addr);
      sqlite3VdbeAddOp1(v, 124, iCsr);
    } else {
      sqlite3MultiWrite(pParse);
      sqlite3VdbeAddOp3(v, 99, regArg, nArg, regRec);

      sqlite3VdbeAddOp2(v, 129, ephemTab, regRowid);
      sqlite3VdbeAddOp3(v, 130, ephemTab, regRec, regRowid);
    }
  }

  if (eOnePass == 0) {
    if (pSrc->nSrc == 1) {
      sqlite3WhereEnd(pWInfo);
    }

    addr = sqlite3VdbeAddOp1(v, 36, ephemTab);

    for (i = 0; i < nArg; i++) {
      sqlite3VdbeAddOp3(v, 96, ephemTab, i, regArg + i);
    }
  }
  sqlite3VtabMakeWritable(pParse, pTab);
  sqlite3VdbeAddOp4(v, 7, 0, nArg, regArg, pVTab, (-12));
  sqlite3VdbeChangeP5(v, onError == 11 ? 2 : onError);
  sqlite3MayAbort(pParse);

  if (eOnePass == 0) {
    sqlite3VdbeAddOp2(v, 40, ephemTab, addr + 1);
    sqlite3VdbeJumpHere(v, addr);
    sqlite3VdbeAddOp2(v, 124, ephemTab, 0);
  } else {
    sqlite3WhereEnd(pWInfo);
  }
}

int sqlite3UpsertAnalyzeTarget(Parse *pParse, SrcList *pTabList, Upsert *pUpsert, Upsert *pAll) {
  Table *pTab;
  int rc;
  int iCursor;
  Index *pIdx;
  ExprList *pTarget;
  Expr *pTerm;
  NameContext sNC;
  Expr sCol[2];
  int nClause = 0;

  memset(&sNC, 0, sizeof(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  for (; pUpsert && pUpsert->pUpsertTarget; pUpsert = pUpsert->pNextUpsert, nClause++) {
    rc = sqlite3ResolveExprListNames(&sNC, pUpsert->pUpsertTarget);
    if (rc)
      return rc;
    rc = sqlite3ResolveExprNames(&sNC, pUpsert->pUpsertTargetWhere);
    if (rc)
      return rc;

    pTab = pTabList->a[0].pSTab;
    pTarget = pUpsert->pUpsertTarget;
    iCursor = pTabList->a[0].iCursor;
    if ((((pTab)->tabFlags & 0x00000080) == 0) && pTarget->nExpr == 1 && (pTerm = pTarget->a[0].pExpr)->op == 168 &&
        pTerm->iColumn == (-1)) {
      continue;
    }

    memset(sCol, 0, sizeof(sCol));
    sCol[0].op = 114;
    sCol[0].pLeft = &sCol[1];
    sCol[1].op = 168;
    sCol[1].iTable = pTabList->a[0].iCursor;

    for (pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext) {
      int ii, jj, nn;
      if (!((pIdx)->onError != 0))
        continue;
      if (pTarget->nExpr != pIdx->nKeyCol)
        continue;
      if (pIdx->pPartIdxWhere) {
        if (pUpsert->pUpsertTargetWhere == 0)
          continue;
        if (sqlite3ExprCompare(pParse, pUpsert->pUpsertTargetWhere, pIdx->pPartIdxWhere, iCursor) != 0) {
          continue;
        }
      }
      nn = pIdx->nKeyCol;
      for (ii = 0; ii < nn; ii++) {
        Expr *pExpr;
        sCol[0].u.zToken = (char *)pIdx->azColl[ii];
        if (pIdx->aiColumn[ii] == (-2)) {
          pExpr = pIdx->aColExpr->a[ii].pExpr;
          if (pExpr->op != 114) {
            sCol[0].pLeft = pExpr;
            pExpr = &sCol[0];
          }
        } else {
          sCol[0].pLeft = &sCol[1];
          sCol[1].iColumn = pIdx->aiColumn[ii];
          pExpr = &sCol[0];
        }
        for (jj = 0; jj < nn; jj++) {
          if (sqlite3ExprCompare(0, pTarget->a[jj].pExpr, pExpr, iCursor) < 2) {
            break;
          }
        }
        if (jj >= nn) {
          break;
        }
      }
      if (ii < nn) {
        continue;
      }
      pUpsert->pUpsertIdx = pIdx;
      if (sqlite3UpsertOfIndex(pAll, pIdx) != pUpsert) {
        pUpsert->isDup = 1;
      }
      break;
    }
    if (pUpsert->pUpsertIdx == 0) {
      char zWhich[16];
      if (nClause == 0 && pUpsert->pNextUpsert == 0) {
        zWhich[0] = 0;
      } else {
        sqlite3_snprintf(sizeof(zWhich), zWhich, "%r ", nClause + 1);
      }
      sqlite3ErrorMsg(pParse,
                      "%sON CONFLICT clause does not match any "
                      "PRIMARY KEY or UNIQUE constraint",
                      zWhich);
      return SQLITE_ERROR;
    }
  }
  return SQLITE_OK;
}

void sqlite3UpsertDoUpdate(Parse *pParse, Upsert *pUpsert, Table *pTab, Index *pIdx, int iCur) {
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  SrcList *pSrc;
  int iDataCur;
  int i;
  Upsert *pTop = pUpsert;

  iDataCur = pUpsert->iDataCur;
  pUpsert = sqlite3UpsertOfIndex(pTop, pIdx);
  if (pIdx && iCur != iDataCur) {
    if ((((pTab)->tabFlags & 0x00000080) == 0)) {
      int regRowid = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp2(v, 144, iCur, regRowid);
      sqlite3VdbeAddOp3(v, 30, iDataCur, 0, regRowid);
      sqlite3ReleaseTempReg(pParse, regRowid);
    } else {
      Index *pPk = sqlite3PrimaryKeyIndex(pTab);
      int nPk = pPk->nKeyCol;
      int iPk = pParse->nMem + 1;
      pParse->nMem += nPk;
      for (i = 0; i < nPk; i++) {
        int k;

        k = sqlite3TableColumnToIndex(pIdx, pPk->aiColumn[i]);
        sqlite3VdbeAddOp3(v, 96, iCur, k, iPk + i);
      };
      i = sqlite3VdbeAddOp4Int(v, 29, iDataCur, 0, iPk, nPk);
      sqlite3VdbeAddOp4(v, 72, SQLITE_CORRUPT, 2, 0, "corrupt database", (-1));
      sqlite3MayAbort(pParse);
      sqlite3VdbeJumpHere(v, i);
    }
  }

  pSrc = sqlite3SrcListDup(db, pTop->pUpsertSrc, 0);

  for (i = 0; i < pTab->nCol; i++) {
    if (pTab->aCol[i].affinity == 0x45) {
      int iStorage = pTop->regData + sqlite3TableColumnToStorage(pTab, i);
      sqlite3VdbeAddOp1(v, 89, iStorage);
    }
  }
  sqlite3Update(pParse, pSrc, sqlite3ExprListDup(db, pUpsert->pUpsertSet, 0),
                sqlite3ExprDup(db, pUpsert->pUpsertWhere, 0), 2, 0, 0, pUpsert);
}

void sqlite3Vacuum(Parse *pParse, Token *pNm, Expr *pInto) {
  Vdbe *v = sqlite3GetVdbe(pParse);
  int iDb = 0;
  if (v == 0)
    goto build_vacuum_end;
  if (pParse->nErr)
    goto build_vacuum_end;
  if (pNm) {
    iDb = sqlite3TwoPartName(pParse, pNm, pNm, &pNm);
    if (iDb < 0)
      goto build_vacuum_end;
  }
  if (iDb != 1) {
    int iIntoReg = 0;
    if (pInto && sqlite3ResolveSelfReference(pParse, 0, 0, pInto, 0) == 0) {
      iIntoReg = ++pParse->nMem;
      sqlite3ExprCode(pParse, pInto, iIntoReg);
    }
    sqlite3VdbeAddOp2(v, 5, iDb, iIntoReg);
    sqlite3VdbeUsesBtree(v, iDb);
  }
build_vacuum_end:
  sqlite3ExprDelete(pParse->db, pInto);
  return;
}

void addModuleArgument(Parse *pParse, Table *pTable, char *zArg) {
  sqlite3_int64 nBytes;
  char **azModuleArg;
  sqlite3 *db = pParse->db;

  nBytes = sizeof(char *) * (2 + pTable->u.vtab.nArg);
  if (pTable->u.vtab.nArg + 3 >= db->aLimit[SQLITE_LIMIT_COLUMN]) {
    sqlite3ErrorMsg(pParse, "too many columns on %s", pTable->zName);
  }
  azModuleArg = (char**)(sqlite3DbRealloc(db, pTable->u.vtab.azArg, nBytes));
  if (azModuleArg == 0) {
    sqlite3DbFree(db, zArg);
  } else {
    int i = pTable->u.vtab.nArg++;
    azModuleArg[i] = zArg;
    azModuleArg[i + 1] = 0;
    pTable->u.vtab.azArg = azModuleArg;
  }
}

void sqlite3VtabBeginParse(Parse *pParse, Token *pName1, Token *pName2, Token *pModuleName, int ifNotExists) {
  Table *pTable;
  sqlite3 *db;

  sqlite3StartTable(pParse, pName1, pName2, 0, 0, 1, ifNotExists);
  pTable = pParse->pNewTable;
  if (pTable == 0)
    return;

  pTable->eTabType = 1;

  db = pParse->db;

  addModuleArgument(pParse, pTable, sqlite3NameFromToken(db, pModuleName));
  addModuleArgument(pParse, pTable, 0);
  addModuleArgument(pParse, pTable, sqlite3DbStrDup(db, pTable->zName));

  pParse->sNameToken.n = (int)(&pModuleName->z[pModuleName->n] - pParse->sNameToken.z);

  if (pTable->u.vtab.azArg) {
    int iDb = sqlite3SchemaToIndex(db, pTable->pSchema);

    sqlite3AuthCheck(pParse, SQLITE_CREATE_VTABLE, pTable->zName, pTable->u.vtab.azArg[0],
                     pParse->db->aDb[iDb].zDbSName);
  }
}

void addArgumentToVtab(Parse *pParse) {
  if (pParse->sArg.z && pParse->pNewTable) {
    const char *z = (const char *)pParse->sArg.z;
    int n = pParse->sArg.n;
    sqlite3 *db = pParse->db;
    addModuleArgument(pParse, pParse->pNewTable, sqlite3DbStrNDup(db, z, n));
  }
}

void sqlite3VtabFinishParse(Parse *pParse, Token *pEnd) {
  Table *pTab = pParse->pNewTable;
  sqlite3 *db = pParse->db;

  if (pTab == 0)
    return;

  addArgumentToVtab(pParse);
  pParse->sArg.z = 0;
  if (pTab->u.vtab.nArg < 1)
    return;

  if (!db->init.busy) {
    char *zStmt;
    char *zWhere;
    int iDb;
    int iReg;
    Vdbe *v;

    sqlite3MayAbort(pParse);

    if (pEnd) {
      pParse->sNameToken.n = (int)(pEnd->z - pParse->sNameToken.z) + pEnd->n;
    }
    zStmt = sqlite3MPrintf(db, "CREATE VIRTUAL TABLE %T", &pParse->sNameToken);

    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);

    sqlite3NestedParse(pParse,
                       "UPDATE %Q."
                       "sqlite_master"
                       " "
                       "SET type='table', name=%Q, tbl_name=%Q, rootpage=0, sql=%Q "
                       "WHERE rowid=#%d",
                       db->aDb[iDb].zDbSName, pTab->zName, pTab->zName, zStmt, pParse->u1.cr.regRowid);
    v = sqlite3GetVdbe(pParse);
    sqlite3ChangeCookie(pParse, iDb);

    sqlite3VdbeAddOp0(v, 168);
    zWhere = sqlite3MPrintf(db, "name=%Q AND sql=%Q", pTab->zName, zStmt);
    sqlite3VdbeAddParseSchemaOp(v, iDb, zWhere, 0);
    sqlite3DbFree(db, zStmt);

    iReg = ++pParse->nMem;
    sqlite3VdbeLoadString(v, iReg, pTab->zName);
    sqlite3VdbeAddOp2(v, 173, iDb, iReg);
  } else {
    Table *pOld;
    Schema *pSchema = pTab->pSchema;
    const char *zName = pTab->zName;

    sqlite3MarkAllShadowTablesOf(db, pTab);
    pOld = (Table*)(sqlite3HashInsert(&pSchema->tblHash, zName, pTab));
    if (pOld) {
      sqlite3OomFault(db);

      return;
    }
    pParse->pNewTable = 0;
  }
}

void sqlite3VtabArgInit(Parse *pParse) {
  addArgumentToVtab(pParse);
  pParse->sArg.z = 0;
  pParse->sArg.n = 0;
}

void sqlite3VtabArgExtend(Parse *pParse, Token *p) {
  Token *pArg = &pParse->sArg;
  if (pArg->z == 0) {
    pArg->z = p->z;
    pArg->n = p->n;
  } else {
    pArg->n = (int)(&p->z[p->n] - pArg->z);
  }
}

int sqlite3VtabCallConnect(Parse *pParse, Table *pTab) {
  sqlite3 *db = pParse->db;
  const char *zMod;
  Module *pMod;
  int rc;

  if (sqlite3GetVTable(db, pTab)) {
    return SQLITE_OK;
  }

  zMod = pTab->u.vtab.azArg[0];
  pMod = (Module *)sqlite3HashFind(&db->aModule, zMod);

  if (!pMod) {
    const char *zModule = pTab->u.vtab.azArg[0];
    sqlite3ErrorMsg(pParse, "no such module: %s", zModule);
    rc = SQLITE_ERROR;
  } else {
    char *zErr = 0;
    rc = vtabCallConstructor(db, pTab, pMod, pMod->pModule->xConnect, &zErr);
    if (rc != SQLITE_OK) {
      sqlite3ErrorMsg(pParse, "%s", zErr);
      pParse->rc = rc;
    }
    sqlite3DbFree(db, zErr);
  }

  return rc;
}

void sqlite3VtabMakeWritable(Parse *pParse, Table *pTab) {
  Parse *pToplevel = ((pParse)->pToplevel ? (pParse)->pToplevel : (pParse));
  int i, n;
  Table **apVtabLock;

  for (i = 0; i < pToplevel->nVtabLock; i++) {
    if (pTab == pToplevel->apVtabLock[i])
      return;
  }
  n = (pToplevel->nVtabLock + 1) * sizeof(pToplevel->apVtabLock[0]);
  apVtabLock = (Table**)(sqlite3Realloc(pToplevel->apVtabLock, n));
  if (apVtabLock) {
    pToplevel->apVtabLock = apVtabLock;
    pToplevel->apVtabLock[pToplevel->nVtabLock++] = pTab;
  } else {
    sqlite3OomFault(pToplevel->db);
  }
}

int sqlite3VtabEponymousTableInit(Parse *pParse, Module *pMod) {
  const sqlite3_module *pModule = pMod->pModule;
  Table *pTab;
  char *zErr = 0;
  int rc;
  sqlite3 *db = pParse->db;
  if (pMod->pEpoTab)
    return 1;
  if (pModule->xCreate != 0 && pModule->xCreate != pModule->xConnect)
    return 0;
  pTab = (Table*)(sqlite3DbMallocZero(db, sizeof(Table)));
  if (pTab == 0)
    return 0;
  pTab->zName = sqlite3DbStrDup(db, pMod->zName);
  if (pTab->zName == 0) {
    sqlite3DbFree(db, pTab);
    return 0;
  }
  pMod->pEpoTab = pTab;
  pTab->nTabRef = 1;
  pTab->eTabType = 1;
  pTab->pSchema = db->aDb[0].pSchema;

  pTab->iPKey = -1;
  pTab->tabFlags |= 0x00008000;
  addModuleArgument(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  addModuleArgument(pParse, pTab, 0);
  addModuleArgument(pParse, pTab, sqlite3DbStrDup(db, pTab->zName));
  db->nSchemaLock++;
  rc = vtabCallConstructor(db, pTab, pMod, pModule->xConnect, &zErr);
  db->nSchemaLock--;
  if (rc) {
    sqlite3ErrorMsg(pParse, "%s", zErr);
    pParse->rc = rc;
    sqlite3DbFree(db, zErr);
    sqlite3VtabEponymousTableClear(db, pMod);
  }
  return 1;
}

void sqlite3WhereAddExplainText(Parse *pParse, int addr, SrcList *pTabList, WhereLevel *pLevel, u16 wctrlFlags) {
  if (((pParse)->pToplevel ? (pParse)->pToplevel : (pParse))->explain == 2 || 0) {
    VdbeOp *pOp = sqlite3VdbeGetOp(pParse->pVdbe, addr);
    SrcItem *pItem = &pTabList->a[pLevel->iFrom];
    sqlite3 *db = pParse->db;
    int isSearch;
    WhereLoop *pLoop;
    u32 flags;

    StrAccum str;
    char zBuf[100];

    if (db->mallocFailed)
      return;

    pLoop = pLevel->pWLoop;
    flags = pLoop->wsFlags;

    isSearch = (flags & (0x00000020 | 0x00000010)) != 0 || ((flags & 0x00000400) == 0 && (pLoop->u.btree.nEq > 0)) ||
               (wctrlFlags & (0x0001 | 0x0002));

    sqlite3StrAccumInit(&str, db, zBuf, sizeof(zBuf), 1000000000);
    str.printfFlags = 0x01;
    sqlite3_str_appendf(&str, "%s %S%s", isSearch ? "SEARCH" : "SCAN", pItem, pItem->fg.fromExists ? " EXISTS" : "");
    if ((flags & (0x00000100 | 0x00000400)) == 0) {
      const char *zFmt = 0;
      Index *pIdx;

      pIdx = pLoop->u.btree.pIndex;

      if (!(((pItem->pSTab)->tabFlags & 0x00000080) == 0) && ((pIdx)->idxType == 2)) {
        if (isSearch) {
          zFmt = "PRIMARY KEY";
        }
      } else if (flags & 0x00020000) {
        zFmt = "AUTOMATIC PARTIAL COVERING INDEX";
      } else if (flags & 0x00004000) {
        zFmt = "AUTOMATIC COVERING INDEX";
      } else if (flags & (0x00000040 | 0x04000000)) {
        zFmt = "COVERING INDEX %s";
      } else {
        zFmt = "INDEX %s";
      }
      if (zFmt) {
        sqlite3_str_append(&str, " USING ", 7);
        sqlite3_str_appendf(&str, zFmt, pIdx->zName);
        explainIndexRange(&str, pLoop);
      }
    } else if ((flags & 0x00000100) != 0 && (flags & 0x0000000f) != 0) {
      char cRangeOp;

      const char *zRowid = "rowid";

      sqlite3_str_appendf(&str, " USING INTEGER PRIMARY KEY (%s", zRowid);
      if (flags & (0x00000001 | 0x00000004)) {
        cRangeOp = '=';
      } else if ((flags & 0x00000030) == 0x00000030) {
        sqlite3_str_appendf(&str, ">? AND %s", zRowid);
        cRangeOp = '<';
      } else if (flags & 0x00000020) {
        cRangeOp = '>';
      } else {
        cRangeOp = '<';
      }
      sqlite3_str_appendf(&str, "%c?)", cRangeOp);
    }

    else if ((flags & 0x00000400) != 0) {
      sqlite3_str_appendall(&str, " VIRTUAL TABLE INDEX ");
      sqlite3_str_appendf(&str, pLoop->u.vtab.bIdxNumHex ? "0x%x:%s" : "%d:%s", pLoop->u.vtab.idxNum,
                          pLoop->u.vtab.idxStr);
    }

    if (pItem->fg.jointype & 0x08) {
      sqlite3_str_appendf(&str, " LEFT-JOIN");
    }

    sqlite3DbFree(db, pOp->p4.z);
    pOp->p4type = (-7);
    pOp->p4.z = sqlite3StrAccumFinish(&str);
  }
}

int sqlite3WhereExplainOneScan(Parse *pParse, SrcList *pTabList, WhereLevel *pLevel, u16 wctrlFlags) {
  int ret = 0;

  if (((pParse)->pToplevel ? (pParse)->pToplevel : (pParse))->explain == 2 || 0) {
    if ((pLevel->pWLoop->wsFlags & 0x00002000) == 0 && (wctrlFlags & 0x0020) == 0) {
      Vdbe *v = pParse->pVdbe;
      int addr = sqlite3VdbeCurrentAddr(v);
      ret = sqlite3VdbeAddOp3(v, 190, addr, pParse->addrExplain, pLevel->pWLoop->rRun);
      sqlite3WhereAddExplainText(pParse, addr, pTabList, pLevel, wctrlFlags);
    }
  }
  return ret;
}

int sqlite3WhereExplainBloomFilter(const Parse *pParse, const WhereInfo *pWInfo, const WhereLevel *pLevel) {
  int ret = 0;
  SrcItem *pItem = &pWInfo->pTabList->a[pLevel->iFrom];
  Vdbe *v = pParse->pVdbe;
  sqlite3 *db = pParse->db;
  char *zMsg;
  int i;
  WhereLoop *pLoop;
  StrAccum str;
  char zBuf[100];

  sqlite3StrAccumInit(&str, db, zBuf, sizeof(zBuf), 1000000000);
  str.printfFlags = 0x01;
  sqlite3_str_appendf(&str, "BLOOM FILTER ON %S (", pItem);
  pLoop = pLevel->pWLoop;
  if (pLoop->wsFlags & 0x00000100) {
    const Table *pTab = pItem->pSTab;
    if (pTab->iPKey >= 0) {
      sqlite3_str_appendf(&str, "%s=?", pTab->aCol[pTab->iPKey].zCnName);
    } else {
      sqlite3_str_appendf(&str, "rowid=?");
    }
  } else {
    for (i = pLoop->nSkip; i < pLoop->u.btree.nEq; i++) {
      const char *z = explainIndexColumnName(pLoop->u.btree.pIndex, i);
      if (i > pLoop->nSkip)
        sqlite3_str_append(&str, " AND ", 5);
      sqlite3_str_appendf(&str, "%s=?", z);
    }
  }
  sqlite3_str_append(&str, ")", 1);
  zMsg = sqlite3StrAccumFinish(&str);
  ret = sqlite3VdbeAddOp4(v, 190, sqlite3VdbeCurrentAddr(v), pParse->addrExplain, 0, zMsg, (-7));

  return ret;
}

void codeApplyAffinity(Parse *pParse, int base, int n, char *zAff) {
  Vdbe *v = pParse->pVdbe;
  if (zAff == 0) {
    return;
  }

  while (n > 0 && zAff[0] <= 0x41) {
    n--;
    base++;
    zAff++;
  }
  while (n > 1 && zAff[n - 1] <= 0x41) {
    n--;
  }

  if (n > 0) {
    sqlite3VdbeAddOp4(v, 98, base, n, 0, zAff, n);
  }
}

Expr *removeUnindexableInClauseTerms(Parse *pParse, int iEq, WhereLoop *pLoop, Expr *pX) {
  sqlite3 *db = pParse->db;
  Select *pSelect;
  Expr *pNew;
  pNew = sqlite3ExprDup(db, pX, 0);
  if (db->mallocFailed == 0) {
    for (pSelect = pNew->x.pSelect; pSelect; pSelect = pSelect->pPrior) {
      ExprList *pOrigRhs;
      ExprList *pOrigLhs = 0;
      ExprList *pRhs = 0;
      ExprList *pLhs = 0;
      int i;

      pOrigRhs = pSelect->pEList;

      if (pSelect == pNew->x.pSelect) {
        pOrigLhs = pNew->pLeft->x.pList;
      }
      for (i = iEq; i < pLoop->nLTerm; i++) {
        if (pLoop->aLTerm[i]->pExpr == pX) {
          int iField;

          iField = pLoop->aLTerm[i]->u.x.iField - 1;
          if (pOrigRhs->a[iField].pExpr == 0) {
            continue;
          }
          pRhs = sqlite3ExprListAppend(pParse, pRhs, pOrigRhs->a[iField].pExpr);
          pOrigRhs->a[iField].pExpr = 0;
          if (pRhs)
            pRhs->a[pRhs->nExpr - 1].u.x.iOrderByCol = iField + 1;
          if (pOrigLhs) {
            pLhs = sqlite3ExprListAppend(pParse, pLhs, pOrigLhs->a[iField].pExpr);
            pOrigLhs->a[iField].pExpr = 0;
          }
        }
      }
      sqlite3ExprListDelete(db, pOrigRhs);
      if (pOrigLhs) {
        sqlite3ExprListDelete(db, pOrigLhs);
        pNew->pLeft->x.pList = pLhs;
      }
      pSelect->pEList = pRhs;
      pSelect->selId = ++pParse->nSelect;
      if (pLhs && pLhs->nExpr == 1) {
        Expr *p = pLhs->a[0].pExpr;
        pLhs->a[0].pExpr = 0;
        sqlite3ExprDelete(db, pNew->pLeft);
        pNew->pLeft = p;
      }

      if (pRhs) {
        adjustOrderByCol(pSelect->pOrderBy, pRhs);
        adjustOrderByCol(pSelect->pGroupBy, pRhs);
        for (i = 0; i < pRhs->nExpr; i++)
          pRhs->a[i].u.x.iOrderByCol = 0;
      }
    }
  }
  return pNew;
}

__attribute__((noinline)) void codeINTerm(Parse *pParse, WhereTerm *pTerm, WhereLevel *pLevel, int iEq, int bRev,
                                          int iTarget) {
  Expr *pX = pTerm->pExpr;
  int eType = 5;
  int iTab;
  struct InLoop *pIn;
  WhereLoop *pLoop = pLevel->pWLoop;
  Vdbe *v = pParse->pVdbe;
  int i;
  int nEq = 0;
  int *aiMap = 0;

  if ((pLoop->wsFlags & 0x00000400) == 0 && pLoop->u.btree.pIndex != 0 && pLoop->u.btree.pIndex->aSortOrder[iEq]) {
    bRev = !bRev;
  }

  for (i = 0; i < iEq; i++) {
    if (pLoop->aLTerm[i] && pLoop->aLTerm[i]->pExpr == pX) {
      disableTerm(pLevel, pTerm);
      return;
    }
  }
  for (i = iEq; i < pLoop->nLTerm; i++) {
    if (pLoop->aLTerm[i]->pExpr == pX)
      nEq++;
  }

  iTab = 0;
  if (!(((pX)->flags & 0x001000) != 0) || pX->x.pSelect->pEList->nExpr == 1) {
    eType = sqlite3FindInIndex(pParse, pX, 0x0004, 0, 0, &iTab);
  } else {
    sqlite3 *db = pParse->db;
    Expr *pXMod = removeUnindexableInClauseTerms(pParse, iEq, pLoop, pX);
    if (!db->mallocFailed) {
      aiMap = (int *)sqlite3DbMallocZero(db, sizeof(int) * nEq);
      eType = sqlite3FindInIndex(pParse, pXMod, 0x0004, 0, aiMap, &iTab);
    }
    sqlite3ExprDelete(db, pXMod);
  }

  if (eType == 4) {
    bRev = !bRev;
  }
  sqlite3VdbeAddOp2(v, bRev ? 32 : 36, iTab, 0);

  pLoop->wsFlags |= 0x00000800;
  if (pLevel->u.in.nIn == 0) {
    pLevel->addrNxt = sqlite3VdbeMakeLabel(pParse);
  }
  if (iEq > 0 && (pLoop->wsFlags & 0x00100000) == 0) {
    pLoop->wsFlags |= 0x00040000;
  }

  i = pLevel->u.in.nIn;
  pLevel->u.in.nIn += nEq;
  pLevel->u.in.aInLoop =
      (InLoop*)(sqlite3WhereRealloc(pTerm->pWC->pWInfo, pLevel->u.in.aInLoop, sizeof(pLevel->u.in.aInLoop[0]) * pLevel->u.in.nIn));
  pIn = pLevel->u.in.aInLoop;
  if (pIn) {
    int iMap = 0;
    pIn += i;
    for (i = iEq; i < pLoop->nLTerm; i++) {
      if (pLoop->aLTerm[i]->pExpr == pX) {
        int iOut = iTarget + i - iEq;
        if (eType == 1) {
          pIn->addrInTop = sqlite3VdbeAddOp2(v, 137, iTab, iOut);
        } else {
          int iCol = aiMap ? aiMap[iMap++] : 0;
          pIn->addrInTop = sqlite3VdbeAddOp3(v, 96, iTab, iCol, iOut);
        }
        sqlite3VdbeAddOp1(v, 51, iOut);
        if (i == iEq) {
          pIn->iCur = iTab;
          pIn->eEndLoopOp = bRev ? 39 : 40;
          if (iEq > 0) {
            pIn->iBase = iTarget - i;
            pIn->nPrefix = i;
          } else {
            pIn->nPrefix = 0;
          }
        } else {
          pIn->eEndLoopOp = 189;
        }
        pIn++;
      }
    }

    if (iEq > 0 && (pLoop->wsFlags & (0x00100000 | 0x00000400)) == 0) {
      sqlite3VdbeAddOp3(v, 127, pLevel->iIdxCur, 0, iEq);
    }
  } else {
    pLevel->u.in.nIn = 0;
  }
  sqlite3DbFree(pParse->db, aiMap);
}

int codeEqualityTerm(Parse *pParse, WhereTerm *pTerm, WhereLevel *pLevel, int iEq, int bRev, int iTarget) {
  Expr *pX = pTerm->pExpr;
  int iReg;

  if (pX->op == 54 || pX->op == 45) {
    iReg = sqlite3ExprCodeTarget(pParse, pX->pRight, iTarget);
  } else if (pX->op == 51) {
    iReg = iTarget;
    sqlite3VdbeAddOp2(pParse->pVdbe, 77, 0, iReg);

  } else {
    iReg = iTarget;
    codeINTerm(pParse, pTerm, pLevel, iEq, bRev, iTarget);
  }

  if ((pLevel->pWLoop->wsFlags & 0x00200000) == 0 || (pTerm->eOperator & 0x0800) == 0) {
    disableTerm(pLevel, pTerm);
  }

  return iReg;
}

int codeAllEqualityTerms(Parse *pParse, WhereLevel *pLevel, int bRev, int nExtraReg, char **pzAff) {
  u16 nEq;
  u16 nSkip;
  Vdbe *v = pParse->pVdbe;
  Index *pIdx;
  WhereTerm *pTerm;
  WhereLoop *pLoop;
  int j;
  int regBase;
  int nReg;
  char *zAff;

  pLoop = pLevel->pWLoop;

  nEq = pLoop->u.btree.nEq;
  nSkip = pLoop->nSkip;
  pIdx = pLoop->u.btree.pIndex;

  regBase = pParse->nMem + 1;
  nReg = nEq + nExtraReg;
  pParse->nMem += nReg;

  zAff = sqlite3DbStrDup(pParse->db, sqlite3IndexAffinityStr(pParse->db, pIdx));

  if (nSkip) {
    int iIdxCur = pLevel->iIdxCur;
    sqlite3VdbeAddOp3(v, 77, 0, regBase, regBase + nSkip - 1);
    sqlite3VdbeAddOp1(v, (bRev ? 32 : 36), iIdxCur);
    j = sqlite3VdbeAddOp0(v, 9);

    pLevel->addrSkip = sqlite3VdbeAddOp4Int(v, (bRev ? 21 : 24), iIdxCur, 0, regBase, nSkip);
    sqlite3VdbeJumpHere(v, j);
    for (j = 0; j < nSkip; j++) {
      sqlite3VdbeAddOp3(v, 96, iIdxCur, j, regBase + j);
    }
  }

  for (j = nSkip; j < nEq; j++) {
    int r1;
    pTerm = pLoop->aLTerm[j];

    r1 = codeEqualityTerm(pParse, pTerm, pLevel, j, bRev, regBase + j);
    if (r1 != regBase + j) {
      if (nReg == 1) {
        sqlite3ReleaseTempReg(pParse, regBase);
        regBase = r1;
      } else {
        sqlite3VdbeAddOp2(v, 82, r1, regBase + j);
      }
    }
    if (pTerm->eOperator & 0x0001) {
      if (pTerm->pExpr->flags & 0x001000) {
        if (zAff)
          zAff[j] = 0x41;
      }
    } else if ((pTerm->eOperator & 0x0100) == 0) {
      Expr *pRight = pTerm->pExpr->pRight;
      if ((pTerm->wtFlags & 0x0800) == 0 && sqlite3ExprCanBeNull(pRight)) {
        sqlite3VdbeAddOp2(v, 51, regBase + j, pLevel->addrBrk);
      }
      if (pParse->nErr == 0) {
        if (sqlite3CompareAffinity(pRight, zAff[j]) == 0x41) {
          zAff[j] = 0x41;
        }
        if (sqlite3ExprNeedsNoAffinityChange(pRight, zAff[j])) {
          zAff[j] = 0x41;
        }
      }
    }
  }
  *pzAff = zAff;
  return regBase;
}

void codeExprOrVector(Parse *pParse, Expr *p, int iReg, int nReg) {
  if (p && sqlite3ExprIsVector(p)) {
    if ((((p)->flags & 0x001000) != 0)) {
      Vdbe *v = pParse->pVdbe;
      int iSelect;

      iSelect = sqlite3CodeSubselect(pParse, p);
      sqlite3VdbeAddOp3(v, 82, iSelect, iReg, nReg - 1);
    } else {
      int i;
      const ExprList *pList;

      pList = p->x.pList;

      for (i = 0; i < nReg; i++) {
        sqlite3ExprCode(pParse, pList->a[i].pExpr, iReg + i);
      }
    }
  } else {
    sqlite3ExprCode(pParse, p, iReg);
  }
}

__attribute__((noinline)) void filterPullDown(Parse *pParse, WhereInfo *pWInfo, int iLevel, int addrNxt,
                                              Bitmask notReady) {
  int saved_addrBrk;
  while (++iLevel < pWInfo->nLevel) {
    WhereLevel *pLevel = &pWInfo->a[iLevel];
    WhereLoop *pLoop = pLevel->pWLoop;
    if (pLevel->regFilter == 0)
      continue;
    if (pLevel->pWLoop->nSkip)
      continue;

    if ((pLoop->prereq & notReady))
      continue;
    saved_addrBrk = pLevel->addrBrk;
    pLevel->addrBrk = addrNxt;
    if (pLoop->wsFlags & 0x00000100) {
      WhereTerm *pTerm = pLoop->aLTerm[0];
      int regRowid;

      regRowid = sqlite3GetTempReg(pParse);
      regRowid = codeEqualityTerm(pParse, pTerm, pLevel, 0, 0, regRowid);
      sqlite3VdbeAddOp2(pParse->pVdbe, 13, regRowid, addrNxt);
      sqlite3VdbeAddOp4Int(pParse->pVdbe, 66, pLevel->regFilter, addrNxt, regRowid, 1);
    } else {
      u16 nEq = pLoop->u.btree.nEq;
      int r1;
      char *zStartAff;

      r1 = codeAllEqualityTerms(pParse, pLevel, 0, 0, &zStartAff);
      codeApplyAffinity(pParse, r1, nEq, zStartAff);
      sqlite3DbFree(pParse->db, zStartAff);
      sqlite3VdbeAddOp4Int(pParse->pVdbe, 66, pLevel->regFilter, addrNxt, r1, nEq);
    }
    pLevel->regFilter = 0;
    pLevel->addrBrk = saved_addrBrk;
  }
}

Bitmask sqlite3WhereCodeOneLoopStart(Parse *pParse, Vdbe *v, WhereInfo *pWInfo, int iLevel, WhereLevel *pLevel,
                                     Bitmask notReady) {
  int j, k;
  int iCur;
  int addrNxt;
  int bRev;
  WhereLoop *pLoop;
  WhereClause *pWC;
  WhereTerm *pTerm;
  sqlite3 *db;
  SrcItem *pTabItem;
  int addrBrk;
  int addrCont;
  int iRowidReg = 0;
  int iReleaseReg = 0;
  Index *pIdx = 0;
  int iLoop;

  pWC = &pWInfo->sWC;
  db = pParse->db;
  pLoop = pLevel->pWLoop;
  pTabItem = &pWInfo->pTabList->a[pLevel->iFrom];
  iCur = pTabItem->iCursor;
  pLevel->notReady = notReady & ~sqlite3WhereGetMask(&pWInfo->sMaskSet, iCur);
  bRev = (pWInfo->revMask >> iLevel) & 1;

  addrBrk = pLevel->addrNxt = pLevel->addrBrk;
  addrCont = pLevel->addrCont = sqlite3VdbeMakeLabel(pParse);

  if (pLevel->iFrom > 0 && (pTabItem[0].fg.jointype & 0x08) != 0) {
    pLevel->iLeftJoin = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, 73, 0, pLevel->iLeftJoin);
  }

  if (pTabItem->fg.viaCoroutine) {
    int regYield;
    Subquery *pSubq;

    pSubq = pTabItem->u4.pSubq;
    regYield = pSubq->regReturn;
    sqlite3VdbeAddOp3(v, 11, regYield, 0, pSubq->addrFillSub);
    pLevel->p2 = sqlite3VdbeAddOp2(v, 12, regYield, addrBrk);
    pLevel->op = 9;
  } else if ((pLoop->wsFlags & 0x00000400) != 0) {
    int iReg;
    int addrNotFound;
    int nConstraint = pLoop->nLTerm;

    iReg = sqlite3GetTempRange(pParse, nConstraint + 2);
    addrNotFound = pLevel->addrBrk;
    for (j = 0; j < nConstraint; j++) {
      int iTarget = iReg + j + 2;
      pTerm = pLoop->aLTerm[j];
      if (pTerm == 0)
        continue;
      if (pTerm->eOperator & 0x0001) {
        if (((j) <= 31 ? ((unsigned int)1) << (j) : 0) & pLoop->u.vtab.mHandleIn) {
          int iTab = pParse->nTab++;
          int iCache = ++pParse->nMem;
          sqlite3CodeRhsOfIN(pParse, pTerm->pExpr, iTab, 0);
          sqlite3VdbeAddOp3(v, 177, iTab, iTarget, iCache);
        } else {
          codeEqualityTerm(pParse, pTerm, pLevel, j, bRev, iTarget);
          addrNotFound = pLevel->addrNxt;
        }
      } else {
        Expr *pRight = pTerm->pExpr->pRight;
        codeExprOrVector(pParse, pRight, iTarget, 1);
        if (pTerm->eMatchOp == SQLITE_INDEX_CONSTRAINT_OFFSET && pLoop->u.vtab.bOmitOffset) {
          sqlite3VdbeAddOp2(v, 73, 0, pWInfo->pSelect->iOffset);
        }
      }
    }
    sqlite3VdbeAddOp2(v, 73, pLoop->u.vtab.idxNum, iReg);
    sqlite3VdbeAddOp2(v, 73, nConstraint, iReg + 1);

    sqlite3VdbeAddOp4(v, 6, iCur, addrNotFound, iReg, pLoop->u.vtab.idxStr, pLoop->u.vtab.needFree ? (-7) : (-1));
    pLoop->u.vtab.needFree = 0;

    if (db->mallocFailed)
      pLoop->u.vtab.idxStr = 0;
    pLevel->p1 = iCur;
    pLevel->op = pWInfo->eOnePass ? 189 : 65;
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);

    for (j = 0; j < nConstraint; j++) {
      pTerm = pLoop->aLTerm[j];
      if (j < 16 && (pLoop->u.vtab.omitMask >> j) & 1) {
        disableTerm(pLevel, pTerm);
        continue;
      }
      if ((pTerm->eOperator & 0x0001) != 0 &&
          (((j) <= 31 ? ((unsigned int)1) << (j) : 0) & pLoop->u.vtab.mHandleIn) == 0 && !db->mallocFailed) {
        Expr *pCompare;
        Expr *pRight;
        VdbeOp *pOp;
        int iIn;

        for (iIn = 0; (iIn < pLevel->u.in.nIn); iIn++) {
          pOp = sqlite3VdbeGetOp(v, pLevel->u.in.aInLoop[iIn].addrInTop);
          if ((pOp->opcode == 96 && pOp->p3 == iReg + j + 2) || (pOp->opcode == 137 && pOp->p2 == iReg + j + 2)) {
            sqlite3VdbeAddOp3(v, pOp->opcode, pOp->p1, pOp->p2, pOp->p3);
            break;
          }
        }

        pCompare = sqlite3PExpr(pParse, 54, 0, 0);
        if (!db->mallocFailed) {
          int iFld = pTerm->u.x.iField;
          Expr *pLeft = pTerm->pExpr->pLeft;

          if (iFld > 0) {
            pCompare->pLeft = pLeft->x.pList->a[iFld - 1].pExpr;
          } else {
            pCompare->pLeft = pLeft;
          }
          pCompare->pRight = pRight = sqlite3Expr(db, 176, 0);
          if (pRight) {
            pRight->iTable = iReg + j + 2;
            sqlite3ExprIfFalse(pParse, pCompare, pLevel->addrCont, 0x10);
          }
          pCompare->pLeft = 0;
        }
        sqlite3ExprDelete(db, pCompare);
      }
    }

  } else if ((pLoop->wsFlags & 0x00000100) != 0 && (pLoop->wsFlags & (0x00000004 | 0x00000001)) != 0) {
    pTerm = pLoop->aLTerm[0];

    iReleaseReg = ++pParse->nMem;
    iRowidReg = codeEqualityTerm(pParse, pTerm, pLevel, 0, bRev, iReleaseReg);
    if (iRowidReg != iReleaseReg)
      sqlite3ReleaseTempReg(pParse, iReleaseReg);
    addrNxt = pLevel->addrNxt;
    if (pLevel->regFilter) {
      sqlite3VdbeAddOp2(v, 13, iRowidReg, addrNxt);
      sqlite3VdbeAddOp4Int(v, 66, pLevel->regFilter, addrNxt, iRowidReg, 1);
      filterPullDown(pParse, pWInfo, iLevel, addrNxt, notReady);
    }
    sqlite3VdbeAddOp3(v, 30, iCur, addrNxt, iRowidReg);
    pLevel->op = 189;
  } else if ((pLoop->wsFlags & 0x00000100) != 0 && (pLoop->wsFlags & 0x00000002) != 0) {
    int testOp = 189;
    int start;
    int memEndValue = 0;
    WhereTerm *pStart, *pEnd;

    j = 0;
    pStart = pEnd = 0;
    if (pLoop->wsFlags & 0x00000020)
      pStart = pLoop->aLTerm[j++];
    if (pLoop->wsFlags & 0x00000010)
      pEnd = pLoop->aLTerm[j++];

    if (bRev) {
      pTerm = pStart;
      pStart = pEnd;
      pEnd = pTerm;
    };
    if (pStart) {
      Expr *pX;
      int r1, rTemp;
      int op;

      const u8 aMoveOp[] = {24, 22, 21, 23};

      pX = pStart->pExpr;

      if (sqlite3ExprIsVector(pX->pRight)) {
        r1 = rTemp = sqlite3GetTempReg(pParse);
        codeExprOrVector(pParse, pX->pRight, r1, 1);
        op = aMoveOp[((pX->op - 55 - 1) & 0x3) | 0x1];

      } else {
        r1 = sqlite3ExprCodeTemp(pParse, pX->pRight, &rTemp);
        disableTerm(pLevel, pStart);
        op = aMoveOp[(pX->op - 55)];
      }
      sqlite3VdbeAddOp3(v, op, iCur, addrBrk, r1);
      sqlite3ReleaseTempReg(pParse, rTemp);
    } else {
      sqlite3VdbeAddOp2(v, bRev ? 32 : 36, iCur, pLevel->addrHalt);
    }
    if (pEnd) {
      Expr *pX;
      pX = pEnd->pExpr;

      memEndValue = ++pParse->nMem;
      codeExprOrVector(pParse, pX->pRight, memEndValue, 1);
      if (0 == sqlite3ExprIsVector(pX->pRight) && (pX->op == 57 || pX->op == 55)) {
        testOp = bRev ? 56 : 58;
      } else {
        testOp = bRev ? 57 : 55;
      }
      if (0 == sqlite3ExprIsVector(pX->pRight)) {
        disableTerm(pLevel, pEnd);
      }
    }
    start = sqlite3VdbeCurrentAddr(v);
    pLevel->op = bRev ? 39 : 40;
    pLevel->p1 = iCur;
    pLevel->p2 = start;

    if (testOp != 189) {
      iRowidReg = ++pParse->nMem;
      sqlite3VdbeAddOp2(v, 137, iCur, iRowidReg);
      sqlite3VdbeAddOp3(v, testOp, memEndValue, addrBrk, iRowidReg);
      sqlite3VdbeChangeP5(v, 0x43 | 0x10);
    }
  } else if (pLoop->wsFlags & 0x00000200) {
    static const u8 aStartOp[] = {0, 0, 36, 32, 24, 21, 23, 22};
    static const u8 aEndOp[] = {
        46,
        42,
        41,
        45,
    };
    u16 nEq = pLoop->u.btree.nEq;
    u16 nBtm = pLoop->u.btree.nBtm;
    u16 nTop = pLoop->u.btree.nTop;
    int regBase;
    WhereTerm *pRangeStart = 0;
    WhereTerm *pRangeEnd = 0;
    int startEq;
    int endEq;
    int start_constraints;
    int nConstraint;
    int iIdxCur;
    int nExtraReg = 0;
    int op;
    char *zStartAff;
    char *zEndAff = 0;
    u8 bSeekPastNull = 0;
    u8 bStopAtNull = 0;
    int omitTable;
    int regBignull = 0;
    int addrSeekScan = 0;

    pIdx = pLoop->u.btree.pIndex;
    iIdxCur = pLevel->iIdxCur;

    j = nEq;
    if (pLoop->wsFlags & 0x00000020) {
      pRangeStart = pLoop->aLTerm[j++];
      nExtraReg = ((nExtraReg) > (pLoop->u.btree.nBtm) ? (nExtraReg) : (pLoop->u.btree.nBtm));
    }
    if (pLoop->wsFlags & 0x00000010) {
      pRangeEnd = pLoop->aLTerm[j++];
      nExtraReg = ((nExtraReg) > (pLoop->u.btree.nTop) ? (nExtraReg) : (pLoop->u.btree.nTop));

      if ((pRangeEnd->wtFlags & 0x0100) != 0) {
        pLevel->iLikeRepCntr = (u32)++pParse->nMem;
        sqlite3VdbeAddOp2(v, 73, 1, (int)pLevel->iLikeRepCntr);
        pLevel->addrLikeRep = sqlite3VdbeCurrentAddr(v);

        pLevel->iLikeRepCntr <<= 1;
        pLevel->iLikeRepCntr |= bRev ^ (pIdx->aSortOrder[nEq] == 1);
      }

      if (pRangeStart == 0) {
        j = pIdx->aiColumn[nEq];
        if ((j >= 0 && pIdx->pTable->aCol[j].notNull == 0) || j == (-2)) {
          bSeekPastNull = 1;
        }
      }
    }

    if ((pLoop->wsFlags & (0x00000010 | 0x00000020)) == 0 && (pLoop->wsFlags & 0x00080000) != 0) {
      nExtraReg = 1;
      bSeekPastNull = 1;
      pLevel->regBignull = regBignull = ++pParse->nMem;
      if (pLevel->iLeftJoin) {
        sqlite3VdbeAddOp2(v, 73, 0, regBignull);
      }
      pLevel->addrBignull = sqlite3VdbeMakeLabel(pParse);
    }

    if ((nEq < pIdx->nColumn && bRev == (pIdx->aSortOrder[nEq] == 0))) {
      {
        WhereTerm *t = pRangeEnd;
        pRangeEnd = pRangeStart;
        pRangeStart = t;
      };
      {
        u8 t = bSeekPastNull;
        bSeekPastNull = bStopAtNull;
        bStopAtNull = t;
      };
      {
        u8 t = nBtm;
        nBtm = nTop;
        nTop = t;
      };
    }

    if (iLevel > 0 && (pLoop->wsFlags & 0x00100000) != 0) {
      sqlite3VdbeAddOp1(v, 138, iIdxCur);
    }

    regBase = codeAllEqualityTerms(pParse, pLevel, bRev, nExtraReg, &zStartAff);

    if (zStartAff && nTop) {
      zEndAff = sqlite3DbStrDup(db, &zStartAff[nEq]);
    }
    addrNxt = (regBignull ? pLevel->addrBignull : pLevel->addrNxt);

    startEq = !pRangeStart || pRangeStart->eOperator & ((0x0002 << (56 - 54)) | (0x0002 << (58 - 54)));
    endEq = !pRangeEnd || pRangeEnd->eOperator & ((0x0002 << (56 - 54)) | (0x0002 << (58 - 54)));
    start_constraints = pRangeStart || nEq > 0;

    nConstraint = nEq;
    if (pRangeStart) {
      Expr *pRight = pRangeStart->pExpr->pRight;
      codeExprOrVector(pParse, pRight, regBase + nEq, nBtm);
      whereLikeOptimizationStringFixup(v, pLevel, pRangeStart);
      if ((pRangeStart->wtFlags & 0x0080) == 0 && sqlite3ExprCanBeNull(pRight)) {
        sqlite3VdbeAddOp2(v, 51, regBase + nEq, addrNxt);
      }
      if (zStartAff) {
        updateRangeAffinityStr(pRight, nBtm, &zStartAff[nEq]);
      }
      nConstraint += nBtm;
      if (sqlite3ExprIsVector(pRight) == 0) {
        disableTerm(pLevel, pRangeStart);
      } else {
        startEq = 1;
      }
      bSeekPastNull = 0;
    } else if (bSeekPastNull) {
      startEq = 0;
      sqlite3VdbeAddOp2(v, 77, 0, regBase + nEq);
      start_constraints = 1;
      nConstraint++;
    } else if (regBignull) {
      sqlite3VdbeAddOp2(v, 77, 0, regBase + nEq);
      start_constraints = 1;
      nConstraint++;
    }
    codeApplyAffinity(pParse, regBase, nConstraint - bSeekPastNull, zStartAff);
    if (pLoop->nSkip > 0 && nConstraint == pLoop->nSkip) {
    } else {
      if (regBignull) {
        sqlite3VdbeAddOp2(v, 73, 1, regBignull);
      }
      if (pLevel->regFilter) {
        sqlite3VdbeAddOp4Int(v, 66, pLevel->regFilter, addrNxt, regBase, nEq);
        filterPullDown(pParse, pWInfo, iLevel, addrNxt, notReady);
      }

      op = aStartOp[(start_constraints << 2) + (startEq << 1) + bRev];

      if ((pLoop->wsFlags & 0x00100000) != 0 && op == 23) {
        addrSeekScan = sqlite3VdbeAddOp1(v, 126, (pIdx->aiRowLogEst[0] + 9) / 10);
        if (pRangeStart || pRangeEnd) {
          sqlite3VdbeChangeP5(v, 1);
          sqlite3VdbeChangeP2(v, addrSeekScan, sqlite3VdbeCurrentAddr(v) + 1);
          addrSeekScan = 0;
        };
      }
      sqlite3VdbeAddOp4Int(v, op, iIdxCur, addrNxt, regBase, nConstraint);

      if (regBignull) {
        sqlite3VdbeAddOp2(v, 9, 0, sqlite3VdbeCurrentAddr(v) + 2);
        op = aStartOp[(nConstraint > 1) * 4 + 2 + bRev];
        sqlite3VdbeAddOp4Int(v, op, iIdxCur, addrNxt, regBase, nConstraint - startEq);
      }
    }

    nConstraint = nEq;

    if (pRangeEnd) {
      Expr *pRight = pRangeEnd->pExpr->pRight;

      codeExprOrVector(pParse, pRight, regBase + nEq, nTop);
      whereLikeOptimizationStringFixup(v, pLevel, pRangeEnd);
      if ((pRangeEnd->wtFlags & 0x0080) == 0 && sqlite3ExprCanBeNull(pRight)) {
        sqlite3VdbeAddOp2(v, 51, regBase + nEq, addrNxt);
      }
      if (zEndAff) {
        updateRangeAffinityStr(pRight, nTop, zEndAff);
        codeApplyAffinity(pParse, regBase + nEq, nTop, zEndAff);
      } else {
      }
      nConstraint += nTop;

      if (sqlite3ExprIsVector(pRight) == 0) {
        disableTerm(pLevel, pRangeEnd);
      } else {
        endEq = 1;
      }
    } else if (bStopAtNull) {
      if (regBignull == 0) {
        sqlite3VdbeAddOp2(v, 77, 0, regBase + nEq);
        endEq = 0;
      }
      nConstraint++;
    }
    if (zStartAff)
      sqlite3DbNNFreeNN(db, zStartAff);
    if (zEndAff)
      sqlite3DbNNFreeNN(db, zEndAff);

    pLevel->p2 = sqlite3VdbeCurrentAddr(v);

    if (nConstraint) {
      if (regBignull) {
        sqlite3VdbeAddOp2(v, 17, regBignull, sqlite3VdbeCurrentAddr(v) + 3);
      }
      op = aEndOp[bRev * 2 + endEq];
      sqlite3VdbeAddOp4Int(v, op, iIdxCur, addrNxt, regBase, nConstraint);
      if (addrSeekScan)
        sqlite3VdbeJumpHere(v, addrSeekScan);
    }
    if (regBignull) {
      sqlite3VdbeAddOp2(v, 16, regBignull, sqlite3VdbeCurrentAddr(v) + 2);
      op = aEndOp[bRev * 2 + bSeekPastNull];
      sqlite3VdbeAddOp4Int(v, op, iIdxCur, addrNxt, regBase, nConstraint + bSeekPastNull);
    }

    if ((pLoop->wsFlags & 0x00040000) != 0) {
      sqlite3VdbeAddOp3(v, 127, iIdxCur, nEq, nEq);
    }

    omitTable = (pLoop->wsFlags & 0x00000040) != 0 && (pWInfo->wctrlFlags & (0x0020 | 0x1000)) == 0;
    if (omitTable) {
    } else if ((((pIdx->pTable)->tabFlags & 0x00000080) == 0)) {
      codeDeferredSeek(pWInfo, pIdx, iCur, iIdxCur);
    } else if (iCur != iIdxCur) {
      Index *pPk = sqlite3PrimaryKeyIndex(pIdx->pTable);
      iRowidReg = sqlite3GetTempRange(pParse, pPk->nKeyCol);
      for (j = 0; j < pPk->nKeyCol; j++) {
        k = sqlite3TableColumnToIndex(pIdx, pPk->aiColumn[j]);
        sqlite3VdbeAddOp3(v, 96, iIdxCur, k, iRowidReg + j);
      }
      sqlite3VdbeAddOp4Int(v, 28, iCur, addrCont, iRowidReg, pPk->nKeyCol);
    }

    if (pLevel->iLeftJoin == 0) {
      if (pIdx->pPartIdxWhere && pLevel->pRJ == 0) {
        whereApplyPartialIndexConstraints(pIdx->pPartIdxWhere, iCur, pWC);
      }
    } else {
    }

    if ((pLoop->wsFlags & 0x00001000) || (pLevel->u.in.nIn && regBignull == 0 && whereLoopIsOneRow(pLoop))) {
      pLevel->op = 189;
    } else if (bRev) {
      pLevel->op = 39;
    } else {
      pLevel->op = 40;
    }
    pLevel->p1 = iIdxCur;
    pLevel->p3 = (pLoop->wsFlags & 0x00010000) != 0 ? 1 : 0;
    if ((pLoop->wsFlags & 0x0000000f) == 0) {
      pLevel->p5 = SQLITE_STMTSTATUS_FULLSCAN_STEP;
    } else {
    }
    if (omitTable)
      pIdx = 0;
  } else if (pLoop->wsFlags & 0x00002000) {
    WhereClause *pOrWc;
    SrcList *pOrTab;
    Index *pCov = 0;
    int iCovCur = pParse->nTab++;

    int regReturn = ++pParse->nMem;
    int regRowset = 0;
    int regRowid = 0;
    int iLoopBody = sqlite3VdbeMakeLabel(pParse);
    int iRetInit;
    int untestedTerms = 0;
    int ii;
    Expr *pAndExpr = 0;
    Table *pTab = pTabItem->pSTab;

    pTerm = pLoop->aLTerm[0];

    pOrWc = &pTerm->u.pOrInfo->wc;
    pLevel->op = 69;
    pLevel->p1 = regReturn;

    if (pWInfo->nLevel > 1 || pTabItem->fg.fromExists) {
      int nNotReady;
      SrcItem *origSrc;
      nNotReady = pWInfo->nLevel - iLevel - 1;
      pOrTab = (SrcList*)(sqlite3DbMallocRawNN(db, (offsetof(SrcList, a) + (nNotReady + 1) * sizeof(SrcItem))));
      if (pOrTab == 0)
        return notReady;
      pOrTab->nAlloc = (u8)(nNotReady + 1);
      pOrTab->nSrc = pOrTab->nAlloc;
      memcpy(pOrTab->a, pTabItem, sizeof(*pTabItem));
      origSrc = pWInfo->pTabList->a;
      for (k = 1; k <= nNotReady; k++) {
        memcpy(&pOrTab->a[k], &origSrc[pLevel[k].iFrom], sizeof(pOrTab->a[k]));
      }

      pOrTab->a[0].fg.fromExists = 0;
    } else {
      pOrTab = pWInfo->pTabList;
    }

    if ((pWInfo->wctrlFlags & 0x0010) == 0) {
      if ((((pTab)->tabFlags & 0x00000080) == 0)) {
        regRowset = ++pParse->nMem;
        sqlite3VdbeAddOp2(v, 77, 0, regRowset);
      } else {
        Index *pPk = sqlite3PrimaryKeyIndex(pTab);
        regRowset = pParse->nTab++;
        sqlite3VdbeAddOp2(v, 120, regRowset, pPk->nKeyCol);
        sqlite3VdbeSetP4KeyInfo(pParse, pPk);
      }
      regRowid = ++pParse->nMem;
    }
    iRetInit = sqlite3VdbeAddOp2(v, 73, 0, regReturn);

    if (pWC->nTerm > 1) {
      int iTerm;
      for (iTerm = 0; iTerm < pWC->nTerm; iTerm++) {
        Expr *pExpr = pWC->a[iTerm].pExpr;
        if (&pWC->a[iTerm] == pTerm)
          continue;
        if ((pWC->a[iTerm].wtFlags & (0x0002 | 0x0004 | 0x8000)) != 0) {
          continue;
        }
        if ((pWC->a[iTerm].eOperator & 0x3fff) == 0)
          continue;
        if ((((pExpr)->flags & (u32)(0x400000)) != 0))
          continue;
        pExpr = sqlite3ExprDup(db, pExpr, 0);
        pAndExpr = sqlite3ExprAnd(pParse, pAndExpr, pExpr);
      }
      if (pAndExpr) {
        pAndExpr = sqlite3PExpr(pParse, 44 | 0x10000, 0, pAndExpr);
      }
    }

    sqlite3VdbeExplain(pParse, 1, "MULTI-INDEX OR");
    for (ii = 0; ii < pOrWc->nTerm; ii++) {
      WhereTerm *pOrTerm = &pOrWc->a[ii];
      if (pOrTerm->leftCursor == iCur || (pOrTerm->eOperator & 0x0400) != 0) {
        WhereInfo *pSubWInfo;
        Expr *pOrExpr = pOrTerm->pExpr;
        Expr *pDelete;
        int jmp1 = 0;

        pDelete = pOrExpr = sqlite3ExprDup(db, pOrExpr, 0);
        if (db->mallocFailed) {
          sqlite3ExprDelete(db, pDelete);
          continue;
        }
        if (pAndExpr) {
          pAndExpr->pLeft = pOrExpr;
          pOrExpr = pAndExpr;
        }

        sqlite3VdbeExplain(pParse, 1, "INDEX %d", ii + 1);
        pSubWInfo = sqlite3WhereBegin(pParse, pOrTab, pOrExpr, 0, 0, 0, 0x0020, iCovCur);

        if (pSubWInfo) {
          WhereLoop *pSubLoop;
          int addrExplain = sqlite3WhereExplainOneScan(pParse, pOrTab, &pSubWInfo->a[0], 0);
          ((void)addrExplain);

          if ((pWInfo->wctrlFlags & 0x0010) == 0) {
            int iSet = ((ii == pOrWc->nTerm - 1) ? -1 : ii);
            if ((((pTab)->tabFlags & 0x00000080) == 0)) {
              sqlite3ExprCodeGetColumnOfTable(v, pTab, iCur, -1, regRowid);
              jmp1 = sqlite3VdbeAddOp4Int(v, 49, regRowset, 0, regRowid, iSet);
            } else {
              Index *pPk = sqlite3PrimaryKeyIndex(pTab);
              int nPk = pPk->nKeyCol;
              int iPk;
              int r;

              r = sqlite3GetTempRange(pParse, nPk);
              for (iPk = 0; iPk < nPk; iPk++) {
                int iCol = pPk->aiColumn[iPk];
                sqlite3ExprCodeGetColumnOfTable(v, pTab, iCur, iCol, r + iPk);
              }

              if (iSet) {
                jmp1 = sqlite3VdbeAddOp4Int(v, 29, regRowset, 0, r, nPk);
              }
              if (iSet >= 0) {
                sqlite3VdbeAddOp3(v, 99, r, nPk, regRowid);
                sqlite3VdbeAddOp4Int(v, 140, regRowset, regRowid, r, nPk);
                if (iSet)
                  sqlite3VdbeChangeP5(v, 0x10);
              }

              sqlite3ReleaseTempRange(pParse, r, nPk);
            }
          }

          sqlite3VdbeAddOp2(v, 10, regReturn, iLoopBody);

          if (jmp1)
            sqlite3VdbeJumpHere(v, jmp1);

          if (pSubWInfo->untestedTerms)
            untestedTerms = 1;

          pSubLoop = pSubWInfo->a[0].pWLoop;

          if ((pSubLoop->wsFlags & 0x00000200) != 0 && (ii == 0 || pSubLoop->u.btree.pIndex == pCov) &&
              ((((pTab)->tabFlags & 0x00000080) == 0) || !((pSubLoop->u.btree.pIndex)->idxType == 2))) {
            pCov = pSubLoop->u.btree.pIndex;
          } else {
            pCov = 0;
          }
          if (sqlite3WhereUsesDeferredSeek(pSubWInfo)) {
            pWInfo->bDeferredSeek = 1;
          }

          sqlite3WhereEnd(pSubWInfo);
          sqlite3VdbeExplainPop(pParse);
        }
        sqlite3ExprDelete(db, pDelete);
      }
    }
    sqlite3VdbeExplainPop(pParse);

    pLevel->u.pCoveringIdx = pCov;
    if (pCov)
      pLevel->iIdxCur = iCovCur;
    if (pAndExpr) {
      pAndExpr->pLeft = 0;
      sqlite3ExprDelete(db, pAndExpr);
    }
    sqlite3VdbeChangeP1(v, iRetInit, sqlite3VdbeCurrentAddr(v));
    sqlite3VdbeGoto(v, pLevel->addrBrk);
    sqlite3VdbeResolveLabel(v, iLoopBody);

    pLevel->p2 = sqlite3VdbeCurrentAddr(v);

    if (pWInfo->pTabList != pOrTab) {
      sqlite3DbFreeNN(db, pOrTab);
    }
    if (!untestedTerms)
      disableTerm(pLevel, pTerm);
  } else {
    static const u8 aStep[] = {40, 39};
    static const u8 aStart[] = {36, 32};

    if (pTabItem->fg.isRecursive) {
      pLevel->op = 189;
    } else {
      pLevel->op = aStep[bRev];
      pLevel->p1 = iCur;
      pLevel->p2 = 1 + sqlite3VdbeAddOp2(v, aStart[bRev], iCur, pLevel->addrHalt);
      pLevel->p5 = SQLITE_STMTSTATUS_FULLSCAN_STEP;
    }
  }

  iLoop = (pIdx ? 1 : 2);
  do {
    int iNext = 0;
    for (pTerm = pWC->a, j = pWC->nTerm; j > 0; j--, pTerm++) {
      Expr *pE;
      int skipLikeAddr = 0;
      if (pTerm->wtFlags & (0x0002 | 0x0004))
        continue;
      if ((pTerm->prereqAll & pLevel->notReady) != 0) {
        pWInfo->untestedTerms = 1;
        continue;
      }
      pE = pTerm->pExpr;

      if (pTabItem->fg.jointype & (0x08 | 0x40 | 0x10)) {
        if (!(((pE)->flags & (u32)(0x000001 | 0x000002)) != 0)) {
          continue;
        } else if ((pTabItem->fg.jointype & 0x08) == 0x08 && !(((pE)->flags & (u32)(0x000001)) != 0)) {
          continue;
        } else {
          Bitmask m = sqlite3WhereGetMask(&pWInfo->sMaskSet, pE->w.iJoin);
          if (m & pLevel->notReady) {
            continue;
          }
        }
      }
      if (iLoop == 1 && !sqlite3ExprCoveredByIndex(pE, pLevel->iTabCur, pIdx)) {
        iNext = 2;
        continue;
      }
      if (iLoop < 3 && (pTerm->wtFlags & 0x1000)) {
        if (iNext == 0)
          iNext = 3;
        continue;
      }

      if ((pTerm->wtFlags & 0x0200) != 0) {
        u32 x = pLevel->iLikeRepCntr;
        if (x > 0) {
          skipLikeAddr = sqlite3VdbeAddOp1(v, (x & 1) ? 17 : 16, (int)(x >> 1));
        }
      }

      sqlite3ExprIfFalse(pParse, pE, addrCont, 0x10);
      if (skipLikeAddr)
        sqlite3VdbeJumpHere(v, skipLikeAddr);
      pTerm->wtFlags |= 0x0004;
    }
    iLoop = iNext;
  } while (iLoop > 0);

  for (pTerm = pWC->a, j = pWC->nBase; j > 0; j--, pTerm++) {
    Expr *pE, sEAlt;
    WhereTerm *pAlt;
    if (pTerm->wtFlags & (0x0002 | 0x0004))
      continue;
    if ((pTerm->eOperator & (0x0002 | 0x0080)) == 0)
      continue;
    if ((pTerm->eOperator & 0x0800) == 0)
      continue;
    if (pTerm->leftCursor != iCur)
      continue;
    if (pTabItem->fg.jointype & (0x08 | 0x40 | 0x10))
      continue;
    pE = pTerm->pExpr;

    pAlt = sqlite3WhereFindTerm(pWC, iCur, pTerm->u.x.leftColumn, notReady, 0x0002 | 0x0001 | 0x0080, 0);
    if (pAlt == 0)
      continue;
    if (pAlt->wtFlags & (0x0004))
      continue;
    if ((((pAlt->pExpr)->flags & (u32)(0x000200)) != 0))
      continue;
    if ((pAlt->eOperator & 0x0001) && (((pAlt->pExpr)->flags & 0x001000) != 0) &&
        (pAlt->pExpr->x.pSelect->pEList->nExpr > 1)) {
      continue;
    };
    sEAlt = *pAlt->pExpr;
    sEAlt.pLeft = pE->pLeft;
    sqlite3ExprIfFalse(pParse, &sEAlt, addrCont, 0x10);
    pAlt->wtFlags |= 0x0004;
  }

  if (pLevel->pRJ) {
    Table *pTab;
    int nPk;
    int r;
    int jmp1 = 0;
    WhereRightJoin *pRJ = pLevel->pRJ;

    pTab = pWInfo->pTabList->a[pLevel->iFrom].pSTab;
    if ((((pTab)->tabFlags & 0x00000080) == 0)) {
      r = sqlite3GetTempRange(pParse, 2);
      sqlite3ExprCodeGetColumnOfTable(v, pTab, pLevel->iTabCur, -1, r + 1);
      nPk = 1;
    } else {
      int iPk;
      Index *pPk = sqlite3PrimaryKeyIndex(pTab);
      nPk = pPk->nKeyCol;
      r = sqlite3GetTempRange(pParse, nPk + 1);
      for (iPk = 0; iPk < nPk; iPk++) {
        int iCol = pPk->aiColumn[iPk];
        sqlite3ExprCodeGetColumnOfTable(v, pTab, iCur, iCol, r + 1 + iPk);
      }
    }
    jmp1 = sqlite3VdbeAddOp4Int(v, 29, pRJ->iMatch, 0, r + 1, nPk);
    sqlite3VdbeAddOp3(v, 99, r + 1, nPk, r);
    sqlite3VdbeAddOp4Int(v, 140, pRJ->iMatch, r, r + 1, nPk);
    sqlite3VdbeAddOp4Int(v, 185, pRJ->regBloom, 0, r + 1, nPk);
    sqlite3VdbeChangeP5(v, 0x10);
    sqlite3VdbeJumpHere(v, jmp1);
    sqlite3ReleaseTempRange(pParse, r, nPk + 1);
  }

  if (pLevel->iLeftJoin) {
    pLevel->addrFirst = sqlite3VdbeCurrentAddr(v);
    sqlite3VdbeAddOp2(v, 73, 1, pLevel->iLeftJoin);
    if (pLevel->pRJ == 0) {
      goto code_outer_join_constraints;
    }
  }

  if (pLevel->pRJ) {
    {
      WhereRightJoin *pRJ = pLevel->pRJ;
      sqlite3VdbeAddOp2(v, 76, 0, pRJ->regReturn);
      pRJ->addrSubrtn = sqlite3VdbeCurrentAddr(v);
    }

    pParse->withinRJSubrtn++;

  code_outer_join_constraints:
    for (pTerm = pWC->a, j = 0; j < pWC->nBase; j++, pTerm++) {
      if (pTerm->wtFlags & (0x0002 | 0x0004))
        continue;
      if ((pTerm->prereqAll & pLevel->notReady) != 0) {
        continue;
      }
      if (pTabItem->fg.jointype & 0x40)
        continue;

      sqlite3ExprIfFalse(pParse, pTerm->pExpr, addrCont, 0x10);
      pTerm->wtFlags |= 0x0004;
    }
  }

  return pLevel->notReady;
}

u16 exprCommute(Parse *pParse, Expr *pExpr) {
  if (pExpr->pLeft->op == 177 || pExpr->pRight->op == 177 ||
      sqlite3BinaryCompareCollSeq(pParse, pExpr->pLeft, pExpr->pRight) !=
          sqlite3BinaryCompareCollSeq(pParse, pExpr->pRight, pExpr->pLeft)) {
    pExpr->flags ^= 0x000400;
  }
  {
    Expr *t = pExpr->pRight;
    pExpr->pRight = pExpr->pLeft;
    pExpr->pLeft = t;
  };
  if (pExpr->op >= 55) {
    pExpr->op = ((pExpr->op - 55) ^ 2) + 55;
  }
  return 0;
}

int isLikeOrGlob(Parse *pParse, Expr *pExpr, Expr **ppPrefix, int *pisComplete, int *pnoCase) {
  const u8 *z = 0;
  Expr *pRight, *pLeft;
  ExprList *pList;
  u8 c;
  int cnt;
  u8 wc[4];
  sqlite3 *db = pParse->db;
  sqlite3_value *pVal = 0;
  int op;
  int rc;

  if (!sqlite3IsLikeFunction(db, pExpr, pnoCase, (char *)wc)) {
    return 0;
  }

  pList = pExpr->x.pList;
  pLeft = pList->a[1].pExpr;

  pRight = sqlite3ExprSkipCollate(pList->a[0].pExpr);
  op = pRight->op;
  if (op == 157 && (db->flags & 0x00800000) == 0) {
    Vdbe *pReprepare = pParse->pReprepare;
    int iCol = pRight->iColumn;
    pVal = sqlite3VdbeGetBoundValue(pReprepare, iCol, 0x41);
    if (pVal && sqlite3_value_type(pVal) == 3) {
      z = sqlite3_value_text(pVal);
    }
    sqlite3VdbeSetVarmask(pParse->pVdbe, iCol);

  } else if (op == 118) {
    z = (u8 *)pRight->u.zToken;
  }
  if (z) {
    cnt = 0;
    while ((c = z[cnt]) != 0 && c != wc[0] && c != wc[1] && c != wc[2]) {
      cnt++;
      if (c == wc[3] && z[cnt] > 0 && z[cnt] < 0x80) {
        cnt++;
      } else if (c >= 0x80) {
        const u8 *z2 = z + cnt - 1;
        if (c == 0xff || sqlite3Utf8Read(&z2) == 0xfffd || ((db)->enc) == SQLITE_UTF16LE) {
          cnt--;
          break;
        } else {
          cnt = (int)(z2 - z);
        }
      }
    }

    if ((cnt > 1 || (cnt > 0 && z[0] != wc[3])) && (255 != (u8)z[cnt - 1])) {
      Expr *pPrefix;

      *pisComplete = c == wc[0] && z[cnt + 1] == 0 && ((db)->enc) != SQLITE_UTF16LE;

      pPrefix = sqlite3Expr(db, 118, (char *)z);
      if (pPrefix) {
        int iFrom, iTo;
        char *zNew;

        zNew = pPrefix->u.zToken;
        zNew[cnt] = 0;
        for (iFrom = iTo = 0; iFrom < cnt; iFrom++) {
          if (zNew[iFrom] == wc[3])
            iFrom++;
          zNew[iTo++] = zNew[iFrom];
        }
        zNew[iTo] = 0;

        if (pLeft->op != 168 || sqlite3ExprAffinity(pLeft) != 0x42 ||
            (((((pLeft)->flags & (0x1000000 | 0x2000000)) == 0)) && (pLeft->y.pTab) &&
             ((pLeft->y.pTab)->eTabType == 1))) {
          int isNum;
          double rDummy;

          isNum = sqlite3AtoF(zNew, &rDummy);
          if (isNum <= 0) {
            if (iTo == 1 && zNew[0] == '-') {
              isNum = +1;
            } else {
              zNew[iTo - 1]++;
              isNum = sqlite3AtoF(zNew, &rDummy);
              zNew[iTo - 1]--;
            }
          }
          if (isNum > 0) {
            sqlite3ExprDelete(db, pPrefix);
            sqlite3ValueFree(pVal);
            return 0;
          }
        }
      }
      *ppPrefix = pPrefix;

      if (op == 157) {
        Vdbe *v = pParse->pVdbe;
        sqlite3VdbeSetVarmask(v, pRight->iColumn);

        if (*pisComplete && pRight->u.zToken[1]) {
          int r1 = sqlite3GetTempReg(pParse);
          sqlite3ExprCodeTarget(pParse, pRight, r1);
          sqlite3VdbeChangeP3(v, sqlite3VdbeCurrentAddr(v) - 1, 0);
          sqlite3ReleaseTempReg(pParse, r1);
        }
      }
    } else {
      z = 0;
    }
  }

  rc = (z != 0);
  sqlite3ValueFree(pVal);
  return rc;
}

int termIsEquivalence(Parse *pParse, Expr *pExpr, SrcList *pSrc) {
  char aff1, aff2;
  if (!(((pParse->db)->dbOptFlags & (0x00000080)) == 0))
    return 0;
  if (pExpr->op != 54 && pExpr->op != 45)
    return 0;
  if ((((pExpr)->flags & (u32)(0x000001 | 0x000200)) != 0))
    return 0;

  if (pExpr->op == 45 && pSrc->nSrc >= 2 && (pSrc->a[0].fg.jointype & 0x40) != 0) {
    return 0;
  }
  aff1 = sqlite3ExprAffinity(pExpr->pLeft);
  aff2 = sqlite3ExprAffinity(pExpr->pRight);
  if (aff1 != aff2 && (!((aff1) >= 0x43) || !((aff2) >= 0x43))) {
    return 0;
  }
  if (!sqlite3ExprCollSeqMatch(pParse, pExpr->pLeft, pExpr->pRight)) {
    return 0;
  }
  return 1;
}

void sqlite3WhereTabFuncArgs(Parse *pParse, SrcItem *pItem, WhereClause *pWC) {
  Table *pTab;
  int j, k;
  ExprList *pArgs;
  Expr *pColRef;
  Expr *pTerm;
  if (pItem->fg.isTabFunc == 0)
    return;
  pTab = pItem->pSTab;

  pArgs = pItem->u1.pFuncArg;
  if (pArgs == 0)
    return;
  for (j = k = 0; j < pArgs->nExpr; j++) {
    Expr *pRhs;
    u32 joinType;
    while (k < pTab->nCol && (pTab->aCol[k].colFlags & 0x0002) == 0) {
      k++;
    }
    if (k >= pTab->nCol) {
      sqlite3ErrorMsg(pParse, "too many arguments on %s() - max %d", pTab->zName, j);
      return;
    }
    pColRef = sqlite3ExprAlloc(pParse->db, 168, 0, 0);
    if (pColRef == 0)
      return;
    pColRef->iTable = pItem->iCursor;
    pColRef->iColumn = k++;

    pColRef->y.pTab = pTab;
    pItem->colUsed |= sqlite3ExprColUsed(pColRef);
    pRhs = sqlite3PExpr(pParse, 173, sqlite3ExprDup(pParse->db, pArgs->a[j].pExpr, 0), 0);
    pTerm = sqlite3PExpr(pParse, 54, pColRef, pRhs);
    if (pItem->fg.jointype & (0x08 | 0x10)) {
      joinType = 0x000001;
    } else {
      joinType = 0x000002;
    }
    sqlite3SetJoinExpr(pTerm, pItem->iCursor, joinType);
    whereClauseInsert(pWC, pTerm, 0x0001);
  }
}

__attribute__((noinline)) const char *indexInAffinityOk(Parse *pParse, WhereTerm *pTerm, u8 idxaff) {
  Expr *pX = pTerm->pExpr;
  Expr inexpr;

  if (sqlite3ExprIsVector(pX->pLeft)) {
    int iField = pTerm->u.x.iField - 1;
    inexpr.flags = 0;
    inexpr.op = 54;
    inexpr.pLeft = pX->pLeft->x.pList->a[iField].pExpr;

    inexpr.pRight = pX->x.pSelect->pEList->a[iField].pExpr;
    pX = &inexpr;
  }

  if (sqlite3IndexAffinityOk(pX, idxaff)) {
    CollSeq *pRet = sqlite3ExprCompareCollSeq(pParse, pX);
    return pRet ? pRet->zName : sqlite3StrBINARY;
  }
  return 0;
}

int findIndexCol(Parse *pParse, ExprList *pList, int iBase, Index *pIdx, int iCol) {
  int i;
  const char *zColl = pIdx->azColl[iCol];

  for (i = 0; i < pList->nExpr; i++) {
    Expr *p = sqlite3ExprSkipCollateAndLikely(pList->a[i].pExpr);
    if ((p != 0) && (p->op == 168 || p->op == 170) && p->iColumn == pIdx->aiColumn[iCol] && p->iTable == iBase) {
      CollSeq *pColl = sqlite3ExprNNCollSeq(pParse, pList->a[i].pExpr);
      if (0 == sqlite3StrICmp(pColl->zName, zColl)) {
        return i;
      }
    }
  }

  return -1;
}

int isDistinctRedundant(Parse *pParse, SrcList *pTabList, WhereClause *pWC, ExprList *pDistinct) {
  Table *pTab;
  Index *pIdx;
  int i;
  int iBase;

  if (pTabList->nSrc != 1)
    return 0;
  iBase = pTabList->a[0].iCursor;
  pTab = pTabList->a[0].pSTab;

  for (i = 0; i < pDistinct->nExpr; i++) {
    Expr *p = sqlite3ExprSkipCollateAndLikely(pDistinct->a[i].pExpr);
    if (p == 0)
      continue;
    if (p->op != 168 && p->op != 170)
      continue;
    if (p->iTable == iBase && p->iColumn < 0)
      return 1;
  }

  for (pIdx = pTab->pIndex; pIdx; pIdx = pIdx->pNext) {
    if (!((pIdx)->onError != 0))
      continue;
    if (pIdx->pPartIdxWhere)
      continue;
    for (i = 0; i < pIdx->nKeyCol; i++) {
      if (0 == sqlite3WhereFindTerm(pWC, iBase, i, ~(Bitmask)0, 0x0002, pIdx)) {
        if (findIndexCol(pParse, pDistinct, iBase, pIdx, i) < 0)
          break;
        if (indexColumnNotNull(pIdx, i) == 0)
          break;
      }
    }
    if (i == pIdx->nKeyCol) {
      return 1;
    }
  }

  return 0;
}

void translateColumnToCopy(Parse *pParse, int iStart, int iTabCur, int iRegister, int iAutoidxCur) {
  Vdbe *v = pParse->pVdbe;
  VdbeOp *pOp = sqlite3VdbeGetOp(v, iStart);
  int iEnd = sqlite3VdbeCurrentAddr(v);
  if (pParse->db->mallocFailed)
    return;

  for (; iStart < iEnd; iStart++, pOp++) {
    if (pOp->p1 != iTabCur)
      continue;
    if (pOp->opcode == 96) {
      pOp->opcode = 82;
      pOp->p1 = pOp->p2 + iRegister;
      pOp->p2 = pOp->p3;
      pOp->p3 = 0;
      pOp->p5 = 2;
    } else if (pOp->opcode == 137) {
      pOp->opcode = 128;
      pOp->p1 = iAutoidxCur;
    }
  }
}

__attribute__((noinline)) void constructAutomaticIndex(Parse *pParse, WhereClause *pWC, const Bitmask notReady,
                                                       WhereLevel *pLevel) {
  int nKeyCol;
  WhereTerm *pTerm;
  WhereTerm *pWCEnd;
  Index *pIdx;
  Vdbe *v;
  int addrInit;
  Table *pTable;
  int addrTop;
  int regRecord;
  int n;
  int i;
  int mxBitCol;
  CollSeq *pColl;
  WhereLoop *pLoop;
  char *zNotUsed;
  Bitmask idxCols;
  Bitmask extraCols;
  u8 sentWarning = 0;
  u8 useBloomFilter = 0;
  Expr *pPartial = 0;
  int iContinue = 0;
  SrcList *pTabList;
  SrcItem *pSrc;
  int addrCounter = 0;
  int regBase;

  v = pParse->pVdbe;

  addrInit = sqlite3VdbeAddOp0(v, 15);

  nKeyCol = 0;
  pTabList = pWC->pWInfo->pTabList;
  pSrc = &pTabList->a[pLevel->iFrom];
  pTable = pSrc->pSTab;
  pWCEnd = &pWC->a[pWC->nTerm];
  pLoop = pLevel->pWLoop;
  idxCols = 0;
  for (pTerm = pWC->a; pTerm < pWCEnd; pTerm++) {
    Expr *pExpr = pTerm->pExpr;

    if ((pTerm->wtFlags & 0x0002) == 0 && sqlite3ExprIsSingleTableConstraint(pExpr, pTabList, pLevel->iFrom, 0)) {
      pPartial = sqlite3ExprAnd(pParse, pPartial, sqlite3ExprDup(pParse->db, pExpr, 0));
    }
    if (termCanDriveIndex(pTerm, pSrc, notReady)) {
      int iCol;
      Bitmask cMask;

      iCol = pTerm->u.x.leftColumn;
      cMask = iCol >= ((int)(sizeof(Bitmask) * 8)) ? (((Bitmask)1) << (((int)(sizeof(Bitmask) * 8)) - 1))
                                                   : (((Bitmask)1) << (iCol));
      if (!sentWarning) {
        sqlite3_log((28 | (1 << 8)), "automatic index on %s(%s)", pTable->zName, pTable->aCol[iCol].zCnName);
        sentWarning = 1;
      }
      if ((idxCols & cMask) == 0) {
        if (whereLoopResize(pParse->db, pLoop, nKeyCol + 1)) {
          goto end_auto_index_create;
        }
        pLoop->aLTerm[nKeyCol++] = pTerm;
        idxCols |= cMask;
      }
    }
  }

  pLoop->u.btree.nEq = pLoop->nLTerm = nKeyCol;
  pLoop->wsFlags = 0x00000001 | 0x00000040 | 0x00000200 | 0x00004000;

  if ((pTable)->eTabType == 2) {
    extraCols = ((Bitmask)-1) & ~idxCols;
  } else {
    extraCols = pSrc->colUsed & (~idxCols | (((Bitmask)1) << (((int)(sizeof(Bitmask) * 8)) - 1)));
  }
  if (!(((pTable)->tabFlags & 0x00000080) == 0)) {
    for (i = 0; i < pTable->nCol; i++) {
      if ((pTable->aCol[i].colFlags & 0x0001) == 0)
        continue;
      if (i >= ((int)(sizeof(Bitmask) * 8)) - 1) {
        extraCols |= (((Bitmask)1) << (((int)(sizeof(Bitmask) * 8)) - 1));
        break;
      }
      if (idxCols & (((Bitmask)1) << (i)))
        continue;
      extraCols |= (((Bitmask)1) << (i));
    }
  }
  mxBitCol =
      ((((int)(sizeof(Bitmask) * 8)) - 1) < (pTable->nCol) ? (((int)(sizeof(Bitmask) * 8)) - 1) : (pTable->nCol));
  for (i = 0; i < mxBitCol; i++) {
    if (extraCols & (((Bitmask)1) << (i)))
      nKeyCol++;
  }
  if (pSrc->colUsed & (((Bitmask)1) << (((int)(sizeof(Bitmask) * 8)) - 1))) {
    nKeyCol += pTable->nCol - ((int)(sizeof(Bitmask) * 8)) + 1;
  }

  pIdx = sqlite3AllocateIndexObject(pParse->db, nKeyCol + (((pTable)->tabFlags & 0x00000080) == 0), 0, &zNotUsed);
  if (pIdx == 0)
    goto end_auto_index_create;
  pLoop->u.btree.pIndex = pIdx;
  pIdx->zName = (char*)("auto-index");
  pIdx->pTable = pTable;
  n = 0;
  idxCols = 0;
  for (pTerm = pWC->a; pTerm < pWCEnd; pTerm++) {
    if (termCanDriveIndex(pTerm, pSrc, notReady)) {
      int iCol;
      Bitmask cMask;

      iCol = pTerm->u.x.leftColumn;
      cMask = iCol >= ((int)(sizeof(Bitmask) * 8)) ? (((Bitmask)1) << (((int)(sizeof(Bitmask) * 8)) - 1))
                                                   : (((Bitmask)1) << (iCol));
      if ((idxCols & cMask) == 0) {
        Expr *pX = pTerm->pExpr;
        idxCols |= cMask;
        pIdx->aiColumn[n] = pTerm->u.x.leftColumn;
        pColl = sqlite3ExprCompareCollSeq(pParse, pX);

        pIdx->azColl[n] = pColl ? pColl->zName : sqlite3StrBINARY;
        n++;
        if ((pX->pLeft != 0) && sqlite3ExprAffinity(pX->pLeft) != 0x42) {
          useBloomFilter = 1;
        }
      }
    }
  }

  for (i = 0; i < mxBitCol; i++) {
    if (extraCols & (((Bitmask)1) << (i))) {
      pIdx->aiColumn[n] = i;
      pIdx->azColl[n] = sqlite3StrBINARY;
      n++;
    }
  }
  if (pSrc->colUsed & (((Bitmask)1) << (((int)(sizeof(Bitmask) * 8)) - 1))) {
    for (i = ((int)(sizeof(Bitmask) * 8)) - 1; i < pTable->nCol; i++) {
      pIdx->aiColumn[n] = i;
      pIdx->azColl[n] = sqlite3StrBINARY;
      n++;
    }
  }

  if ((((pTable)->tabFlags & 0x00000080) == 0)) {
    pIdx->aiColumn[n] = (-1);
    pIdx->azColl[n] = sqlite3StrBINARY;
  }

  pLevel->iIdxCur = pParse->nTab++;
  sqlite3VdbeAddOp2(v, 119, pLevel->iIdxCur, nKeyCol + 1);
  sqlite3VdbeSetP4KeyInfo(pParse, pIdx);
  if ((((pParse->db)->dbOptFlags & (0x00080000)) == 0) && useBloomFilter) {
    sqlite3WhereExplainBloomFilter(pParse, pWC->pWInfo, pLevel);
    pLevel->regFilter = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, 79, 10000, pLevel->regFilter);
  }

  if (pSrc->fg.viaCoroutine) {
    int regYield;
    Subquery *pSubq;

    pSubq = pSrc->u4.pSubq;

    regYield = pSubq->regReturn;
    addrCounter = sqlite3VdbeAddOp2(v, 73, 0, 0);
    sqlite3VdbeAddOp3(v, 11, regYield, 0, pSubq->addrFillSub);
    addrTop = sqlite3VdbeAddOp1(v, 12, regYield);
  } else {
    addrTop = sqlite3VdbeAddOp2(v, 36, pLevel->iTabCur, pLevel->addrHalt);
  }
  if (pPartial) {
    iContinue = sqlite3VdbeMakeLabel(pParse);
    sqlite3ExprIfFalse(pParse, pPartial, iContinue, 0x10);
    pLoop->wsFlags |= 0x00020000;
  }
  regRecord = sqlite3GetTempReg(pParse);
  regBase = sqlite3GenerateIndexKey(pParse, pIdx, pLevel->iTabCur, regRecord, 0, 0, 0, 0);
  if (pLevel->regFilter) {
    sqlite3VdbeAddOp4Int(v, 185, pLevel->regFilter, 0, regBase, pLoop->u.btree.nEq);
  };
  sqlite3VdbeAddOp2(v, 140, pLevel->iIdxCur, regRecord);
  sqlite3VdbeChangeP5(v, 0x10);
  if (pPartial)
    sqlite3VdbeResolveLabel(v, iContinue);
  if (pSrc->fg.viaCoroutine) {
    sqlite3VdbeChangeP2(v, addrCounter, regBase + n);

    translateColumnToCopy(pParse, addrTop, pLevel->iTabCur, pSrc->u4.pSubq->regResult, pLevel->iIdxCur);
    sqlite3VdbeGoto(v, addrTop);
    pSrc->fg.viaCoroutine = 0;
    sqlite3VdbeJumpHere(v, addrTop);
  } else {
    sqlite3VdbeAddOp2(v, 40, pLevel->iTabCur, addrTop + 1);
    sqlite3VdbeChangeP5(v, SQLITE_STMTSTATUS_AUTOINDEX);
    if ((pSrc->fg.jointype & 0x08) != 0) {
      sqlite3VdbeJumpHere(v, addrTop);
    }
  }
  sqlite3ReleaseTempReg(pParse, regRecord);

  sqlite3VdbeJumpHere(v, addrInit);

end_auto_index_create:
  sqlite3ExprDelete(pParse->db, pPartial);
}

int vtabBestIndex(Parse *pParse, Table *pTab, sqlite3_index_info *p) {
  int rc;
  sqlite3_vtab *pVtab;

  pVtab = sqlite3GetVTable(pParse->db, pTab)->pVtab;
  pParse->db->nSchemaLock++;
  rc = pVtab->pModule->xBestIndex(pVtab, p);
  pParse->db->nSchemaLock--;

  if (rc != SQLITE_OK && rc != SQLITE_CONSTRAINT) {
    if (rc == SQLITE_NOMEM) {
      sqlite3OomFault(pParse->db);
    } else if (!pVtab->zErrMsg) {
      sqlite3ErrorMsg(pParse, "%s", sqlite3ErrStr(rc));
    } else {
      sqlite3ErrorMsg(pParse, "%s", pVtab->zErrMsg);
    }
  }
  if (pTab->u.vtab.p->bAllSchemas) {
    sqlite3VtabUsesAllSchemas(pParse);
  }
  sqlite3_free(pVtab->zErrMsg);
  pVtab->zErrMsg = 0;
  return rc;
}

int whereRangeScanEst(Parse *pParse, WhereLoopBuilder *pBuilder, WhereTerm *pLower, WhereTerm *pUpper,
                      WhereLoop *pLoop) {
  int rc = SQLITE_OK;
  int nOut = pLoop->nOut;
  LogEst nNew;

  (void)(pParse);
  (void)(pBuilder);

  nNew = whereRangeAdjust(pLower, nOut);
  nNew = whereRangeAdjust(pUpper, nNew);

  if (pLower && pLower->truthProb > 0 && pUpper && pUpper->truthProb > 0) {
    nNew -= 20;
  }

  nOut -= (pLower != 0) + (pUpper != 0);
  if (nNew < 10)
    nNew = 10;
  if (nNew < nOut)
    nOut = nNew;

  pLoop->nOut = (LogEst)nOut;
  return rc;
}

int whereRangeVectorLen(Parse *pParse, int iCur, Index *pIdx, int nEq, WhereTerm *pTerm) {
  int nCmp = sqlite3ExprVectorSize(pTerm->pExpr->pLeft);
  int i;

  nCmp = ((nCmp) < ((pIdx->nColumn - nEq)) ? (nCmp) : ((pIdx->nColumn - nEq)));
  for (i = 1; i < nCmp; i++) {
    char aff;
    char idxaff = 0;
    CollSeq *pColl;
    Expr *pLhs, *pRhs;

    pLhs = pTerm->pExpr->pLeft->x.pList->a[i].pExpr;
    pRhs = pTerm->pExpr->pRight;
    if ((((pRhs)->flags & 0x001000) != 0)) {
      pRhs = pRhs->x.pSelect->pEList->a[i].pExpr;
    } else {
      pRhs = pRhs->x.pList->a[i].pExpr;
    }

    if (pLhs->op != 168 || pLhs->iTable != iCur || pLhs->iColumn != pIdx->aiColumn[i + nEq] ||
        pIdx->aSortOrder[i + nEq] != pIdx->aSortOrder[nEq]) {
      break;
    }

    aff = sqlite3CompareAffinity(pRhs, sqlite3ExprAffinity(pLhs));
    idxaff = sqlite3TableColumnAffinity(pIdx->pTable, pLhs->iColumn);
    if (aff != idxaff)
      break;

    if ((((pTerm->pExpr)->flags & (u32)(0x000400)) != 0)) {
      Expr *t = pRhs;
      pRhs = pLhs;
      pLhs = t;
    };
    pColl = sqlite3BinaryCompareCollSeq(pParse, pLhs, pRhs);
    if (pColl == 0)
      break;
    if (sqlite3StrICmp(pColl->zName, pIdx->azColl[i + nEq]))
      break;
  }
  return i;
}

void wherePartIdxExpr(Parse *pParse, Index *pIdx, Expr *pPart, Bitmask *pMask, int iIdxCur, SrcItem *pItem) {
  if (pPart->op == 44) {
    wherePartIdxExpr(pParse, pIdx, pPart->pRight, pMask, iIdxCur, pItem);
    pPart = pPart->pLeft;
  }

  if ((pPart->op == 54 || pPart->op == 45)) {
    Expr *pLeft = pPart->pLeft;
    Expr *pRight = pPart->pRight;
    u8 aff;

    if (pLeft->op != 168)
      return;
    if (!sqlite3ExprIsConstant(0, pRight))
      return;
    if (!sqlite3IsBinary(sqlite3ExprCompareCollSeq(pParse, pPart)))
      return;
    if (pLeft->iColumn < 0)
      return;
    aff = pIdx->pTable->aCol[pLeft->iColumn].affinity;
    if (aff >= 0x42) {
      if (pItem) {
        sqlite3 *db = pParse->db;
        IndexedExpr *p = (IndexedExpr *)sqlite3DbMallocRaw(db, sizeof(*p));
        if (p) {
          int bNullRow = (pItem->fg.jointype & (0x08 | 0x40)) != 0;
          p->pExpr = sqlite3ExprDup(db, pRight, 0);
          p->iDataCur = pItem->iCursor;
          p->iIdxCur = iIdxCur;
          p->iIdxCol = pLeft->iColumn;
          p->bMaybeNullRow = bNullRow;
          p->pIENext = pParse->pIdxPartExpr;
          p->aff = aff;
          pParse->pIdxPartExpr = p;
          if (p->pIENext == 0) {
            void *pArg = (void *)&pParse->pIdxPartExpr;
            sqlite3ParserAddCleanup(pParse, whereIndexedExprCleanup, pArg);
          }
        }
      } else if (pLeft->iColumn < (((int)(sizeof(Bitmask) * 8)) - 1)) {
        *pMask &= ~((Bitmask)1 << pLeft->iColumn);
      }
    }
  }
}

void sqlite3VtabUsesAllSchemas(Parse *pParse) {
  int nDb = pParse->db->nDb;
  int i;
  for (i = 0; i < nDb; i++) {
    sqlite3CodeVerifySchema(pParse, i);
  }
  if (((pParse->writeMask) != 0)) {
    for (i = 0; i < nDb; i++) {
      sqlite3BeginWriteOperation(pParse, 0, i);
    }
  }
}

__attribute__((noinline)) void whereAddIndexedExpr(Parse *pParse, Index *pIdx, int iIdxCur, SrcItem *pTabItem) {
  int i;
  IndexedExpr *p;
  Table *pTab;

  pTab = pIdx->pTable;
  for (i = 0; i < pIdx->nColumn; i++) {
    Expr *pExpr;
    int j = pIdx->aiColumn[i];
    if (j == (-2)) {
      pExpr = pIdx->aColExpr->a[i].pExpr;
    } else if (j >= 0 && (pTab->aCol[j].colFlags & 0x0020) != 0) {
      pExpr = sqlite3ColumnExpr(pTab, &pTab->aCol[j]);
    } else {
      continue;
    }
    if (sqlite3ExprIsConstant(0, pExpr))
      continue;
    p = (IndexedExpr*)(sqlite3DbMallocRaw(pParse->db, sizeof(IndexedExpr)));
    if (p == 0)
      break;
    p->pIENext = pParse->pIdxEpr;

    p->pExpr = sqlite3ExprDup(pParse->db, pExpr, 0);
    p->iDataCur = pTabItem->iCursor;
    p->iIdxCur = iIdxCur;
    p->iIdxCol = i;
    p->bMaybeNullRow = (pTabItem->fg.jointype & (0x08 | 0x40 | 0x10)) != 0;
    if (sqlite3IndexAffinityStr(pParse->db, pIdx)) {
      p->aff = pIdx->zColAff[i];
    }

    pParse->pIdxEpr = p;
    if (p->pIENext == 0) {
      void *pArg = (void *)&pParse->pIdxEpr;
      sqlite3ParserAddCleanup(pParse, whereIndexedExprCleanup, pArg);
    }
  }
}

WhereInfo *sqlite3WhereBegin(Parse *pParse, SrcList *pTabList, Expr *pWhere, ExprList *pOrderBy, ExprList *pResultSet,
                             Select *pSelect, u16 wctrlFlags, int iAuxArg) {
  int nByteWInfo;
  int nTabList;
  WhereInfo *pWInfo;
  Vdbe *v = pParse->pVdbe;
  Bitmask notReady;
  WhereLoopBuilder sWLB;
  WhereMaskSet *pMaskSet;
  WhereLevel *pLevel;
  WhereLoop *pLoop;
  int ii;
  sqlite3 *db;
  int rc;
  u8 bFordelete = 0;

  db = pParse->db;
  memset(&sWLB, 0, sizeof(sWLB));

  if (pOrderBy && pOrderBy->nExpr >= ((int)(sizeof(Bitmask) * 8))) {
    pOrderBy = 0;
    wctrlFlags &= ~0x0100;
    wctrlFlags |= 0x2000;
  }

  if (pTabList->nSrc > ((int)(sizeof(Bitmask) * 8))) {
    sqlite3ErrorMsg(pParse, "at most %d tables in a join", ((int)(sizeof(Bitmask) * 8)));
    return 0;
  }

  nTabList = (wctrlFlags & 0x0020) ? 1 : pTabList->nSrc;

  nByteWInfo = (((offsetof(WhereInfo, a) + (nTabList) * sizeof(WhereLevel)) + 7) & ~7);
  pWInfo = (WhereInfo*)(sqlite3DbMallocRawNN(db, nByteWInfo + sizeof(WhereLoop)));
  if (db->mallocFailed) {
    sqlite3DbFree(db, pWInfo);
    pWInfo = 0;
    goto whereBeginError;
  }
  pWInfo->pParse = pParse;
  pWInfo->pTabList = pTabList;
  pWInfo->pOrderBy = pOrderBy;

  pWInfo->pResultSet = pResultSet;
  pWInfo->aiCurOnePass[0] = pWInfo->aiCurOnePass[1] = -1;
  pWInfo->nLevel = nTabList;
  pWInfo->iBreak = pWInfo->iContinue = sqlite3VdbeMakeLabel(pParse);
  pWInfo->wctrlFlags = wctrlFlags;
  pWInfo->iLimit = iAuxArg;
  pWInfo->savedNQueryLoop = pParse->nQueryLoop;
  pWInfo->pSelect = pSelect;
  memset(&pWInfo->nOBSat, 0, offsetof(WhereInfo, sWC) - offsetof(WhereInfo, nOBSat));
  memset(&pWInfo->a[0], 0, sizeof(WhereLoop) + nTabList * sizeof(WhereLevel));

  pMaskSet = &pWInfo->sMaskSet;
  pMaskSet->n = 0;
  pMaskSet->ix[0] = -99;

  sWLB.pWInfo = pWInfo;
  sWLB.pWC = &pWInfo->sWC;
  sWLB.pNew = (WhereLoop *)(((char *)pWInfo) + nByteWInfo);

  whereLoopInit(sWLB.pNew);

  sqlite3WhereClauseInit(&pWInfo->sWC, pWInfo);
  sqlite3WhereSplit(&pWInfo->sWC, pWhere, 44);

  if (nTabList == 0) {
    if (pOrderBy)
      pWInfo->nOBSat = pOrderBy->nExpr;
    if ((wctrlFlags & 0x0100) != 0 && (((db)->dbOptFlags & (0x00000010)) == 0)) {
      pWInfo->eDistinct = 1;
    }
    if ((pWInfo->pSelect) && (pWInfo->pSelect->selFlags & 0x0000400) == 0) {
      sqlite3VdbeExplain(pParse, 0, "SCAN CONSTANT ROW");
    }
  } else {
    ii = 0;
    do {
      createMask(pMaskSet, pTabList->a[ii].iCursor);
      sqlite3WhereTabFuncArgs(pParse, &pTabList->a[ii], &pWInfo->sWC);
    } while ((++ii) < pTabList->nSrc);
  }

  sqlite3WhereExprAnalyze(pTabList, &pWInfo->sWC);
  if (pSelect && pSelect->pLimit) {
    sqlite3WhereAddLimit(&pWInfo->sWC, pSelect);
  }
  if (pParse->nErr)
    goto whereBeginError;

  for (ii = 0; ii < sWLB.pWC->nBase; ii++) {
    WhereTerm *pT = &sWLB.pWC->a[ii];
    Expr *pX;
    if (pT->wtFlags & 0x0002)
      continue;
    pX = pT->pExpr;

    if (pT->prereqAll == 0 && (nTabList == 0 || exprIsDeterministic(pX)) &&
        !((((pX)->flags & (u32)(0x000002)) != 0) && (pTabList->a[0].fg.jointype & 0x40) != 0)) {
      sqlite3ExprIfFalse(pParse, pX, pWInfo->iBreak, 0x10);
      pT->wtFlags |= 0x0004;
    }
  }

  if (wctrlFlags & 0x0100) {
    if ((((db)->dbOptFlags & (0x00000010)) != 0)) {
      wctrlFlags &= ~0x0100;
      pWInfo->wctrlFlags &= ~0x0100;
    } else if (isDistinctRedundant(pParse, pTabList, &pWInfo->sWC, pResultSet)) {
      pWInfo->eDistinct = 1;
    } else if (pOrderBy == 0) {
      pWInfo->wctrlFlags |= 0x0080;
      pWInfo->pOrderBy = pResultSet;
    }
  }

  if (nTabList != 1 || whereShortCut(&sWLB) == 0) {
    rc = whereLoopAddAll(&sWLB);
    if (rc)
      goto whereBeginError;

    wherePathSolver(pWInfo, 0);
    if (db->mallocFailed)
      goto whereBeginError;
    if (pWInfo->pOrderBy) {
      whereInterstageHeuristic(pWInfo);
      wherePathSolver(pWInfo, pWInfo->nRowOut < 0 ? 1 : pWInfo->nRowOut + 1);
      if (db->mallocFailed)
        goto whereBeginError;
    }

    if ((pWInfo->wctrlFlags & 0x0100) != 0) {
      pWInfo->nRowOut -= 30;
    }
  }

  if (pWInfo->pOrderBy == 0 && (db->flags & 0x00001000) != 0) {
    whereReverseScanOrder(pWInfo);
  }
  if (pParse->nErr) {
    goto whereBeginError;
  }

  notReady = ~(Bitmask)0;
  if (pWInfo->nLevel >= 2 && pResultSet != 0 && 0 == (wctrlFlags & (0x0400 | 0x2000)) &&
      (((db)->dbOptFlags & (0x00000100)) == 0)) {
    notReady = whereOmitNoopJoin(pWInfo, notReady);
    nTabList = pWInfo->nLevel;
  }

  if (pWInfo->nLevel >= 2 && (((db)->dbOptFlags & (0x00080000)) == 0)) {
    whereCheckIfBloomFilterIsUseful(pWInfo);
  }

  pWInfo->pParse->nQueryLoop += pWInfo->nRowOut;

  if ((wctrlFlags & 0x0004) != 0) {
    int wsFlags = pWInfo->a[0].pWLoop->wsFlags;
    int bOnerow = (wsFlags & 0x00001000) != 0;

    if (bOnerow ||
        (0 != (wctrlFlags & 0x0008) && !((pTabList->a[0].pSTab)->eTabType == 1) &&
         (0 == (wsFlags & 0x00002000) || (wctrlFlags & 0x0010)) && (((db)->dbOptFlags & (0x08000000)) == 0))) {
      pWInfo->eOnePass = bOnerow ? 1 : 2;
      if ((((pTabList->a[0].pSTab)->tabFlags & 0x00000080) == 0) && (wsFlags & 0x00000040)) {
        if (wctrlFlags & 0x0008) {
          bFordelete = 0x08;
        }
        pWInfo->a[0].pWLoop->wsFlags = (wsFlags & ~0x00000040);
      }
    }
  }

  for (ii = 0, pLevel = pWInfo->a; ii < nTabList; ii++, pLevel++) {
    Table *pTab;
    int iDb;
    SrcItem *pTabItem;

    pTabItem = &pTabList->a[pLevel->iFrom];
    pTab = pTabItem->pSTab;
    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    pLoop = pLevel->pWLoop;
    pLevel->addrBrk = sqlite3VdbeMakeLabel(pParse);
    if (ii == 0 || (pTabItem[0].fg.jointype & 0x08) != 0) {
      pLevel->addrHalt = pLevel->addrBrk;
    } else if (pWInfo->a[ii - 1].pRJ) {
      pLevel->addrHalt = pWInfo->a[ii - 1].addrBrk;
    } else {
      pLevel->addrHalt = pWInfo->a[ii - 1].addrHalt;
    }
    if ((pTab->tabFlags & 0x00004000) != 0 || ((pTab)->eTabType == 2)) {
    } else if ((pLoop->wsFlags & 0x00000400) != 0) {
      const char *pVTab = (const char *)sqlite3GetVTable(db, pTab);
      int iCur = pTabItem->iCursor;
      sqlite3VdbeAddOp4(v, 175, iCur, 0, 0, pVTab, (-12));
    } else if ((pTab)->eTabType == 1) {
    } else if (((pLoop->wsFlags & 0x00000040) == 0 && (wctrlFlags & 0x0020) == 0) ||
               (pTabItem->fg.jointype & (0x40 | 0x10)) != 0) {
      int op = 114;
      if (pWInfo->eOnePass != 0) {
        op = 116;
        pWInfo->aiCurOnePass[0] = pTabItem->iCursor;
      };
      sqlite3OpenTable(pParse, pTabItem->iCursor, iDb, pTab, op);

      if (pWInfo->eOnePass == 0 && pTab->nCol < ((int)(sizeof(Bitmask) * 8)) &&
          (pTab->tabFlags & (0x00000060 | 0x00000080)) == 0 && (pLoop->wsFlags & (0x00004000 | 0x00400000)) == 0) {
        Bitmask b = pTabItem->colUsed;
        int n = 0;
        for (; b; b = b >> 1, n++) {
        }
        sqlite3VdbeChangeP4(v, -1, (const char*)(((void *)(intptr_t)(n))), (-3));
      }

      {
        sqlite3VdbeChangeP5(v, bFordelete);
      }

      if (ii >= 2 && (pTabItem[0].fg.jointype & (0x40 | 0x08)) == 0 && pLevel->addrHalt == pWInfo->a[0].addrHalt) {
        sqlite3VdbeAddOp2(v, 37, pTabItem->iCursor, pWInfo->iBreak);
      }
    } else {
      sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);
    }
    if (pLoop->wsFlags & 0x00000200) {
      Index *pIx = pLoop->u.btree.pIndex;
      int iIndexCur;
      int op = 114;

      if (!(((pTab)->tabFlags & 0x00000080) == 0) && ((pIx)->idxType == 2) && (wctrlFlags & 0x0020) != 0) {
        iIndexCur = pLevel->iTabCur;
        op = 0;
      } else if (pWInfo->eOnePass != 0) {
        Index *pJ = pTabItem->pSTab->pIndex;
        iIndexCur = iAuxArg;

        while ((pJ) && pJ != pIx) {
          iIndexCur++;
          pJ = pJ->pNext;
        }
        op = 116;
        pWInfo->aiCurOnePass[1] = iIndexCur;
      } else if (iAuxArg && (wctrlFlags & 0x0020) != 0) {
        iIndexCur = iAuxArg;
        op = 113;
      } else {
        iIndexCur = pParse->nTab++;
        if (pIx->bHasExpr && (((db)->dbOptFlags & (0x01000000)) == 0)) {
          whereAddIndexedExpr(pParse, pIx, iIndexCur, pTabItem);
        }
        if (pIx->pPartIdxWhere && (pTabItem->fg.jointype & 0x10) == 0) {
          wherePartIdxExpr(pParse, pIx, pIx->pPartIdxWhere, 0, iIndexCur, pTabItem);
        }
      }
      pLevel->iIdxCur = iIndexCur;

      if (op) {
        sqlite3VdbeAddOp3(v, op, iIndexCur, pIx->tnum, iDb);
        sqlite3VdbeSetP4KeyInfo(pParse, pIx);
        if ((pLoop->wsFlags & 0x0000000f) != 0 && (pLoop->wsFlags & (0x00000002 | 0x00008000)) == 0 &&
            (pLoop->wsFlags & 0x00080000) == 0 && (pLoop->wsFlags & 0x00100000) == 0 &&
            (pWInfo->wctrlFlags & 0x0001) == 0 && pWInfo->eDistinct != 2) {
          sqlite3VdbeChangeP5(v, 0x02);
        };
      }
    }
    if (iDb >= 0)
      sqlite3CodeVerifySchema(pParse, iDb);
    if ((pTabItem->fg.jointype & 0x10) != 0 &&
        (pLevel->pRJ = (WhereRightJoin*)(sqlite3WhereMalloc(pWInfo, sizeof(WhereRightJoin)))) != 0) {
      WhereRightJoin *pRJ = pLevel->pRJ;
      pRJ->iMatch = pParse->nTab++;
      pRJ->regBloom = ++pParse->nMem;
      sqlite3VdbeAddOp2(v, 79, 65536, pRJ->regBloom);
      pRJ->regReturn = ++pParse->nMem;
      sqlite3VdbeAddOp2(v, 77, 0, pRJ->regReturn);

      if ((((pTab)->tabFlags & 0x00000080) == 0)) {
        KeyInfo *pInfo;
        sqlite3VdbeAddOp2(v, 120, pRJ->iMatch, 1);
        pInfo = sqlite3KeyInfoAlloc(pParse->db, 1, 0);
        if (pInfo) {
          pInfo->aColl[0] = 0;
          pInfo->aSortFlags[0] = 0;
          sqlite3VdbeAppendP4(v, pInfo, (-9));
        }
      } else {
        Index *pPk = sqlite3PrimaryKeyIndex(pTab);
        sqlite3VdbeAddOp2(v, 120, pRJ->iMatch, pPk->nKeyCol);
        sqlite3VdbeSetP4KeyInfo(pParse, pPk);
      }
      pLoop->wsFlags &= ~0x00000040;

      pWInfo->nOBSat = 0;
      pWInfo->eDistinct = 3;
    }
  }
  pWInfo->iTop = sqlite3VdbeCurrentAddr(v);
  if (db->mallocFailed)
    goto whereBeginError;

  for (ii = 0; ii < nTabList; ii++) {
    int addrExplain;
    int wsFlags;
    SrcItem *pSrc;
    if (pParse->nErr)
      goto whereBeginError;
    pLevel = &pWInfo->a[ii];
    wsFlags = pLevel->pWLoop->wsFlags;
    pSrc = &pTabList->a[pLevel->iFrom];
    if (pSrc->fg.isMaterialized) {
      Subquery *pSubq;
      int iOnce = 0;

      pSubq = pSrc->u4.pSubq;
      if (pSrc->fg.isCorrelated == 0) {
        iOnce = sqlite3VdbeAddOp0(v, 15);
      } else {
        iOnce = 0;
      }
      sqlite3VdbeAddOp2(v, 10, pSubq->regReturn, pSubq->addrFillSub);
      if (iOnce)
        sqlite3VdbeJumpHere(v, iOnce);
    }

    if ((wsFlags & (0x00004000 | 0x00400000)) != 0) {
      if ((wsFlags & 0x00004000) != 0) {
        constructAutomaticIndex(pParse, &pWInfo->sWC, notReady, pLevel);

      } else {
        sqlite3ConstructBloomFilter(pWInfo, ii, pLevel, notReady);
      }
      if (db->mallocFailed)
        goto whereBeginError;
    }
    addrExplain = sqlite3WhereExplainOneScan(pParse, pTabList, pLevel, wctrlFlags);
    pLevel->addrBody = sqlite3VdbeCurrentAddr(v);
    notReady = sqlite3WhereCodeOneLoopStart(pParse, v, pWInfo, ii, pLevel, notReady);
    pWInfo->iContinue = pLevel->addrCont;
    if ((wsFlags & 0x00002000) == 0 && (wctrlFlags & 0x0020) == 0) {
      ((void)addrExplain);
    }
  }

  pWInfo->iEndWhere = sqlite3VdbeCurrentAddr(v);
  return pWInfo;

whereBeginError:
  if (pWInfo) {
    pParse->nQueryLoop = pWInfo->savedNQueryLoop;
    whereInfoFree(db, pWInfo);
  }

  return 0;
}

Window *windowFind(Parse *pParse, Window *pList, const char *zName) {
  Window *p;
  for (p = pList; p; p = p->pNextWin) {
    if (sqlite3StrICmp(p->zName, zName) == 0)
      break;
  }
  if (p == 0) {
    sqlite3ErrorMsg(pParse, "no such window: %s", zName);
  }
  return p;
}

void sqlite3WindowUpdate(Parse *pParse, Window *pList, Window *pWin, FuncDef *pFunc) {
  if (pWin->zName && pWin->eFrmType == 0) {
    Window *p = windowFind(pParse, pList, pWin->zName);
    if (p == 0)
      return;
    pWin->pPartition = sqlite3ExprListDup(pParse->db, p->pPartition, 0);
    pWin->pOrderBy = sqlite3ExprListDup(pParse->db, p->pOrderBy, 0);
    pWin->pStart = sqlite3ExprDup(pParse->db, p->pStart, 0);
    pWin->pEnd = sqlite3ExprDup(pParse->db, p->pEnd, 0);
    pWin->eStart = p->eStart;
    pWin->eEnd = p->eEnd;
    pWin->eFrmType = p->eFrmType;
    pWin->eExclude = p->eExclude;
  } else {
    sqlite3WindowChain(pParse, pWin, pList);
  }
  if ((pWin->eFrmType == 90) && (pWin->pStart || pWin->pEnd) && (pWin->pOrderBy == 0 || pWin->pOrderBy->nExpr != 1)) {
    sqlite3ErrorMsg(pParse, "RANGE with offset PRECEDING/FOLLOWING requires one ORDER BY expression");
  } else if (pFunc->funcFlags & 0x00010000) {
    sqlite3 *db = pParse->db;
    if (pWin->pFilter) {
      sqlite3ErrorMsg(pParse, "FILTER clause may only be used with aggregate window functions");
    } else {
      struct WindowUpdate {
        const char *zFunc;
        int eFrmType;
        int eStart;
        int eEnd;
      } aUp[] = {
          {row_numberName, 77, 91, 86},   {dense_rankName, 90, 91, 86}, {rankName, 90, 91, 86},
          {percent_rankName, 93, 86, 91}, {cume_distName, 93, 87, 91},  {ntileName, 77, 86, 91},
          {leadName, 77, 91, 91},         {lagName, 77, 91, 86},
      };
      int i;
      for (i = 0; i < ((int)(sizeof(aUp) / sizeof(aUp[0]))); i++) {
        if (pFunc->zName == aUp[i].zFunc) {
          sqlite3ExprDelete(db, pWin->pStart);
          sqlite3ExprDelete(db, pWin->pEnd);
          pWin->pEnd = pWin->pStart = 0;
          pWin->eFrmType = aUp[i].eFrmType;
          pWin->eStart = aUp[i].eStart;
          pWin->eEnd = aUp[i].eEnd;
          pWin->eExclude = 0;
          if (pWin->eStart == 87) {
            pWin->pStart = sqlite3ExprInt32(db, 1);
          }
          break;
        }
      }
    }
  }
  pWin->pWFunc = pFunc;
}

void selectWindowRewriteEList(Parse *pParse, Window *pWin, SrcList *pSrc, ExprList *pEList, Table *pTab,
                              ExprList **ppSub) {
  Walker sWalker;
  WindowRewrite sRewrite;

  memset(&sWalker, 0, sizeof(Walker));
  memset(&sRewrite, 0, sizeof(WindowRewrite));

  sRewrite.pSub = *ppSub;
  sRewrite.pWin = pWin;
  sRewrite.pSrc = pSrc;
  sRewrite.pTab = pTab;

  sWalker.pParse = pParse;
  sWalker.xExprCallback = selectWindowRewriteExprCb;
  sWalker.xSelectCallback = selectWindowRewriteSelectCb;
  sWalker.u.pRewrite = &sRewrite;

  (void)sqlite3WalkExprList(&sWalker, pEList);

  *ppSub = sRewrite.pSub;
}

ExprList *exprListAppendList(Parse *pParse, ExprList *pList, ExprList *pAppend, int bIntToNull) {
  if (pAppend) {
    int i;
    int nInit = pList ? pList->nExpr : 0;
    for (i = 0; i < pAppend->nExpr; i++) {
      sqlite3 *db = pParse->db;
      Expr *pDup = sqlite3ExprDup(db, pAppend->a[i].pExpr, 0);
      if (db->mallocFailed) {
        sqlite3ExprDelete(db, pDup);
        break;
      }
      if (bIntToNull) {
        int iDummy;
        Expr *pSub;
        pSub = sqlite3ExprSkipCollateAndLikely(pDup);
        if (sqlite3ExprIsInteger(pSub, &iDummy, 0)) {
          pSub->op = 122;
          pSub->flags &= ~(0x000800 | 0x10000000 | 0x20000000);
          pSub->u.zToken = 0;
        }
      }
      pList = sqlite3ExprListAppend(pParse, pList, pDup);
      if (pList)
        pList->a[nInit + i].fg.sortFlags = pAppend->a[i].fg.sortFlags;
    }
  }
  return pList;
}

int sqlite3WindowRewrite(Parse *pParse, Select *p) {
  int rc = SQLITE_OK;
  if (p->pWin && p->pPrior == 0 && ((p->selFlags & 0x0100000) == 0) && (!(pParse->eParseMode >= 2))) {
    Vdbe *v = sqlite3GetVdbe(pParse);
    sqlite3 *db = pParse->db;
    Select *pSub = 0;
    SrcList *pSrc = p->pSrc;
    Expr *pWhere = p->pWhere;
    ExprList *pGroupBy = p->pGroupBy;
    Expr *pHaving = p->pHaving;
    ExprList *pSort = 0;

    ExprList *pSublist = 0;
    Window *pMWin = p->pWin;
    Window *pWin;
    Table *pTab;
    Walker w;

    u32 selFlags = p->selFlags;

    pTab = (Table*)(sqlite3DbMallocZero(db, sizeof(Table)));
    if (pTab == 0) {
      return sqlite3ErrorToParser(db, SQLITE_NOMEM);
    }
    sqlite3AggInfoPersistWalkerInit(&w, pParse);
    sqlite3WalkSelect(&w, p);
    if ((p->selFlags & 0x0000008) == 0) {
      w.xExprCallback = disallowAggregatesInOrderByCb;
      w.xSelectCallback = 0;
      sqlite3WalkExprList(&w, p->pOrderBy);
    }

    p->pSrc = 0;
    p->pWhere = 0;
    p->pGroupBy = 0;
    p->pHaving = 0;
    p->selFlags &= ~(u32)0x0000008;
    p->selFlags |= 0x0100000;

    pSort = exprListAppendList(pParse, 0, pMWin->pPartition, 1);
    pSort = exprListAppendList(pParse, pSort, pMWin->pOrderBy, 1);
    if (pSort && p->pOrderBy && p->pOrderBy->nExpr <= pSort->nExpr) {
      int nSave = pSort->nExpr;
      pSort->nExpr = p->pOrderBy->nExpr;
      if (sqlite3ExprListCompare(pSort, p->pOrderBy, -1) == 0) {
        sqlite3ExprListDelete(db, p->pOrderBy);
        p->pOrderBy = 0;
      }
      pSort->nExpr = nSave;
    }

    pMWin->iEphCsr = pParse->nTab++;
    pParse->nTab += 3;

    selectWindowRewriteEList(pParse, pMWin, pSrc, p->pEList, pTab, &pSublist);
    selectWindowRewriteEList(pParse, pMWin, pSrc, p->pOrderBy, pTab, &pSublist);
    pMWin->nBufferCol = (pSublist ? pSublist->nExpr : 0);

    pSublist = exprListAppendList(pParse, pSublist, pMWin->pPartition, 0);
    pSublist = exprListAppendList(pParse, pSublist, pMWin->pOrderBy, 0);

    for (pWin = pMWin; pWin; pWin = pWin->pNextWin) {
      ExprList *pArgs;

      pArgs = pWin->pOwner->x.pList;
      if (pWin->pWFunc->funcFlags & SQLITE_SUBTYPE) {
        selectWindowRewriteEList(pParse, pMWin, pSrc, pArgs, pTab, &pSublist);
        pWin->iArgCol = (pSublist ? pSublist->nExpr : 0);
        pWin->bExprArgs = 1;
      } else {
        pWin->iArgCol = (pSublist ? pSublist->nExpr : 0);
        pSublist = exprListAppendList(pParse, pSublist, pArgs, 0);
      }
      if (pWin->pFilter) {
        Expr *pFilter = sqlite3ExprDup(db, pWin->pFilter, 0);
        pSublist = sqlite3ExprListAppend(pParse, pSublist, pFilter);
      }
      pWin->regAccum = ++pParse->nMem;
      pWin->regResult = ++pParse->nMem;
      sqlite3VdbeAddOp2(v, 77, 0, pWin->regAccum);
    }

    if (pSublist == 0) {
      pSublist = sqlite3ExprListAppend(pParse, 0, sqlite3ExprInt32(db, 0));
    }

    pSub = sqlite3SelectNew(pParse, pSublist, pSrc, pWhere, pGroupBy, pHaving, pSort, 0, 0);

    p->pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);

    if (p->pSrc == 0) {
      sqlite3SelectDelete(db, pSub);
    } else if (sqlite3SrcItemAttachSubquery(pParse, &p->pSrc->a[0], pSub, 0)) {
      Table *pTab2;
      p->pSrc->a[0].fg.isCorrelated = 1;
      sqlite3SrcListAssignCursors(pParse, p->pSrc);
      pSub->selFlags |= 0x0000040 | 0x8000000;
      pTab2 = sqlite3ResultSetOfSelect(pParse, pSub, 0x40);
      pSub->selFlags |= (selFlags & 0x0000008);
      if (pTab2 == 0) {
        rc = SQLITE_NOMEM;
      } else {
        memcpy(pTab, pTab2, sizeof(Table));
        pTab->tabFlags |= 0x00004000;
        p->pSrc->a[0].pSTab = pTab;
        pTab = pTab2;
        memset(&w, 0, sizeof(w));
        w.xExprCallback = sqlite3WindowExtraAggFuncDepth;
        w.xSelectCallback = sqlite3WalkerDepthIncrease;
        w.xSelectCallback2 = sqlite3WalkerDepthDecrease;
        sqlite3WalkSelect(&w, pSub);
      }
    }
    if (db->mallocFailed)
      rc = SQLITE_NOMEM;

    sqlite3ParserAddCleanup(pParse, sqlite3DbFree, pTab);
  }

  return rc;
}

Expr *sqlite3WindowOffsetExpr(Parse *pParse, Expr *pExpr) {
  if (0 == sqlite3ExprIsConstant(0, pExpr)) {
    if ((pParse->eParseMode >= 2))
      sqlite3RenameExprUnmap(pParse, pExpr);
    sqlite3ExprDelete(pParse->db, pExpr);
    pExpr = sqlite3ExprAlloc(pParse->db, 122, 0, 0);
  }
  return pExpr;
}

Window *sqlite3WindowAlloc(Parse *pParse, int eType, int eStart, Expr *pStart, int eEnd, Expr *pEnd, u8 eExclude) {
  Window *pWin = 0;
  int bImplicitFrame = 0;

  if (eType == 0) {
    bImplicitFrame = 1;
    eType = 90;
  }

  if ((eStart == 86 && eEnd == 89) || (eStart == 87 && (eEnd == 89 || eEnd == 86))) {
    sqlite3ErrorMsg(pParse, "unsupported frame specification");
    goto windowAllocErr;
  }

  pWin = (Window *)sqlite3DbMallocZero(pParse->db, sizeof(Window));
  if (pWin == 0)
    goto windowAllocErr;
  pWin->eFrmType = eType;
  pWin->eStart = eStart;
  pWin->eEnd = eEnd;
  if (eExclude == 0 && (((pParse->db)->dbOptFlags & (0x00000002)) != 0)) {
    eExclude = 67;
  }
  pWin->eExclude = eExclude;
  pWin->bImplicitFrame = bImplicitFrame;
  pWin->pEnd = sqlite3WindowOffsetExpr(pParse, pEnd);
  pWin->pStart = sqlite3WindowOffsetExpr(pParse, pStart);
  return pWin;

windowAllocErr:
  sqlite3ExprDelete(pParse->db, pEnd);
  sqlite3ExprDelete(pParse->db, pStart);
  return 0;
}

Window *sqlite3WindowAssemble(Parse *pParse, Window *pWin, ExprList *pPartition, ExprList *pOrderBy, Token *pBase) {
  if (pWin) {
    pWin->pPartition = pPartition;
    pWin->pOrderBy = pOrderBy;
    if (pBase) {
      pWin->zBase = sqlite3DbStrNDup(pParse->db, pBase->z, pBase->n);
    }
  } else {
    sqlite3ExprListDelete(pParse->db, pPartition);
    sqlite3ExprListDelete(pParse->db, pOrderBy);
  }
  return pWin;
}

void sqlite3WindowChain(Parse *pParse, Window *pWin, Window *pList) {
  if (pWin->zBase) {
    sqlite3 *db = pParse->db;
    Window *pExist = windowFind(pParse, pList, pWin->zBase);
    if (pExist) {
      const char *zErr = 0;

      if (pWin->pPartition) {
        zErr = "PARTITION clause";
      } else if (pExist->pOrderBy && pWin->pOrderBy) {
        zErr = "ORDER BY clause";
      } else if (pExist->bImplicitFrame == 0) {
        zErr = "frame specification";
      }
      if (zErr) {
        sqlite3ErrorMsg(pParse, "cannot override %s of window: %s", zErr, pWin->zBase);
      } else {
        pWin->pPartition = sqlite3ExprListDup(db, pExist->pPartition, 0);
        if (pExist->pOrderBy) {
          pWin->pOrderBy = sqlite3ExprListDup(db, pExist->pOrderBy, 0);
        }
        sqlite3DbFree(db, pWin->zBase);
        pWin->zBase = 0;
      }
    }
  }
}

void sqlite3WindowAttach(Parse *pParse, Expr *p, Window *pWin) {
  if (p) {
    p->y.pWin = pWin;
    (p)->flags |= (u32)(0x1000000 | 0x020000);
    pWin->pOwner = p;
    if ((p->flags & 0x000004) && pWin->eFrmType != 167) {
      sqlite3ErrorMsg(pParse, "DISTINCT is not supported for window functions");
    }
  } else {
    sqlite3WindowDelete(pParse->db, pWin);
  }
}

int sqlite3WindowCompare(const Parse *pParse, const Window *p1, const Window *p2, int bFilter) {
  int res;
  if ((p1 == 0) || (p2 == 0))
    return 1;
  if (p1->eFrmType != p2->eFrmType)
    return 1;
  if (p1->eStart != p2->eStart)
    return 1;
  if (p1->eEnd != p2->eEnd)
    return 1;
  if (p1->eExclude != p2->eExclude)
    return 1;
  if (sqlite3ExprCompare(pParse, p1->pStart, p2->pStart, -1))
    return 1;
  if (sqlite3ExprCompare(pParse, p1->pEnd, p2->pEnd, -1))
    return 1;
  if ((res = sqlite3ExprListCompare(p1->pPartition, p2->pPartition, -1))) {
    return res;
  }
  if ((res = sqlite3ExprListCompare(p1->pOrderBy, p2->pOrderBy, -1))) {
    return res;
  }
  if (bFilter) {
    if ((res = sqlite3ExprCompare(pParse, p1->pFilter, p2->pFilter, -1))) {
      return res;
    }
  }
  return 0;
}

void sqlite3WindowCodeInit(Parse *pParse, Select *pSelect) {
  Window *pWin;
  int nEphExpr;
  Window *pMWin;
  Vdbe *v;

  nEphExpr = pSelect->pSrc->a[0].u4.pSubq->pSelect->pEList->nExpr;
  pMWin = pSelect->pWin;
  v = sqlite3GetVdbe(pParse);

  sqlite3VdbeAddOp2(v, 120, pMWin->iEphCsr, nEphExpr);
  sqlite3VdbeAddOp2(v, 117, pMWin->iEphCsr + 1, pMWin->iEphCsr);
  sqlite3VdbeAddOp2(v, 117, pMWin->iEphCsr + 2, pMWin->iEphCsr);
  sqlite3VdbeAddOp2(v, 117, pMWin->iEphCsr + 3, pMWin->iEphCsr);

  if (pMWin->pPartition) {
    int nExpr = pMWin->pPartition->nExpr;
    pMWin->regPart = pParse->nMem + 1;
    pParse->nMem += nExpr;
    sqlite3VdbeAddOp3(v, 77, 0, pMWin->regPart, pMWin->regPart + nExpr - 1);
  }

  pMWin->regOne = ++pParse->nMem;
  sqlite3VdbeAddOp2(v, 73, 1, pMWin->regOne);

  if (pMWin->eExclude) {
    pMWin->regStartRowid = ++pParse->nMem;
    pMWin->regEndRowid = ++pParse->nMem;
    pMWin->csrApp = pParse->nTab++;
    sqlite3VdbeAddOp2(v, 73, 1, pMWin->regStartRowid);
    sqlite3VdbeAddOp2(v, 73, 0, pMWin->regEndRowid);
    sqlite3VdbeAddOp2(v, 117, pMWin->csrApp, pMWin->iEphCsr);
    return;
  }

  for (pWin = pMWin; pWin; pWin = pWin->pNextWin) {
    FuncDef *p = pWin->pWFunc;
    if ((p->funcFlags & 0x1000) && pWin->eStart != 91) {
      ExprList *pList;
      KeyInfo *pKeyInfo;

      pList = pWin->pOwner->x.pList;
      pKeyInfo = sqlite3KeyInfoFromExprList(pParse, pList, 0, 0);
      pWin->csrApp = pParse->nTab++;
      pWin->regApp = pParse->nMem + 1;
      pParse->nMem += 3;
      if (pKeyInfo && pWin->pWFunc->zName[1] == 'i') {
        pKeyInfo->aSortFlags[0] = 0x01;
      }
      sqlite3VdbeAddOp2(v, 120, pWin->csrApp, 2);
      sqlite3VdbeAppendP4(v, pKeyInfo, (-9));
      sqlite3VdbeAddOp2(v, 73, 0, pWin->regApp + 1);
    } else if (p->zName == nth_valueName || p->zName == first_valueName) {
      pWin->regApp = pParse->nMem + 1;
      pWin->csrApp = pParse->nTab++;
      pParse->nMem += 2;
      sqlite3VdbeAddOp2(v, 117, pWin->csrApp, pMWin->iEphCsr);
    } else if (p->zName == leadName || p->zName == lagName) {
      pWin->csrApp = pParse->nTab++;
      sqlite3VdbeAddOp2(v, 117, pWin->csrApp, pMWin->iEphCsr);
    }
  }
}

void windowCheckValue(Parse *pParse, int reg, int eCond) {
  static const char *azErr[] = {
      "frame starting offset must be a non-negative integer",    "frame ending offset must be a non-negative integer",
      "second argument to nth_value must be a positive integer", "frame starting offset must be a non-negative number",
      "frame ending offset must be a non-negative number",
  };
  static int aOp[] = {58, 58, 55, 58, 58};
  Vdbe *v = sqlite3GetVdbe(pParse);
  int regZero = sqlite3GetTempReg(pParse);

  sqlite3VdbeAddOp2(v, 73, 0, regZero);
  if (eCond >= 3) {
    int regString = sqlite3GetTempReg(pParse);
    sqlite3VdbeAddOp4(v, 118, 0, regString, 0, "", (-1));
    sqlite3VdbeAddOp3(v, 58, regString, sqlite3VdbeCurrentAddr(v) + 2, reg);
    sqlite3VdbeChangeP5(v, 0x43 | 0x10);

  } else {
    sqlite3VdbeAddOp2(v, 13, reg, sqlite3VdbeCurrentAddr(v) + 2);
  }
  sqlite3VdbeAddOp3(v, aOp[eCond], regZero, sqlite3VdbeCurrentAddr(v) + 2, reg);
  sqlite3VdbeChangeP5(v, 0x43);
  sqlite3MayAbort(pParse);
  sqlite3VdbeAddOp2(v, 72, SQLITE_ERROR, 2);
  sqlite3VdbeAppendP4(v, (void *)azErr[eCond], (-1));
  sqlite3ReleaseTempReg(pParse, regZero);
}

int windowInitAccum(Parse *pParse, Window *pMWin) {
  Vdbe *v = sqlite3GetVdbe(pParse);
  int regArg;
  int nArg = 0;
  Window *pWin;
  for (pWin = pMWin; pWin; pWin = pWin->pNextWin) {
    FuncDef *pFunc = pWin->pWFunc;

    sqlite3VdbeAddOp2(v, 77, 0, pWin->regAccum);
    nArg = ((nArg) > (windowArgCount(pWin)) ? (nArg) : (windowArgCount(pWin)));
    if (pMWin->regStartRowid == 0) {
      if (pFunc->zName == nth_valueName || pFunc->zName == first_valueName) {
        sqlite3VdbeAddOp2(v, 73, 0, pWin->regApp);
        sqlite3VdbeAddOp2(v, 73, 0, pWin->regApp + 1);
      }

      if ((pFunc->funcFlags & 0x1000) && pWin->csrApp) {
        sqlite3VdbeAddOp1(v, 148, pWin->csrApp);
        sqlite3VdbeAddOp2(v, 73, 0, pWin->regApp + 1);
      }
    }
  }
  regArg = pParse->nMem + 1;
  pParse->nMem += nArg;
  return regArg;
}

void windowIfNewPeer(Parse *pParse, ExprList *pOrderBy, int regNew, int regOld, int addr) {
  Vdbe *v = sqlite3GetVdbe(pParse);
  if (pOrderBy) {
    int nVal = pOrderBy->nExpr;
    KeyInfo *pKeyInfo = sqlite3KeyInfoFromExprList(pParse, pOrderBy, 0, 0);
    sqlite3VdbeAddOp3(v, 92, regOld, regNew, nVal);
    sqlite3VdbeAppendP4(v, (void *)pKeyInfo, (-9));
    sqlite3VdbeAddOp3(v, 14, sqlite3VdbeCurrentAddr(v) + 1, addr, sqlite3VdbeCurrentAddr(v) + 1);
    sqlite3VdbeAddOp3(v, 82, regNew, regOld, nVal - 1);
  } else {
    sqlite3VdbeAddOp2(v, 9, 0, addr);
  }
}

int windowExprGtZero(Parse *pParse, Expr *pExpr) {
  int ret = 0;
  sqlite3 *db = pParse->db;
  sqlite3_value *pVal = 0;
  sqlite3ValueFromExpr(db, pExpr, db->enc, 0x43, &pVal);
  if (pVal && sqlite3_value_int(pVal) > 0) {
    ret = 1;
  }
  sqlite3ValueFree(pVal);
  return ret;
}

void sqlite3WindowCodeStep(Parse *pParse, Select *p, WhereInfo *pWInfo, int regGosub, int addrGosub) {
  Window *pMWin = p->pWin;
  ExprList *pOrderBy = pMWin->pOrderBy;
  Vdbe *v = sqlite3GetVdbe(pParse);
  int csrWrite;
  int csrInput = p->pSrc->a[0].iCursor;
  int nInput = p->pSrc->a[0].pSTab->nCol;
  int iInput;
  int addrNe;
  int addrGosubFlush = 0;
  int addrInteger = 0;
  int addrEmpty;
  int regNew;
  int regRecord;
  int regNewPeer = 0;
  int regPeer = 0;
  int regFlushPart = 0;
  WindowCodeArg s;
  int lblWhereEnd;
  int regStart = 0;
  int regEnd = 0;

  lblWhereEnd = sqlite3VdbeMakeLabel(pParse);

  memset(&s, 0, sizeof(WindowCodeArg));
  s.pParse = pParse;
  s.pMWin = pMWin;
  s.pVdbe = v;
  s.regGosub = regGosub;
  s.addrGosub = addrGosub;
  s.current.csr = pMWin->iEphCsr;
  csrWrite = s.current.csr + 1;
  s.start.csr = s.current.csr + 2;
  s.end.csr = s.current.csr + 3;

  switch (pMWin->eStart) {
    case 87:
      if (pMWin->eFrmType != 90 && windowExprGtZero(pParse, pMWin->pStart)) {
        s.eDelete = 1;
      }
      break;
    case 91:
      if (windowCacheFrame(pMWin) == 0) {
        if (pMWin->eEnd == 89) {
          if (pMWin->eFrmType != 90 && windowExprGtZero(pParse, pMWin->pEnd)) {
            s.eDelete = 3;
          }
        } else {
          s.eDelete = 1;
        }
      }
      break;
    default:
      s.eDelete = 2;
      break;
  }

  regNew = pParse->nMem + 1;
  pParse->nMem += nInput;
  regRecord = ++pParse->nMem;
  s.regRowid = ++pParse->nMem;

  if (pMWin->eStart == 89 || pMWin->eStart == 87) {
    regStart = ++pParse->nMem;
  }
  if (pMWin->eEnd == 89 || pMWin->eEnd == 87) {
    regEnd = ++pParse->nMem;
  }

  if (pMWin->eFrmType != 77) {
    int nPeer = (pOrderBy ? pOrderBy->nExpr : 0);
    regNewPeer = regNew + pMWin->nBufferCol;
    if (pMWin->pPartition)
      regNewPeer += pMWin->pPartition->nExpr;
    regPeer = pParse->nMem + 1;
    pParse->nMem += nPeer;
    s.start.reg = pParse->nMem + 1;
    pParse->nMem += nPeer;
    s.current.reg = pParse->nMem + 1;
    pParse->nMem += nPeer;
    s.end.reg = pParse->nMem + 1;
    pParse->nMem += nPeer;
  }

  for (iInput = 0; iInput < nInput; iInput++) {
    sqlite3VdbeAddOp3(v, 96, csrInput, iInput, regNew + iInput);
  }
  sqlite3VdbeAddOp3(v, 99, regNew, nInput, regRecord);

  if (pMWin->pPartition) {
    int addr;
    ExprList *pPart = pMWin->pPartition;
    int nPart = pPart->nExpr;
    int regNewPart = regNew + pMWin->nBufferCol;
    KeyInfo *pKeyInfo = sqlite3KeyInfoFromExprList(pParse, pPart, 0, 0);

    regFlushPart = ++pParse->nMem;
    addr = sqlite3VdbeAddOp3(v, 92, regNewPart, pMWin->regPart, nPart);
    sqlite3VdbeAppendP4(v, (void *)pKeyInfo, (-9));
    sqlite3VdbeAddOp3(v, 14, addr + 2, addr + 4, addr + 2);
    addrGosubFlush = sqlite3VdbeAddOp1(v, 10, regFlushPart);
    sqlite3VdbeAddOp3(v, 82, regNewPart, pMWin->regPart, nPart - 1);
  }

  sqlite3VdbeAddOp2(v, 129, csrWrite, s.regRowid);
  sqlite3VdbeAddOp3(v, 130, csrWrite, regRecord, s.regRowid);
  addrNe = sqlite3VdbeAddOp3(v, 53, pMWin->regOne, 0, s.regRowid);

  s.regArg = windowInitAccum(pParse, pMWin);

  if (regStart) {
    sqlite3ExprCode(pParse, pMWin->pStart, regStart);
    windowCheckValue(pParse, regStart, 0 + (pMWin->eFrmType == 90 ? 3 : 0));
  }
  if (regEnd) {
    sqlite3ExprCode(pParse, pMWin->pEnd, regEnd);
    windowCheckValue(pParse, regEnd, 1 + (pMWin->eFrmType == 90 ? 3 : 0));
  }

  if (pMWin->eFrmType != 90 && pMWin->eStart == pMWin->eEnd && regStart) {
    int op = ((pMWin->eStart == 87) ? 58 : 56);
    int addrGe = sqlite3VdbeAddOp3(v, op, regStart, 0, regEnd);
    windowAggFinal(&s, 0);
    sqlite3VdbeAddOp1(v, 36, s.current.csr);
    windowReturnOneRow(&s);
    sqlite3VdbeAddOp1(v, 148, s.current.csr);
    sqlite3VdbeAddOp2(v, 9, 0, lblWhereEnd);
    sqlite3VdbeJumpHere(v, addrGe);
  }
  if (pMWin->eStart == 87 && pMWin->eFrmType != 90 && regEnd) {
    sqlite3VdbeAddOp3(v, 108, regStart, regEnd, regStart);
  }

  if (pMWin->eStart != 91) {
    sqlite3VdbeAddOp1(v, 36, s.start.csr);
  }
  sqlite3VdbeAddOp1(v, 36, s.current.csr);
  sqlite3VdbeAddOp1(v, 36, s.end.csr);
  if (regPeer && pOrderBy) {
    sqlite3VdbeAddOp3(v, 82, regNewPeer, regPeer, pOrderBy->nExpr - 1);
    sqlite3VdbeAddOp3(v, 82, regPeer, s.start.reg, pOrderBy->nExpr - 1);
    sqlite3VdbeAddOp3(v, 82, regPeer, s.current.reg, pOrderBy->nExpr - 1);
    sqlite3VdbeAddOp3(v, 82, regPeer, s.end.reg, pOrderBy->nExpr - 1);
  }

  sqlite3VdbeAddOp2(v, 9, 0, lblWhereEnd);

  sqlite3VdbeJumpHere(v, addrNe);

  if (regPeer) {
    windowIfNewPeer(pParse, pOrderBy, regNewPeer, regPeer, lblWhereEnd);
  }
  if (pMWin->eStart == 87) {
    windowCodeOp(&s, 3, 0, 0);
    if (pMWin->eEnd != 91) {
      if (pMWin->eFrmType == 90) {
        int lbl = sqlite3VdbeMakeLabel(pParse);
        int addrNext = sqlite3VdbeCurrentAddr(v);
        windowCodeRangeTest(&s, 58, s.current.csr, regEnd, s.end.csr, lbl);
        windowCodeOp(&s, 2, regStart, 0);
        windowCodeOp(&s, 1, 0, 0);
        sqlite3VdbeAddOp2(v, 9, 0, addrNext);
        sqlite3VdbeResolveLabel(v, lbl);
      } else {
        windowCodeOp(&s, 1, regEnd, 0);
        windowCodeOp(&s, 2, regStart, 0);
      }
    }
  } else if (pMWin->eEnd == 89) {
    int bRPS = (pMWin->eStart == 89 && pMWin->eFrmType == 90);
    windowCodeOp(&s, 3, regEnd, 0);
    if (bRPS)
      windowCodeOp(&s, 2, regStart, 0);
    windowCodeOp(&s, 1, 0, 0);
    if (!bRPS)
      windowCodeOp(&s, 2, regStart, 0);
  } else {
    int addr = 0;
    windowCodeOp(&s, 3, 0, 0);
    if (pMWin->eEnd != 91) {
      if (pMWin->eFrmType == 90) {
        int lbl = 0;
        addr = sqlite3VdbeCurrentAddr(v);
        if (regEnd) {
          lbl = sqlite3VdbeMakeLabel(pParse);
          windowCodeRangeTest(&s, 58, s.current.csr, regEnd, s.end.csr, lbl);
        }
        windowCodeOp(&s, 1, 0, 0);
        windowCodeOp(&s, 2, regStart, 0);
        if (regEnd) {
          sqlite3VdbeAddOp2(v, 9, 0, addr);
          sqlite3VdbeResolveLabel(v, lbl);
        }
      } else {
        if (regEnd) {
          addr = sqlite3VdbeAddOp3(v, 61, regEnd, 0, 1);
        }
        windowCodeOp(&s, 1, 0, 0);
        windowCodeOp(&s, 2, regStart, 0);
        if (regEnd)
          sqlite3VdbeJumpHere(v, addr);
      }
    }
  }

  sqlite3VdbeResolveLabel(v, lblWhereEnd);
  sqlite3WhereEnd(pWInfo);

  if (pMWin->pPartition) {
    addrInteger = sqlite3VdbeAddOp2(v, 73, 0, regFlushPart);
    sqlite3VdbeJumpHere(v, addrGosubFlush);
  }

  s.regRowid = 0;
  addrEmpty = sqlite3VdbeAddOp1(v, 36, csrWrite);
  if (pMWin->eEnd == 89) {
    int bRPS = (pMWin->eStart == 89 && pMWin->eFrmType == 90);
    windowCodeOp(&s, 3, regEnd, 0);
    if (bRPS)
      windowCodeOp(&s, 2, regStart, 0);
    windowCodeOp(&s, 1, 0, 0);
  } else if (pMWin->eStart == 87) {
    int addrStart;
    int addrBreak1;
    int addrBreak2;
    int addrBreak3;
    windowCodeOp(&s, 3, 0, 0);
    if (pMWin->eFrmType == 90) {
      addrStart = sqlite3VdbeCurrentAddr(v);
      addrBreak2 = windowCodeOp(&s, 2, regStart, 1);
      addrBreak1 = windowCodeOp(&s, 1, 0, 1);
    } else if (pMWin->eEnd == 91) {
      addrStart = sqlite3VdbeCurrentAddr(v);
      addrBreak1 = windowCodeOp(&s, 1, regStart, 1);
      addrBreak2 = windowCodeOp(&s, 2, 0, 1);
    } else {
      sqlite3VdbeAddOp3(v, 108, regStart, regEnd, regEnd);
      sqlite3VdbeAddOp2(v, 73, 0, regStart);

      addrStart = sqlite3VdbeCurrentAddr(v);
      addrBreak1 = windowCodeOp(&s, 1, regEnd, 1);
      addrBreak2 = windowCodeOp(&s, 2, regStart, 1);
    }
    sqlite3VdbeAddOp2(v, 9, 0, addrStart);
    sqlite3VdbeJumpHere(v, addrBreak2);
    addrStart = sqlite3VdbeCurrentAddr(v);
    addrBreak3 = windowCodeOp(&s, 1, 0, 1);
    sqlite3VdbeAddOp2(v, 9, 0, addrStart);
    sqlite3VdbeJumpHere(v, addrBreak1);
    sqlite3VdbeJumpHere(v, addrBreak3);
  } else {
    int addrBreak;
    int addrStart;
    windowCodeOp(&s, 3, 0, 0);
    addrStart = sqlite3VdbeCurrentAddr(v);
    addrBreak = windowCodeOp(&s, 1, 0, 1);
    windowCodeOp(&s, 2, regStart, 0);
    sqlite3VdbeAddOp2(v, 9, 0, addrStart);
    sqlite3VdbeJumpHere(v, addrBreak);
  }
  sqlite3VdbeJumpHere(v, addrEmpty);

  sqlite3VdbeAddOp1(v, 148, s.current.csr);
  if (pMWin->pPartition) {
    if (pMWin->regStartRowid) {
      sqlite3VdbeAddOp2(v, 73, 1, pMWin->regStartRowid);
      sqlite3VdbeAddOp2(v, 73, 0, pMWin->regEndRowid);
    }
    sqlite3VdbeChangeP1(v, addrInteger, sqlite3VdbeCurrentAddr(v));
    sqlite3VdbeAddOp1(v, 69, regFlushPart);
  }
}

void parserSyntaxError(Parse *pParse, Token *p) {
  sqlite3ErrorMsg(pParse, "near \"%T\": syntax error", p);
}

void disableLookaside(Parse *pParse) {
  sqlite3 *db = pParse->db;
  pParse->disableLookaside++;

  memset(&pParse->u1.cr, 0, sizeof(pParse->u1.cr));
  db->lookaside.bDisable++;
  db->lookaside.sz = 0;
}

void parserDoubleLinkSelect(Parse *pParse, Select *p) {
  if (p->pPrior) {
    Select *pNext = 0, *pLoop = p;
    int mxSelect, cnt = 1;
    while (1) {
      pLoop->pNext = pNext;
      pLoop->selFlags |= 0x0000100;
      pNext = pLoop;
      pLoop = pLoop->pPrior;
      if (pLoop == 0)
        break;
      cnt++;
      if (pLoop->pOrderBy || pLoop->pLimit) {
        sqlite3ErrorMsg(pParse, "%s clause should come after %s not before",
                        pLoop->pOrderBy != 0 ? "ORDER BY" : "LIMIT", sqlite3SelectOpName(pNext->op));
        break;
      }
    }
    if ((p->selFlags & (0x0000400 | 0x0000200)) == 0 &&
        (mxSelect = pParse->db->aLimit[SQLITE_LIMIT_COMPOUND_SELECT]) > 0 && cnt > mxSelect) {
      sqlite3ErrorMsg(pParse, "too many terms in compound SELECT");
    }
  }
}

Select *attachWithToSelect(Parse *pParse, Select *pSelect, With *pWith) {
  if (pSelect) {
    pSelect->pWith = pWith;
    parserDoubleLinkSelect(pParse, pSelect);
  } else {
    sqlite3WithDelete(pParse->db, pWith);
  }
  return pSelect;
}

int parserStackSizeLimit(Parse *pParse) {
  return pParse->db->aLimit[SQLITE_LIMIT_PARSER_DEPTH];
}

Expr *tokenExpr(Parse *pParse, int op, Token t) {
  Expr *p = (Expr*)(sqlite3DbMallocRawNN(pParse->db, sizeof(Expr) + t.n + 1));
  if (p) {
    p->op = (u8)op;
    p->affExpr = 0;
    p->flags = 0x800000;

    p->pLeft = p->pRight = 0;
    p->pAggInfo = 0;
    memset(&p->x, 0, sizeof(p->x));
    memset(&p->y, 0, sizeof(p->y));
    p->op2 = 0;
    p->iTable = 0;
    p->iColumn = 0;
    p->u.zToken = (char *)&p[1];
    memcpy(p->u.zToken, t.z, t.n);
    p->u.zToken[t.n] = 0;
    p->w.iOfst = (int)(t.z - pParse->zTail);
    if ((sqlite3CtypeMap[(unsigned char)(p->u.zToken[0])] & 0x80)) {
      sqlite3DequoteExpr(p);
    }

    p->nHeight = 1;

    if ((pParse->eParseMode >= 2)) {
      return (Expr *)sqlite3RenameTokenMap(pParse, (void *)p, &t);
    }
  }
  return p;
}

Expr *sqlite3PExprIsNull(Parse *pParse, int op, Expr *pLeft) {
  Expr *p = pLeft;

  while (p->op == 173 || p->op == 174) {
    p = p->pLeft;
  }
  switch (p->op) {
    case 156:
    case 118:
    case 154:
    case 155:
      sqlite3ExprDeferredDelete(pParse, pLeft);
      return sqlite3ExprInt32(pParse->db, op == 52);
    default:
      break;
  }
  return sqlite3PExpr(pParse, op, pLeft, 0);
}

Expr *sqlite3PExprIs(Parse *pParse, int op, Expr *pLeft, Expr *pRight) {
  if (pRight && pRight->op == 122) {
    sqlite3ExprDeferredDelete(pParse, pRight);
    return sqlite3PExprIsNull(pParse, op == 45 ? 51 : 52, pLeft);
  }
  return sqlite3PExpr(pParse, op, pLeft, pRight);
}

ExprList *parserAddExprIdListTerm(Parse *pParse, ExprList *pPrior, Token *pIdToken, int hasCollate, int sortOrder) {
  ExprList *p = sqlite3ExprListAppend(pParse, pPrior, 0);
  if ((hasCollate || sortOrder != -1) && pParse->db->init.busy == 0) {
    sqlite3ErrorMsg(pParse, "syntax error after column name \"%.*s\"", pIdToken->n, pIdToken->z);
  }
  sqlite3ExprListSetName(pParse, p, pIdToken, 1);
  return p;
}

int sqlite3RunParser(Parse *pParse, const char *zSql) {
  int nErr = 0;
  void *pEngine;
  i64 n = 0;
  int tokenType;
  int lastTokenParsed = -1;
  sqlite3 *db = pParse->db;
  i64 mxSqlLen;
  Parse *pParentParse = 0;

  yyParser sEngine;

  mxSqlLen = db->aLimit[SQLITE_LIMIT_SQL_LENGTH];
  if (db->nVdbeActive == 0) {
    __atomic_store_n((&db->u1.isInterrupted), (0), 0);
  }
  pParse->rc = SQLITE_OK;
  pParse->zTail = zSql;

  pEngine = &sEngine;
  sqlite3ParserInit(pEngine, pParse);

  pParentParse = db->pParse;
  db->pParse = pParse;
  while (1) {
    n = sqlite3GetToken((u8 *)zSql, &tokenType);
    mxSqlLen -= n;
    if (mxSqlLen < 0) {
      pParse->rc = SQLITE_TOOBIG;
      pParse->nErr++;
      break;
    }

    if (tokenType >= 165) {
      if (__atomic_load_n((&db->u1.isInterrupted), 0)) {
        pParse->rc = SQLITE_INTERRUPT;
        pParse->nErr++;
        break;
      }
      if (tokenType == 184) {
        zSql += n;
        continue;
      }
      if (zSql[0] == 0) {
        if (lastTokenParsed == 1) {
          tokenType = 0;
        } else if (lastTokenParsed == 0) {
          break;
        } else {
          tokenType = 1;
        }
        n = 0;

      } else if (tokenType == 165) {
        tokenType = analyzeWindowKeyword((const u8 *)&zSql[6]);
      } else if (tokenType == 166) {
        tokenType = analyzeOverKeyword((const u8 *)&zSql[4], lastTokenParsed);
      } else if (tokenType == 167) {
        tokenType = analyzeFilterKeyword((const u8 *)&zSql[6], lastTokenParsed);

      } else if (tokenType == 185 && (db->init.busy || (db->flags & ((u64)(0x00040) << 32)) != 0)) {
        zSql += n;
        continue;
      } else if (tokenType != 183) {
        Token x;
        x.z = zSql;
        x.n = (u32)n;
        sqlite3ErrorMsg(pParse, "unrecognized token: \"%T\"", &x);
        break;
      }
    }
    pParse->sLastToken.z = zSql;
    pParse->sLastToken.n = (u32)n;
    sqlite3Parser(pEngine, tokenType, pParse->sLastToken);
    lastTokenParsed = tokenType;
    zSql += n;

    if (pParse->rc != SQLITE_OK)
      break;
  }

  sqlite3ParserFinalize(pEngine);

  if (db->mallocFailed) {
    pParse->rc = 7;
  }
  if (pParse->zErrMsg || (pParse->rc != SQLITE_OK && pParse->rc != SQLITE_DONE)) {
    if (pParse->zErrMsg == 0) {
      pParse->zErrMsg = sqlite3DbStrDup(db, sqlite3ErrStr(pParse->rc));
    }
    if ((pParse->prepFlags & SQLITE_PREPARE_DONT_LOG) == 0) {
      sqlite3_log(pParse->rc, "%s in \"%s\"", pParse->zErrMsg, pParse->zTail);
    }
    nErr++;
  }
  pParse->zTail = zSql;

  sqlite3_free(pParse->apVtabLock);

  if (pParse->pNewTable && !(pParse->eParseMode != 0)) {
    sqlite3DeleteTable(db, pParse->pNewTable);
  }
  if (pParse->pNewTrigger && !(pParse->eParseMode >= 2)) {
    sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  }
  if (pParse->pVList)
    sqlite3DbNNFreeNN(db, pParse->pVList);
  db->pParse = pParentParse;

  return nErr;
}

int sqlite3ParseUri(const char *zDefaultVfs, const char *zUri, unsigned int *pFlags, sqlite3_vfs **ppVfs, char **pzFile,
                    char **pzErrMsg) {
  int rc = SQLITE_OK;
  unsigned int flags = *pFlags;
  const char *zVfs = zDefaultVfs;
  char *zFile;
  char c;
  i64 nUri = strlen(zUri);

  if (((flags & SQLITE_OPEN_URI) || __atomic_load_n((&sqlite3Config.bOpenUri), 0)) && nUri >= 5 &&
      memcmp(zUri, "file:", 5) == 0) {
    char *zOpt;
    int eState;
    i64 iIn;
    i64 iOut = 0;
    u64 nByte = nUri + 8;

    flags |= SQLITE_OPEN_URI;

    for (iIn = 0; iIn < nUri; iIn++)
      nByte += (zUri[iIn] == '&');
    zFile = (char*)(sqlite3_malloc64(nByte));
    if (!zFile)
      return 7;

    memset(zFile, 0, 4);
    zFile += 4;

    iIn = 5;

    if (zUri[5] == '/' && zUri[6] == '/') {
      iIn = 7;
      while (zUri[iIn] && zUri[iIn] != '/')
        iIn++;
      if (iIn != 7 && (iIn != 16 || memcmp("localhost", &zUri[7], 9))) {
        *pzErrMsg = sqlite3_mprintf("invalid uri authority: %.*s", (int)(iIn - 7), &zUri[7]);
        rc = SQLITE_ERROR;
        goto parse_uri_out;
      }
    }

    eState = 0;
    while ((c = zUri[iIn]) != 0 && c != '#') {
      iIn++;
      if (c == '%' && (sqlite3CtypeMap[(unsigned char)(zUri[iIn])] & 0x08) &&
          (sqlite3CtypeMap[(unsigned char)(zUri[iIn + 1])] & 0x08)) {
        int octet = (sqlite3HexToInt(zUri[iIn++]) << 4);
        octet += sqlite3HexToInt(zUri[iIn++]);

        if (octet == 0) {
          while ((c = zUri[iIn]) != 0 && c != '#' && (eState != 0 || c != '?') &&
                 (eState != 1 || (c != '=' && c != '&')) && (eState != 2 || c != '&')) {
            iIn++;
          }
          continue;
        }
        c = octet;
      } else if (eState == 1 && (c == '&' || c == '=')) {
        if (zFile[iOut - 1] == 0) {
          while (zUri[iIn] && zUri[iIn] != '#' && zUri[iIn - 1] != '&')
            iIn++;
          continue;
        }
        if (c == '&') {
          zFile[iOut++] = '\0';
        } else {
          eState = 2;
        }
        c = 0;
      } else if ((eState == 0 && c == '?') || (eState == 2 && c == '&')) {
        c = 0;
        eState = 1;
      }
      zFile[iOut++] = c;
    }
    if (eState == 1)
      zFile[iOut++] = '\0';
    memset(zFile + iOut, 0, 4);

    zOpt = &zFile[strlen(zFile) + 1];
    while (zOpt[0]) {
      i64 nOpt = strlen(zOpt);
      char *zVal = &zOpt[nOpt + 1];
      i64 nVal = strlen(zVal);

      if (nOpt == 3 && memcmp("vfs", zOpt, 3) == 0) {
        zVfs = zVal;
      } else {
        struct OpenMode {
          const char *z;
          int mode;
        } *aMode = 0;
        char *zModeType = 0;
        int mask = 0;
        int limit = 0;

        if (nOpt == 5 && memcmp("cache", zOpt, 5) == 0) {
          static struct OpenMode aCacheMode[] = {
              {"shared", SQLITE_OPEN_SHAREDCACHE}, {"private", SQLITE_OPEN_PRIVATECACHE}, {0, 0}};

          mask = SQLITE_OPEN_SHAREDCACHE | SQLITE_OPEN_PRIVATECACHE;
          aMode = aCacheMode;
          limit = mask;
          zModeType = (char*)("cache");
        }
        if (nOpt == 4 && memcmp("mode", zOpt, 4) == 0) {
          static struct OpenMode aOpenMode[] = {{"ro", SQLITE_OPEN_READONLY},
                                                {"rw", SQLITE_OPEN_READWRITE},
                                                {"rwc", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE},
                                                {"memory", SQLITE_OPEN_MEMORY},
                                                {0, 0}};

          mask = SQLITE_OPEN_READONLY | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MEMORY;
          aMode = aOpenMode;
          limit = mask & flags;
          zModeType = (char*)("access");
        }

        if (aMode) {
          int i;
          int mode = 0;
          for (i = 0; aMode[i].z; i++) {
            const char *z = aMode[i].z;
            if (nVal == (i64)strlen(z) && 0 == memcmp(zVal, z, nVal)) {
              mode = aMode[i].mode;
              break;
            }
          }
          if (mode == 0) {
            *pzErrMsg = sqlite3_mprintf("no such %s mode: %s", zModeType, zVal);
            rc = SQLITE_ERROR;
            goto parse_uri_out;
          }
          if ((mode & ~SQLITE_OPEN_MEMORY) > limit) {
            *pzErrMsg = sqlite3_mprintf("%s mode not allowed: %s", zModeType, zVal);
            rc = SQLITE_PERM;
            goto parse_uri_out;
          }
          flags = (flags & ~mask) | mode;
        }
      }

      zOpt = &zVal[nVal + 1];
    }

  } else {
    zFile = (char*)(sqlite3_malloc64(nUri + 8));
    if (!zFile)
      return 7;
    memset(zFile, 0, 4);
    zFile += 4;
    if (nUri) {
      memcpy(zFile, zUri, nUri);
    }
    memset(zFile + nUri, 0, 4);
    flags &= ~SQLITE_OPEN_URI;
  }

  *ppVfs = sqlite3_vfs_find(zVfs);
  if (*ppVfs == 0) {
    *pzErrMsg = sqlite3_mprintf("no such vfs: %s", zVfs);
    rc = SQLITE_ERROR;
  }
parse_uri_out:
  if (rc != SQLITE_OK) {
    sqlite3_free_filename(zFile);
    zFile = 0;
  }
  *pFlags = flags;
  *pzFile = zFile;
  return rc;
}