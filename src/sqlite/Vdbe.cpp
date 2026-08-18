#define _GNU_SOURCE 1
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "sqlite/Vdbe.h"
#include "sqlite/AuxData.h"
#include "sqlite/Bool.h"
#include "sqlite/BtCursor.h"
#include "sqlite/Btree.h"
#include "sqlite/BtreePayload.h"
#include "sqlite/BusyHandler.h"
#include "sqlite/CollSeq.h"
#include "sqlite/Column.h"
#include "sqlite/Db.h"
#include "sqlite/Expr.h"
#include "sqlite/FuncDef.h"
#include "sqlite/IncrMerger.h"
#include "sqlite/Index.h"
#include "sqlite/InitData.h"
#include "sqlite/KeyInfo.h"
#include "sqlite/LogEst.h"
#include "sqlite/Mem.h"
#include "sqlite/MergeEngine.h"
#include "sqlite/Op.h"
#include "sqlite/Pager.h"
#include "sqlite/Parse.h"
#include "sqlite/Pgno.h"
#include "sqlite/PmaReader.h"
#include "sqlite/PragmaName.h"
#include "sqlite/RCStr.h"
#include "sqlite/ReusableSpace.h"
#include "sqlite/RowSet.h"
#include "sqlite/Savepoint.h"
#include "sqlite/Schema.h"
#include "sqlite/SortSubtask.h"
#include "sqlite/Sqlite3Config.h"
#include "sqlite/StrAccum.h"
#include "sqlite/SubProgram.h"
#include "sqlite/Table.h"
#include "sqlite/UnpackedRecord.h"
#include "sqlite/VList.h"
#include "sqlite/VTable.h"
#include "sqlite/ValueList.h"
#include "sqlite/VdbeCursor.h"
#include "sqlite/VdbeFrame.h"
#include "sqlite/VdbeOp.h"
#include "sqlite/VdbeOpList.h"
#include "sqlite/VdbeSorter.h"
#include "sqlite/VdbeTxtBlbCache.h"
#include "sqlite/WhereInfo.h"
#include "sqlite/WhereLevel.h"
#include "sqlite/WhereLoop.h"
#include "sqlite/WhereTerm.h"
#include "sqlite/bft.h"
#include "sqlite/i16.h"
#include "sqlite/i64.h"
#include "sqlite/i8.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_context.h"
#include "sqlite/sqlite3_destructor_type.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_module.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/sqlite3_stmt.h"
#include "sqlite/sqlite3_str.h"
#include "sqlite/sqlite3_uint64.h"
#include "sqlite/sqlite3_value.h"
#include "sqlite/sqlite3_vfs.h"
#include "sqlite/sqlite3_vtab.h"
#include "sqlite/sqlite3_vtab_cursor.h"
#include "sqlite/sqlite3_xauth.h"
#include "sqlite/sqlite_int64.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/yDbMask.h"
#include "sqlite/ynVar.h"
#include "sqlite/SqliteAuthorizerActionCode.h"
#include "sqlite/SqliteFunctionFlags.h"
#include "sqlite/SqliteLimitCategory.h"
#include "sqlite/SqliteOpenFlags.h"
#include "sqlite/SqliteResultCode.h"
#include "sqlite/SqliteStmtStatusParameter.h"
#include "sqlite/SqliteTextEncoding.h"
#include "sqlite/SqliteTraceEventCode.h"
/* Private helpers, formerly declared in _Uncategorized.h. */
static i64 findNextHostParameter(const char *zSql, i64 *pnToken);
static void serialGet(const unsigned char *buf, u32 serial_type, Mem *pMem);
static int serialGet7(const unsigned char *buf, Mem *pMem);

static const unsigned char *sqlite3aLTb = &sqlite3UpperToLower[256 - 53];

static const unsigned char *sqlite3aEQb = &sqlite3UpperToLower[256 + 6 - 53];

static const unsigned char *sqlite3aGTb = &sqlite3UpperToLower[256 + 12 - 53];

static void serialGet(const unsigned char *buf, u32 serial_type, Mem *pMem) {
  u64 x = (((u32)(buf)[0] << 24) | ((buf)[1] << 16) | ((buf)[2] << 8) | (buf)[3]);
  u32 y = (((u32)(buf + 4)[0] << 24) | ((buf + 4)[1] << 16) | ((buf + 4)[2] << 8) | (buf + 4)[3]);
  x = (x << 32) + y;
  if (serial_type == 6) {
    pMem->u.i = *(i64 *)&x;
    pMem->flags = 0x0004;
  } else {
    memcpy(&pMem->u.r, &x, sizeof(x));
    pMem->flags =
        (((x) & (((u64)0x7ff) << 52)) == (((u64)0x7ff) << 52) && ((x) & ((((u64)1) << 52) - 1)) != 0) ? 0x0001 : 0x0008;
  }
}

static int serialGet7(const unsigned char *buf, Mem *pMem) {
  u64 x = (((u32)(buf)[0] << 24) | ((buf)[1] << 16) | ((buf)[2] << 8) | (buf)[3]);
  u32 y = (((u32)(buf + 4)[0] << 24) | ((buf + 4)[1] << 16) | ((buf + 4)[2] << 8) | (buf + 4)[3]);
  x = (x << 32) + y;

  memcpy(&pMem->u.r, &x, sizeof(x));
  if ((((x) & (((u64)0x7ff) << 52)) == (((u64)0x7ff) << 52) && ((x) & ((((u64)1) << 52) - 1)) != 0)) {
    pMem->flags = 0x0001;
    return 1;
  }
  pMem->flags = 0x0008;
  return 0;
}

static i64 findNextHostParameter(const char *zSql, i64 *pnToken) {
  int tokenType;
  i64 nTotal = 0;
  i64 n;

  *pnToken = 0;
  while (zSql[0]) {
    n = sqlite3GetToken((u8 *)zSql, &tokenType);

    if (tokenType == 157) {
      *pnToken = n;
      break;
    }
    nTotal += n;
    zSql += n;
  }
  return nTotal;
}

const char *const pragCName[] = {
    "id",    "seq",     "table",      "from",     "to",     "on_update",  "on_delete", "match",   "cid", "name",
    "type",  "notnull", "dflt_value", "pk",       "hidden", "name",       "builtin",   "type",    "enc", "narg",
    "flags", "schema",  "name",       "type",     "ncol",   "wr",         "strict",    "seqno",   "cid", "name",
    "desc",  "coll",    "key",        "seq",      "name",   "unique",     "origin",    "partial", "tbl", "idx",
    "wdth",  "hght",    "flgs",       "table",    "rowid",  "parent",     "fkid",      "busy",    "log", "checkpointed",
    "seq",   "name",    "file",       "database", "status", "cache_size", "timeout",
};

void vdbeMemRenderNum(int sz, char *zBuf, Mem *p) {
  StrAccum acc;

  if (p->flags & (0x0004 | 0x0020)) {
    p->n = sqlite3Int64ToText(p->u.i, zBuf);

    if (p->flags & 0x0020) {
      memcpy(zBuf + p->n, ".0", 3);
      p->n += 2;
    }
  } else {
    sqlite3StrAccumInit(&acc, 0, zBuf, sz, 0);
    sqlite3_str_appendf(&acc, "%!.*g", (p->db ? p->db->nFpDigit : 17), p->u.r);

    zBuf[acc.nChar] = 0;
    p->n = acc.nChar;
  }
}

Parse *sqlite3VdbeParser(Vdbe *p) {
  return p->pParse;
}

void sqlite3VdbeError(Vdbe *p, const char *zFormat, ...) {
  va_list ap;
  sqlite3DbFree(p->db, p->zErrMsg);

  va_start(ap, zFormat);
  p->zErrMsg = sqlite3VMPrintf(p->db, zFormat, ap);

  va_end(ap);
}

void sqlite3VdbeSetSql(Vdbe *p, const char *z, int n, u8 prepFlags) {
  if (p == 0)
    return;
  p->prepFlags = prepFlags;
  if ((prepFlags & 0x80) == 0) {
    p->expmask = 0;
  }

  p->zSql = sqlite3DbStrNDup(p->db, z, n);
}

void sqlite3VdbeSwap(Vdbe *pA, Vdbe *pB) {
  Vdbe tmp, *pTmp, **ppTmp;
  char *zTmp;

  tmp = *pA;
  *pA = *pB;
  *pB = tmp;
  pTmp = pA->pVNext;
  pA->pVNext = pB->pVNext;
  pB->pVNext = pTmp;
  ppTmp = pA->ppVPrev;
  pA->ppVPrev = pB->ppVPrev;
  pB->ppVPrev = ppTmp;
  zTmp = pA->zSql;
  pA->zSql = pB->zSql;
  pB->zSql = zTmp;

  pB->expmask = pA->expmask;
  pB->prepFlags = pA->prepFlags;
  memcpy(pB->aCounter, pA->aCounter, sizeof(pB->aCounter));
  pB->aCounter[SQLITE_STMTSTATUS_REPREPARE]++;
}

int growOpArray(Vdbe *v, int nOp) {
  VdbeOp *pNew;
  Parse *p = v->pParse;

  sqlite3_int64 nNew = (v->nOpAlloc ? 2 * (sqlite3_int64)v->nOpAlloc : (sqlite3_int64)(1024 / sizeof(Op)));
  (void)(nOp);

  if (nNew > p->db->aLimit[SQLITE_LIMIT_VDBE_OP]) {
    sqlite3OomFault(p->db);
    return SQLITE_NOMEM;
  }

  pNew = sqlite3DbRealloc(p->db, v->aOp, nNew * sizeof(Op));
  if (pNew) {
    p->szOpAlloc = sqlite3DbMallocSize(p->db, pNew);
    v->nOpAlloc = p->szOpAlloc / sizeof(Op);
    v->aOp = pNew;
  }
  return (pNew ? SQLITE_OK : 7);
}

__attribute__((noinline)) int growOp3(Vdbe *p, int op, int p1, int p2, int p3) {
  if (growOpArray(p, 1))
    return 1;

  return sqlite3VdbeAddOp3(p, op, p1, p2, p3);
}

__attribute__((noinline)) int addOp4IntSlow(Vdbe *p, int op, int p1, int p2, int p3, int p4) {
  int addr = sqlite3VdbeAddOp3(p, op, p1, p2, p3);
  if (p->db->mallocFailed == 0) {
    VdbeOp *pOp = &p->aOp[addr];
    pOp->p4type = (-3);
    pOp->p4.i = p4;
  }
  return addr;
}

int sqlite3VdbeAddOp0(Vdbe *p, int op) {
  return sqlite3VdbeAddOp3(p, op, 0, 0, 0);
}

int sqlite3VdbeAddOp1(Vdbe *p, int op, int p1) {
  return sqlite3VdbeAddOp3(p, op, p1, 0, 0);
}

int sqlite3VdbeAddOp2(Vdbe *p, int op, int p1, int p2) {
  return sqlite3VdbeAddOp3(p, op, p1, p2, 0);
}

int sqlite3VdbeAddOp3(Vdbe *p, int op, int p1, int p2, int p3) {
  int i;
  VdbeOp *pOp;

  i = p->nOp;

  if (p->nOpAlloc <= i) {
    return growOp3(p, op, p1, p2, p3);
  }

  p->nOp++;
  pOp = &p->aOp[i];

  pOp->opcode = (u8)op;
  pOp->p5 = 0;
  pOp->p1 = p1;
  pOp->p2 = p2;
  pOp->p3 = p3;
  pOp->p4.p = 0;
  pOp->p4type = 0;

  return i;
}

int sqlite3VdbeAddOp4Int(Vdbe *p, int op, int p1, int p2, int p3, int p4) {
  int i;
  VdbeOp *pOp;

  i = p->nOp;
  if (p->nOpAlloc <= i) {
    return addOp4IntSlow(p, op, p1, p2, p3, p4);
  }
  p->nOp++;
  pOp = &p->aOp[i];

  pOp->opcode = (u8)op;
  pOp->p5 = 0;
  pOp->p1 = p1;
  pOp->p2 = p2;
  pOp->p3 = p3;
  pOp->p4.i = p4;
  pOp->p4type = (-3);

  return i;
}

int sqlite3VdbeGoto(Vdbe *p, int iDest) {
  return sqlite3VdbeAddOp3(p, 9, 0, iDest, 0);
}

int sqlite3VdbeLoadString(Vdbe *p, int iDest, const char *zStr) {
  return sqlite3VdbeAddOp4(p, 118, 0, iDest, 0, zStr, 0);
}

void sqlite3VdbeMultiLoad(Vdbe *p, int iDest, const char *zTypes, ...) {
  va_list ap;
  int i;
  char c;

  va_start(ap, zTypes);
  for (i = 0; (c = zTypes[i]) != 0; i++) {
    if (c == 's') {
      const char *z = va_arg(ap, const char *);
      sqlite3VdbeAddOp4(p, z == 0 ? 77 : 118, 0, iDest + i, 0, z, 0);
    } else if (c == 'i') {
      sqlite3VdbeAddOp2(p, 73, va_arg(ap, int), iDest + i);
    } else {
      goto skip_op_resultrow;
    }
  }
  sqlite3VdbeAddOp2(p, 86, iDest, i);
skip_op_resultrow:
  va_end(ap);
}

int sqlite3VdbeAddOp4(Vdbe *p, int op, int p1, int p2, int p3, const char *zP4, int p4type) {
  int addr = sqlite3VdbeAddOp3(p, op, p1, p2, p3);
  sqlite3VdbeChangeP4(p, addr, zP4, p4type);
  return addr;
}

int sqlite3VdbeAddOp4Dup8(Vdbe *p, int op, int p1, int p2, int p3, const u8 *zP4, int p4type) {
  char *p4copy = sqlite3DbMallocRawNN(sqlite3VdbeDb(p), 8);
  if (p4copy)
    memcpy(p4copy, zP4, 8);
  return sqlite3VdbeAddOp4(p, op, p1, p2, p3, p4copy, p4type);
}

void sqlite3VdbeAddParseSchemaOp(Vdbe *p, int iDb, char *zWhere, u16 p5) {
  int j;
  sqlite3VdbeAddOp4(p, 151, iDb, 0, 0, zWhere, (-7));
  sqlite3VdbeChangeP5(p, p5);
  for (j = 0; j < p->db->nDb; j++)
    sqlite3VdbeUsesBtree(p, j);
  sqlite3MayAbort(p->pParse);
}

void sqlite3VdbeEndCoroutine(Vdbe *v, int regYield) {
  sqlite3VdbeAddOp1(v, 70, regYield);

  v->pParse->nTempReg = 0;
  v->pParse->nRangeReg = 0;
}

void sqlite3VdbeResolveLabel(Vdbe *v, int x) {
  Parse *p = v->pParse;
  int j = (~(x));

  if (p->nLabelAlloc + p->nLabel < 0) {
    resizeResolveLabel(p, v, j);
  } else {
    p->aLabel[j] = v->nOp;
  }
}

void sqlite3VdbeRunOnlyOnce(Vdbe *p) {
  sqlite3VdbeAddOp2(p, 168, 1, 1);
}

void sqlite3VdbeReusable(Vdbe *p) {
  int i;
  for (i = 1; (i < p->nOp); i++) {
    if (p->aOp[i].opcode == 168) {
      p->aOp[1].opcode = 189;
      break;
    }
  }
}

void resolveP2Values(Vdbe *p, int *pMaxVtabArgs) {
  int nMaxVtabArgs = *pMaxVtabArgs;
  Op *pOp;
  Parse *pParse = p->pParse;
  int *aLabel = pParse->aLabel;

  p->readOnly = 1;
  p->bIsReader = 0;
  pOp = &p->aOp[p->nOp - 1];

  while (1) {
    if (pOp->opcode <= 66) {
      switch (pOp->opcode) {
        case 2: {
          if (pOp->p2 != 0)
            p->readOnly = 0;
          __attribute__((fallthrough));
        }
        case 1:
        case 0: {
          p->bIsReader = 1;
          break;
        }

        case 3:
        case 5:
        case 4: {
          p->readOnly = 0;
          p->bIsReader = 1;
          break;
        }
        case 8: {
          goto resolve_p2_values_loop_exit;
        }

        case 7: {
          if (pOp->p2 > nMaxVtabArgs)
            nMaxVtabArgs = pOp->p2;
          break;
        }
        case 6: {
          int n;

          n = pOp[-1].p1;
          if (n > nMaxVtabArgs)
            nMaxVtabArgs = n;

          __attribute__((fallthrough));
        }

        default: {
          if (pOp->p2 < 0) {
            pOp->p2 = aLabel[(~(pOp->p2))];
          }

          break;
        }
      }
    }

    pOp--;
  }
resolve_p2_values_loop_exit:
  if (aLabel) {
    sqlite3DbNNFreeNN(p->db, pParse->aLabel);
    pParse->aLabel = 0;
  }
  pParse->nLabel = 0;
  *pMaxVtabArgs = nMaxVtabArgs;
}

int sqlite3VdbeCurrentAddr(Vdbe *p) {
  return p->nOp;
}

VdbeOp *sqlite3VdbeTakeOpArray(Vdbe *p, int *pnOp, int *pnMaxArg) {
  VdbeOp *aOp = p->aOp;

  resolveP2Values(p, pnMaxArg);
  *pnOp = p->nOp;
  p->aOp = 0;
  return aOp;
}

VdbeOp *sqlite3VdbeAddOpList(Vdbe *p, int nOp, VdbeOpList const *aOp, int iLineno) {
  int i;
  VdbeOp *pOut, *pFirst;

  if (p->nOp + nOp > p->nOpAlloc && growOpArray(p, nOp)) {
    return 0;
  }
  pFirst = pOut = &p->aOp[p->nOp];
  for (i = 0; i < nOp; i++, aOp++, pOut++) {
    pOut->opcode = aOp->opcode;
    pOut->p1 = aOp->p1;
    pOut->p2 = aOp->p2;

    if ((sqlite3OpcodeProperty[aOp->opcode] & 0x01) != 0 && aOp->p2 > 0) {
      pOut->p2 += p->nOp;
    }
    pOut->p3 = aOp->p3;
    pOut->p4type = 0;
    pOut->p4.p = 0;
    pOut->p5 = 0;

    (void)iLineno;
  }
  p->nOp += nOp;
  return pFirst;
}

void sqlite3VdbeChangeOpcode(Vdbe *p, int addr, u8 iNewOpcode) {
  sqlite3VdbeGetOp(p, addr)->opcode = iNewOpcode;
}

void sqlite3VdbeChangeP1(Vdbe *p, int addr, int val) {
  sqlite3VdbeGetOp(p, addr)->p1 = val;
}

void sqlite3VdbeChangeP2(Vdbe *p, int addr, int val) {
  sqlite3VdbeGetOp(p, addr)->p2 = val;
}

void sqlite3VdbeChangeP3(Vdbe *p, int addr, int val) {
  sqlite3VdbeGetOp(p, addr)->p3 = val;
}

void sqlite3VdbeChangeP5(Vdbe *p, u16 p5) {
  if (p->nOp > 0)
    p->aOp[p->nOp - 1].p5 = p5;
}

void sqlite3VdbeTypeofColumn(Vdbe *p, int iDest) {
  VdbeOp *pOp = sqlite3VdbeGetLastOp(p);

  if (pOp->p3 == iDest && pOp->opcode == 96) {
    pOp->p5 |= 0x80;
  }
}

void sqlite3VdbeJumpHere(Vdbe *p, int addr) {
  sqlite3VdbeChangeP2(p, addr, p->nOp);
}

void sqlite3VdbeJumpHereOrPopInst(Vdbe *p, int addr) {
  if (addr == p->nOp - 1) {
    p->nOp--;
  } else {
    sqlite3VdbeChangeP2(p, addr, p->nOp);
  }
}

void sqlite3VdbeLinkSubProgram(Vdbe *pVdbe, SubProgram *p) {
  p->pNext = pVdbe->pProgram;
  pVdbe->pProgram = p;
}

int sqlite3VdbeHasSubProgram(Vdbe *pVdbe) {
  return pVdbe->pProgram != 0;
}

int sqlite3VdbeChangeToNoop(Vdbe *p, int addr) {
  VdbeOp *pOp;
  if (p->db->mallocFailed)
    return 0;

  pOp = &p->aOp[addr];
  freeP4(p->db, pOp->p4type, pOp->p4.p);
  pOp->p4type = 0;
  pOp->p4.z = 0;
  pOp->opcode = 189;
  return 1;
}

int sqlite3VdbeDeletePriorOpcode(Vdbe *p, u8 op) {
  if (p->nOp > 0 && p->aOp[p->nOp - 1].opcode == op) {
    return sqlite3VdbeChangeToNoop(p, p->nOp - 1);
  } else {
    return 0;
  }
}

void __attribute__((noinline)) vdbeChangeP4Full(Vdbe *p, Op *pOp, const char *zP4, int n) {
  if (pOp->p4type) {
    pOp->p4type = 0;
    pOp->p4.p = 0;
  }
  if (n < 0) {
    sqlite3VdbeChangeP4(p, (int)(pOp - p->aOp), zP4, n);
  } else {
    if (n == 0)
      n = sqlite3Strlen30(zP4);
    pOp->p4.z = sqlite3DbStrNDup(p->db, zP4, n);
    pOp->p4type = (-7);
  }
}

void sqlite3VdbeChangeP4(Vdbe *p, int addr, const char *zP4, int n) {
  Op *pOp;
  sqlite3 *db;

  db = p->db;

  if (db->mallocFailed) {
    if (n != (-12))
      freeP4(db, n, (void *)*(char **)&zP4);
    return;
  }

  if (addr < 0) {
    addr = p->nOp - 1;
  }
  pOp = &p->aOp[addr];
  if (n >= 0 || pOp->p4type) {
    vdbeChangeP4Full(p, pOp, zP4, n);
    return;
  }
  if (n == (-3)) {
    pOp->p4.i = ((int)(intptr_t)(zP4));
    pOp->p4type = (-3);
  } else if (zP4 != 0) {
    pOp->p4.p = (void *)zP4;
    pOp->p4type = (signed char)n;
    if (n == (-12))
      sqlite3VtabLock((VTable *)zP4);
  }
}

void sqlite3VdbeAppendP4(Vdbe *p, void *pP4, int n) {
  VdbeOp *pOp;

  if (p->db->mallocFailed) {
    freeP4(p->db, n, pP4);
  } else {
    pOp = &p->aOp[p->nOp - 1];

    pOp->p4type = n;
    pOp->p4.p = pP4;
  }
}

VdbeOp *sqlite3VdbeGetOp(Vdbe *p, int addr) {
  static VdbeOp dummy;

  if (p->db->mallocFailed) {
    return (VdbeOp *)&dummy;
  } else {
    return &p->aOp[addr];
  }
}

VdbeOp *sqlite3VdbeGetLastOp(Vdbe *p) {
  return sqlite3VdbeGetOp(p, p->nOp - 1);
}

void sqlite3VdbeUsesBtree(Vdbe *p, int i) {
  ((p->btreeMask) |= (((yDbMask)1) << (i)));
  if (i != 1 && sqlite3BtreeSharable(p->db->aDb[i].pBt)) {
    ((p->lockMask) |= (((yDbMask)1) << (i)));
  }
}

void sqlite3VdbeEnter(Vdbe *p) {
  int i;
  sqlite3 *db;
  Db *aDb;
  int nDb;
  if ((p->lockMask) == 0)
    return;
  db = p->db;
  aDb = db->aDb;
  nDb = db->nDb;
  for (i = 0; i < nDb; i++) {
    if (i != 1 && (((p->lockMask) & (((yDbMask)1) << (i))) != 0) && (aDb[i].pBt != 0)) {
      sqlite3BtreeEnter(aDb[i].pBt);
    }
  }
}

__attribute__((noinline)) void vdbeLeave(Vdbe *p) {
  int i;
  sqlite3 *db;
  Db *aDb;
  int nDb;
  db = p->db;
  aDb = db->aDb;
  nDb = db->nDb;
  for (i = 0; i < nDb; i++) {
    if (i != 1 && (((p->lockMask) & (((yDbMask)1) << (i))) != 0) && (aDb[i].pBt != 0)) {
      sqlite3BtreeLeave(aDb[i].pBt);
    }
  }
}

void sqlite3VdbeLeave(Vdbe *p) {
  if ((p->lockMask) == 0)
    return;
  vdbeLeave(p);
}

int sqlite3VdbeNextOpcode(Vdbe *p, Mem *pSub, int eMode, int *piPc, int *piAddr, Op **paOp) {
  int nRow;
  int nSub = 0;
  SubProgram **apSub = 0;
  int i;
  int rc = SQLITE_OK;
  Op *aOp = 0;
  int iPc;

  nRow = p->nOp;
  if (pSub != 0) {
    if (pSub->flags & 0x0010) {
      nSub = pSub->n / sizeof(Vdbe *);
      apSub = (SubProgram **)pSub->z;
    }
    for (i = 0; i < nSub; i++) {
      nRow += apSub[i]->nOp;
    }
  }
  iPc = *piPc;
  while (1) {
    i = iPc++;
    if (i >= nRow) {
      p->rc = SQLITE_OK;
      rc = SQLITE_DONE;
      break;
    }
    if (i < p->nOp) {
      aOp = p->aOp;
    } else {
      int j;
      i -= p->nOp;

      for (j = 0; i >= apSub[j]->nOp; j++) {
        i -= apSub[j]->nOp;
      }
      aOp = apSub[j]->aOp;
    }

    if (pSub != 0 && aOp[i].p4type == (-4)) {
      int nByte = (nSub + 1) * sizeof(SubProgram *);
      int j;
      for (j = 0; j < nSub; j++) {
        if (apSub[j] == aOp[i].p4.pProgram)
          break;
      }
      if (j == nSub) {
        p->rc = sqlite3VdbeMemGrow(pSub, nByte, nSub != 0);
        if (p->rc != SQLITE_OK) {
          rc = SQLITE_ERROR;
          break;
        }
        apSub = (SubProgram **)pSub->z;
        apSub[nSub++] = aOp[i].p4.pProgram;
        ((pSub)->flags = ((pSub)->flags & ~(0x0dbf | 0x0400)) | 0x0010);
        pSub->n = nSub * sizeof(SubProgram *);
        nRow += aOp[i].p4.pProgram->nOp;
      }
    }
    if (eMode == 0)
      break;

    {
      if (aOp[i].opcode == 190)
        break;
      if (aOp[i].opcode == 8 && iPc > 1)
        break;
    }
  }
  *piPc = iPc;
  *piAddr = i;
  *paOp = aOp;
  return rc;
}

int sqlite3VdbeList(Vdbe *p) {
  Mem *pSub = 0;
  sqlite3 *db = p->db;
  int i;
  int rc = SQLITE_OK;
  Mem *pMem = &p->aMem[1];
  int bListSubprogs = (p->explain == 1 || (db->flags & 0x01000000) != 0);
  Op *aOp;
  Op *pOp;

  releaseMemArray(pMem, 8);

  if (p->rc == SQLITE_NOMEM) {
    sqlite3OomFault(db);
    return SQLITE_ERROR;
  }

  if (bListSubprogs) {
    pSub = &p->aMem[9];
  } else {
    pSub = 0;
  }

  rc = sqlite3VdbeNextOpcode(p, pSub, p->explain == 2, &p->pc, &i, &aOp);

  if (rc == SQLITE_OK) {
    pOp = aOp + i;
    if (__atomic_load_n((&db->u1.isInterrupted), 0)) {
      p->rc = SQLITE_INTERRUPT;
      rc = SQLITE_ERROR;
      sqlite3VdbeError(p, sqlite3ErrStr(p->rc));
    } else {
      char *zP4 = sqlite3VdbeDisplayP4(db, pOp);
      if (p->explain == 2) {
        sqlite3VdbeMemSetInt64(pMem, pOp->p1);
        sqlite3VdbeMemSetInt64(pMem + 1, pOp->p2);
        sqlite3VdbeMemSetInt64(pMem + 2, pOp->p3);
        sqlite3VdbeMemSetStr(pMem + 3, zP4, -1, SQLITE_UTF8, sqlite3_free);

      } else {
        sqlite3VdbeMemSetInt64(pMem + 0, i);
        sqlite3VdbeMemSetStr(pMem + 1, (char *)sqlite3OpcodeName(pOp->opcode), -1, SQLITE_UTF8,
                             ((sqlite3_destructor_type)0));
        sqlite3VdbeMemSetInt64(pMem + 2, pOp->p1);
        sqlite3VdbeMemSetInt64(pMem + 3, pOp->p2);
        sqlite3VdbeMemSetInt64(pMem + 4, pOp->p3);

        sqlite3VdbeMemSetInt64(pMem + 6, pOp->p5);

        sqlite3VdbeMemSetNull(pMem + 7);

        sqlite3VdbeMemSetStr(pMem + 5, zP4, -1, SQLITE_UTF8, sqlite3_free);
      }
      p->pResultRow = pMem;
      if (db->mallocFailed) {
        p->rc = SQLITE_NOMEM;
        rc = SQLITE_ERROR;
      } else {
        p->rc = SQLITE_OK;
        rc = SQLITE_ROW;
      }
    }
  }
  return rc;
}

void sqlite3VdbeRewind(Vdbe *p) {
  p->eVdbeState = 1;

  p->pc = -1;
  p->rc = SQLITE_OK;
  p->errorAction = 2;
  p->nChange = 0;
  p->cacheCtr = 1;
  p->minWriteFileFormat = 255;
  p->iStatement = 0;
  p->nFkConstraint = 0;
}

void sqlite3VdbeMakeReady(Vdbe *p, Parse *pParse) {
  sqlite3 *db;
  int nVar;
  int nMem;
  int nCursor;
  int nArg;
  int n;
  struct ReusableSpace x;

  p->pVList = pParse->pVList;
  pParse->pVList = 0;
  db = p->db;

  nVar = pParse->nVar;
  nMem = pParse->nMem;
  nCursor = pParse->nTab;
  nArg = pParse->nMaxArg;

  nMem += nCursor;
  if (nCursor == 0 && nMem > 0)
    nMem++;

  n = (sizeof(Op) * p->nOp);
  x.pSpace = &((u8 *)p->aOp)[n];

  x.nFree = ((pParse->szOpAlloc - n) & ~7);

  resolveP2Values(p, &nArg);
  p->usesStmtJournal = (u8)(pParse->isMultiWrite && pParse->mayAbort);
  if (pParse->explain) {
    if (nMem < 10)
      nMem = 10;
    p->explain = pParse->explain;
    p->nResColumn = 12 - 4 * p->explain;
  }
  p->expired = 0;

  x.nNeeded = 0;
  p->aMem = allocSpace(&x, 0, nMem * sizeof(Mem));
  p->aVar = allocSpace(&x, 0, nVar * sizeof(Mem));
  p->apArg = allocSpace(&x, 0, nArg * sizeof(Mem *));
  p->apCsr = allocSpace(&x, 0, nCursor * sizeof(VdbeCursor *));
  if (x.nNeeded) {
    x.pSpace = p->pFree = sqlite3DbMallocRawNN(db, x.nNeeded);
    x.nFree = x.nNeeded;
    if (!db->mallocFailed) {
      p->aMem = allocSpace(&x, p->aMem, nMem * sizeof(Mem));
      p->aVar = allocSpace(&x, p->aVar, nVar * sizeof(Mem));
      p->apArg = allocSpace(&x, p->apArg, nArg * sizeof(Mem *));
      p->apCsr = allocSpace(&x, p->apCsr, nCursor * sizeof(VdbeCursor *));
    }
  }

  if (db->mallocFailed) {
    p->nVar = 0;
    p->nCursor = 0;
    p->nMem = 0;
  } else {
    p->nCursor = nCursor;
    p->nVar = (ynVar)nVar;
    initMemArray(p->aVar, nVar, db, 0x0001);
    p->nMem = nMem;
    initMemArray(p->aMem, nMem, db, 0x0000);
    memset(p->apCsr, 0, nCursor * sizeof(VdbeCursor *));
  }
  sqlite3VdbeRewind(p);
}

void sqlite3VdbeFreeCursor(Vdbe *p, VdbeCursor *pCx) {
  if (pCx)
    sqlite3VdbeFreeCursorNN(p, pCx);
}

__attribute__((noinline)) void freeCursorWithCache(Vdbe *p, VdbeCursor *pCx) {
  VdbeTxtBlbCache *pCache = pCx->pCache;

  pCx->colCache = 0;
  pCx->pCache = 0;
  if (pCache->pCValue) {
    sqlite3RCStrUnref(pCache->pCValue);
    pCache->pCValue = 0;
  }
  sqlite3DbFree(p->db, pCache);
  sqlite3VdbeFreeCursorNN(p, pCx);
}

void sqlite3VdbeFreeCursorNN(Vdbe *p, VdbeCursor *pCx) {
  if (pCx->colCache) {
    freeCursorWithCache(p, pCx);
    return;
  }
  switch (pCx->eCurType) {
    case 1: {
      sqlite3VdbeSorterClose(p->db, pCx);
      break;
    }
    case 0: {
      sqlite3BtreeCloseCursor(pCx->uc.pCursor);
      break;
    }

    case 2: {
      sqlite3_vtab_cursor *pVCur = pCx->uc.pVCur;
      const sqlite3_module *pModule = pVCur->pVtab->pModule;

      pVCur->pVtab->nRef--;
      pModule->xClose(pVCur);
      break;
    }
  }
}

void closeCursorsInFrame(Vdbe *p) {
  int i;
  for (i = 0; i < p->nCursor; i++) {
    VdbeCursor *pC = p->apCsr[i];
    if (pC) {
      sqlite3VdbeFreeCursorNN(p, pC);
      p->apCsr[i] = 0;
    }
  }
}

void closeAllCursors(Vdbe *p) {
  if (p->pFrame) {
    VdbeFrame *pFrame;
    for (pFrame = p->pFrame; pFrame->pParent; pFrame = pFrame->pParent)
      ;
    sqlite3VdbeFrameRestore(pFrame);
    p->pFrame = 0;
    p->nFrame = 0;
  }

  closeCursorsInFrame(p);
  releaseMemArray(p->aMem, p->nMem);
  while (p->pDelFrame) {
    VdbeFrame *pDel = p->pDelFrame;
    p->pDelFrame = pDel->pParent;
    sqlite3VdbeFrameDelete(pDel);
  }

  if (p->pAuxData)
    sqlite3VdbeDeleteAuxData(p->db, &p->pAuxData, -1, 0);
}

void sqlite3VdbeSetNumCols(Vdbe *p, int nResColumn) {
  int n;
  sqlite3 *db = p->db;

  if (p->nResAlloc) {
    releaseMemArray(p->aColName, p->nResAlloc * 2);
    sqlite3DbFree(db, p->aColName);
  }
  n = nResColumn * 2;
  p->nResColumn = p->nResAlloc = (u16)nResColumn;
  p->aColName = (Mem *)sqlite3DbMallocRawNN(db, sizeof(Mem) * n);
  if (p->aColName == 0)
    return;
  initMemArray(p->aColName, n, db, 0x0001);
}

int sqlite3VdbeSetColName(Vdbe *p, int idx, int var, const char *zName, void (*xDel)(void *)) {
  int rc;
  Mem *pColName;

  if (p->db->mallocFailed) {
    return 7;
  }

  pColName = &(p->aColName[idx + var * p->nResAlloc]);
  rc = sqlite3VdbeMemSetText(pColName, zName, -1, xDel);

  return rc;
}

__attribute__((noinline)) int vdbeCloseStatement(Vdbe *p, int eOp) {
  sqlite3 *const db = p->db;
  int rc = SQLITE_OK;
  int i;
  const int iSavepoint = p->iStatement - 1;

  for (i = 0; i < db->nDb; i++) {
    int rc2 = SQLITE_OK;
    Btree *pBt = db->aDb[i].pBt;
    if (pBt) {
      if (eOp == 2) {
        rc2 = sqlite3BtreeSavepoint(pBt, 2, iSavepoint);
      }
      if (rc2 == SQLITE_OK) {
        rc2 = sqlite3BtreeSavepoint(pBt, 1, iSavepoint);
      }
      if (rc == SQLITE_OK) {
        rc = rc2;
      }
    }
  }
  db->nStatement--;
  p->iStatement = 0;

  if (rc == SQLITE_OK) {
    if (eOp == 2) {
      rc = sqlite3VtabSavepoint(db, 2, iSavepoint);
    }
    if (rc == SQLITE_OK) {
      rc = sqlite3VtabSavepoint(db, 1, iSavepoint);
    }
  }

  if (eOp == 2) {
    db->nDeferredCons = p->nStmtDefCons;
    db->nDeferredImmCons = p->nStmtDefImmCons;
  }
  return rc;
}

int sqlite3VdbeCloseStatement(Vdbe *p, int eOp) {
  if (p->db->nStatement && p->iStatement) {
    return vdbeCloseStatement(p, eOp);
  }
  return SQLITE_OK;
}

__attribute__((noinline)) int vdbeFkError(Vdbe *p) {
  p->rc = (19 | (3 << 8));
  p->errorAction = 2;
  sqlite3VdbeError(p, "FOREIGN KEY constraint failed");
  if ((p->prepFlags & 0x80) == 0)
    return SQLITE_ERROR;
  return (19 | (3 << 8));
}

int sqlite3VdbeCheckFkImmediate(Vdbe *p) {
  if (p->nFkConstraint == 0)
    return SQLITE_OK;
  return vdbeFkError(p);
}

int sqlite3VdbeCheckFkDeferred(Vdbe *p) {
  sqlite3 *db = p->db;
  if ((db->nDeferredCons + db->nDeferredImmCons) == 0)
    return SQLITE_OK;
  return vdbeFkError(p);
}

int sqlite3VdbeHalt(Vdbe *p) {
  int rc;
  sqlite3 *db = p->db;

  if (db->mallocFailed) {
    p->rc = 7;
  }
  closeAllCursors(p);

  if (p->bIsReader) {
    int mrc;
    int eStatementOp = 0;
    int isSpecialError;

    sqlite3VdbeEnter(p);

    if (p->rc) {
      mrc = p->rc & 0xff;
      isSpecialError = mrc == SQLITE_NOMEM || mrc == SQLITE_IOERR || mrc == SQLITE_INTERRUPT || mrc == SQLITE_FULL;
    } else {
      mrc = isSpecialError = 0;
    }
    if (isSpecialError) {
      if (!p->readOnly || mrc != SQLITE_INTERRUPT) {
        if ((mrc == SQLITE_NOMEM || mrc == SQLITE_FULL) && p->usesStmtJournal) {
          eStatementOp = 2;
        } else {
          sqlite3RollbackAll(db, (4 | (2 << 8)));
          sqlite3CloseSavepoints(db);
          db->autoCommit = 1;
          p->nChange = 0;
        }
      }
    }

    if (p->rc == SQLITE_OK || (p->errorAction == 3 && !isSpecialError)) {
      (void)sqlite3VdbeCheckFkImmediate(p);
    }

    if (!((db)->nVTrans > 0 && (db)->aVTrans == 0) && db->autoCommit && db->nVdbeWrite == (p->readOnly == 0)) {
      if (p->rc == SQLITE_OK || (p->errorAction == 3 && !isSpecialError)) {
        rc = sqlite3VdbeCheckFkDeferred(p);
        if (rc != SQLITE_OK) {
          if ((p->readOnly)) {
            sqlite3VdbeLeave(p);
            return SQLITE_ERROR;
          }
          rc = (19 | (3 << 8));
        } else if (db->flags & ((u64)(0x00002) << 32)) {
          rc = SQLITE_CORRUPT;
          db->flags &= ~((u64)(0x00002) << 32);
        } else {
          rc = vdbeCommit(db, p);
        }
        if (rc == SQLITE_BUSY && p->readOnly) {
          sqlite3VdbeLeave(p);
          return SQLITE_BUSY;
        } else if (rc != SQLITE_OK) {
          sqlite3SystemError(db, rc);
          p->rc = rc;
          sqlite3RollbackAll(db, SQLITE_OK);
          p->nChange = 0;
        } else {
          db->nDeferredCons = 0;
          db->nDeferredImmCons = 0;
          db->flags &= ~(u64)0x00080000;
          sqlite3CommitInternalChanges(db);
        }
      } else if (p->rc == SQLITE_SCHEMA && db->nVdbeActive > 1) {
        p->nChange = 0;
      } else {
        sqlite3RollbackAll(db, SQLITE_OK);
        p->nChange = 0;
      }
      db->nStatement = 0;
    } else if (eStatementOp == 0) {
      if (p->rc == SQLITE_OK || p->errorAction == 3) {
        eStatementOp = 1;
      } else if (p->errorAction == 2) {
        eStatementOp = 2;
      } else {
        sqlite3RollbackAll(db, (4 | (2 << 8)));
        sqlite3CloseSavepoints(db);
        db->autoCommit = 1;
        p->nChange = 0;
      }
    }

    if (eStatementOp) {
      rc = sqlite3VdbeCloseStatement(p, eStatementOp);
      if (rc) {
        if (p->rc == SQLITE_OK || (p->rc & 0xff) == SQLITE_CONSTRAINT) {
          p->rc = rc;
          sqlite3DbFree(db, p->zErrMsg);
          p->zErrMsg = 0;
        }
        sqlite3RollbackAll(db, (4 | (2 << 8)));
        sqlite3CloseSavepoints(db);
        db->autoCommit = 1;
        p->nChange = 0;
      }
    }

    if (p->changeCntOn) {
      if (eStatementOp != 2) {
        sqlite3VdbeSetChanges(db, p->nChange);
      } else {
        sqlite3VdbeSetChanges(db, 0);
      }
      p->nChange = 0;
    }

    sqlite3VdbeLeave(p);
  }

  db->nVdbeActive--;
  if (!p->readOnly)
    db->nVdbeWrite--;
  if (p->bIsReader)
    db->nVdbeRead--;

  p->eVdbeState = 3;
  if (db->mallocFailed) {
    p->rc = 7;
  }

  if (db->autoCommit) {
  }

  return (p->rc == SQLITE_BUSY ? SQLITE_BUSY : SQLITE_OK);
}

void sqlite3VdbeResetStepResult(Vdbe *p) {
  p->rc = SQLITE_OK;
}

int sqlite3VdbeTransferError(Vdbe *p) {
  sqlite3 *db = p->db;
  int rc = p->rc;
  if (p->zErrMsg) {
    db->bBenignMalloc++;
    sqlite3BeginBenignMalloc();
    if (db->pErr == 0)
      db->pErr = sqlite3ValueNew(db);
    sqlite3ValueSetStr(db->pErr, -1, p->zErrMsg, SQLITE_UTF8, ((sqlite3_destructor_type)-1));
    sqlite3EndBenignMalloc();
    db->bBenignMalloc--;
  } else if (db->pErr) {
    sqlite3ValueSetNull(db->pErr);
  }
  db->errCode = rc;
  db->errByteOffset = -1;
  return rc;
}

int sqlite3VdbeReset(Vdbe *p) {
  sqlite3 *db;
  db = p->db;

  if (p->eVdbeState == 2)
    sqlite3VdbeHalt(p);

  if (p->pc >= 0) {
    if (db->pErr || p->zErrMsg) {
      sqlite3VdbeTransferError(p);
    } else {
      db->errCode = p->rc;
    }
  }

  if (p->zErrMsg) {
    sqlite3DbFree(db, p->zErrMsg);
    p->zErrMsg = 0;
  }
  p->pResultRow = 0;

  return p->rc & db->errMask;
}

int sqlite3VdbeFinalize(Vdbe *p) {
  int rc = 0;

  if (p->eVdbeState >= 1) {
    rc = sqlite3VdbeReset(p);
  }
  sqlite3VdbeDelete(p);
  return rc;
}

void sqlite3VdbeDelete(Vdbe *p) {
  sqlite3 *db;

  db = p->db;

  sqlite3VdbeClearObject(db, p);
  if (db->pnBytesFreed == 0) {
    *p->ppVPrev = p->pVNext;
    if (p->pVNext) {
      p->pVNext->ppVPrev = p->ppVPrev;
    }
  }
  sqlite3DbNNFreeNN(db, p);
}

u32 sqlite3VdbeSerialTypeLen(u32 serial_type) {
  if (serial_type >= 128) {
    return (serial_type - 12) / 2;
  } else {
    return sqlite3SmallTypeSizes[serial_type];
  }
}

static u8 sqlite3VdbeOneByteSerialTypeLen(u8 serial_type) {
  return sqlite3SmallTypeSizes[serial_type];
}

void sqlite3VdbeSerialGet(const unsigned char *buf, u32 serial_type, Mem *pMem) {
  switch (serial_type) {
    case 10: {
      pMem->flags = 0x0001 | 0x0400;
      pMem->n = 0;
      pMem->u.nZero = 0;
      return;
    }
    case 11:
    case 0: {
      pMem->flags = 0x0001;
      return;
    }
    case 1: {
      pMem->u.i = ((i8)(buf)[0]);
      pMem->flags = 0x0004;
      return;
    }
    case 2: {
      pMem->u.i = (256 * (i8)((buf)[0]) | (buf)[1]);
      pMem->flags = 0x0004;
      return;
    }
    case 3: {
      pMem->u.i = (65536 * (i8)((buf)[0]) | ((buf)[1] << 8) | (buf)[2]);
      pMem->flags = 0x0004;
      return;
    }
    case 4: {
      pMem->u.i = (16777216 * (i8)((buf)[0]) | ((buf)[1] << 16) | ((buf)[2] << 8) | (buf)[3]);

      pMem->flags = 0x0004;
      return;
    }
    case 5: {
      pMem->u.i = (((u32)(buf + 2)[0] << 24) | ((buf + 2)[1] << 16) | ((buf + 2)[2] << 8) | (buf + 2)[3]) +
                  (((i64)1) << 32) * (256 * (i8)((buf)[0]) | (buf)[1]);
      pMem->flags = 0x0004;
      return;
    }
    case 6:
    case 7: {
      serialGet(buf, serial_type, pMem);
      return;
    }
    case 8:
    case 9: {
      pMem->u.i = serial_type - 8;
      pMem->flags = 0x0004;
      return;
    }
    default: {
      static const u16 aFlag[] = {0x0010 | 0x4000, 0x0002 | 0x4000};
      pMem->z = (char *)buf;
      pMem->n = (serial_type - 12) / 2;
      pMem->flags = aFlag[serial_type & 1];
      return;
    }
  }
  return;
}

void sqlite3VdbeRecordUnpack(int nKey, const void *pKey, UnpackedRecord *p) {
  const unsigned char *aKey = (const unsigned char *)pKey;
  u32 d;
  u32 idx;
  u16 u;
  u32 szHdr;
  Mem *pMem = p->aMem;
  KeyInfo *pKeyInfo = p->pKeyInfo;

  p->default_rc = 0;

  idx = (u8)((*(aKey) < (u8)0x80) ? ((szHdr) = (u32) * (aKey)), 1 : sqlite3GetVarint32((aKey), (u32 *)&(szHdr)));
  d = szHdr;
  u = 0;
  while (idx < szHdr && d <= (u32)nKey) {
    u32 serial_type;

    idx += (u8)((*(&aKey[idx]) < (u8)0x80) ? ((serial_type) = (u32) * (&aKey[idx])),
                1                          : sqlite3GetVarint32((&aKey[idx]), (u32 *)&(serial_type)));
    pMem->enc = pKeyInfo->enc;
    pMem->db = pKeyInfo->db;

    pMem->szMalloc = 0;
    pMem->z = 0;
    sqlite3VdbeSerialGet(&aKey[d], serial_type, pMem);
    d += sqlite3VdbeSerialTypeLen(serial_type);
    if ((++u) >= p->nField)
      break;
    pMem++;
  }
  if (d > (u32)nKey && u) {
    sqlite3VdbeMemSetNull(pMem - (u < p->nField));
  };

  p->nField = u;
}

static i64 vdbeRecordDecodeInt(u32 serial_type, const u8 *aKey) {
  u32 y;

  switch (serial_type) {
    case 0:
    case 1:;
      return ((i8)(aKey)[0]);
    case 2:;
      return (256 * (i8)((aKey)[0]) | (aKey)[1]);
    case 3:;
      return (65536 * (i8)((aKey)[0]) | ((aKey)[1] << 8) | (aKey)[2]);
    case 4: {
      y = (((u32)(aKey)[0] << 24) | ((aKey)[1] << 16) | ((aKey)[2] << 8) | (aKey)[3]);
      return (i64) * (int *)&y;
    }
    case 5: {
      return (((u32)(aKey + 2)[0] << 24) | ((aKey + 2)[1] << 16) | ((aKey + 2)[2] << 8) | (aKey + 2)[3]) +
             (((i64)1) << 32) * (256 * (i8)((aKey)[0]) | (aKey)[1]);
    }
    case 6: {
      u64 x = (((u32)(aKey)[0] << 24) | ((aKey)[1] << 16) | ((aKey)[2] << 8) | (aKey)[3]);
      x = (x << 32) | (((u32)(aKey + 4)[0] << 24) | ((aKey + 4)[1] << 16) | ((aKey + 4)[2] << 8) | (aKey + 4)[3]);
      return (i64) * (i64 *)&x;
    }
  }

  return (serial_type - 8);
}

int sqlite3VdbeRecordCompareWithSkip(int nKey1, const void *pKey1, UnpackedRecord *pPKey2, int bSkip) {
  u32 d1;
  int i;
  u32 szHdr1;
  u32 idx1;
  int rc = 0;
  Mem *pRhs = pPKey2->aMem;
  KeyInfo *pKeyInfo;
  const unsigned char *aKey1 = (const unsigned char *)pKey1;
  Mem mem1;

  if (bSkip) {
    u32 s1 = aKey1[1];
    if (s1 < 0x80) {
      idx1 = 2;
    } else {
      idx1 = 1 + sqlite3GetVarint32(&aKey1[1], &s1);
    }
    szHdr1 = aKey1[0];
    d1 = szHdr1 + sqlite3VdbeSerialTypeLen(s1);
    i = 1;
    pRhs++;
  } else {
    if ((szHdr1 = aKey1[0]) < 0x80) {
      idx1 = 1;
    } else {
      idx1 = sqlite3GetVarint32(aKey1, &szHdr1);
    }
    d1 = szHdr1;
    i = 0;
  }
  if (d1 > (unsigned)nKey1) {
    pPKey2->errCode = (u8)sqlite3CorruptError(92646);
    return 0;
  }

  while (1) {
    u32 serial_type;

    if (pRhs->flags & (0x0004 | 0x0020)) {
      serial_type = aKey1[idx1];
      if (serial_type >= 10) {
        rc = serial_type == 10 ? -1 : +1;
      } else if (serial_type == 0) {
        rc = -1;
      } else if (serial_type == 7) {
        serialGet7(&aKey1[d1], &mem1);
        rc = -sqlite3IntFloatCompare(pRhs->u.i, mem1.u.r);
      } else {
        i64 lhs = vdbeRecordDecodeInt(serial_type, &aKey1[d1]);
        i64 rhs = pRhs->u.i;
        if (lhs < rhs) {
          rc = -1;
        } else if (lhs > rhs) {
          rc = +1;
        }
      }
    }

    else if (pRhs->flags & 0x0008) {
      serial_type = aKey1[idx1];
      if (serial_type >= 10) {
        rc = serial_type == 10 ? -1 : +1;
      } else if (serial_type == 0) {
        rc = -1;
      } else {
        if (serial_type == 7) {
          if (serialGet7(&aKey1[d1], &mem1)) {
            rc = -1;
          } else if (mem1.u.r < pRhs->u.r) {
            rc = -1;
          } else if (mem1.u.r > pRhs->u.r) {
            rc = +1;
          } else {
          }
        } else {
          sqlite3VdbeSerialGet(&aKey1[d1], serial_type, &mem1);
          rc = sqlite3IntFloatCompare(mem1.u.i, pRhs->u.r);
        }
      }
    }

    else if (pRhs->flags & 0x0002) {
      serial_type = (u32) * (&aKey1[idx1]);
      if (serial_type >= 0x80)
        sqlite3GetVarint32((&aKey1[idx1]), (u32 *)&(serial_type));
      if (serial_type < 12) {
        rc = -1;
      } else if (!(serial_type & 0x01)) {
        rc = +1;
      } else {
        mem1.n = (serial_type - 12) / 2;
        if ((d1 + mem1.n) > (unsigned)nKey1 || (pKeyInfo = pPKey2->pKeyInfo)->nAllField <= i) {
          pPKey2->errCode = (u8)sqlite3CorruptError(92727);
          return 0;
        } else if (pKeyInfo->aColl[i]) {
          mem1.enc = pKeyInfo->enc;
          mem1.db = pKeyInfo->db;
          mem1.flags = 0x0002;
          mem1.z = (char *)&aKey1[d1];
          rc = vdbeCompareMemString(&mem1, pRhs, pKeyInfo->aColl[i], &pPKey2->errCode);
        } else {
          int nCmp = ((mem1.n) < (pRhs->n) ? (mem1.n) : (pRhs->n));
          rc = memcmp(&aKey1[d1], pRhs->z, nCmp);
          if (rc == 0)
            rc = mem1.n - pRhs->n;
        }
      }
    }

    else if (pRhs->flags & 0x0010) {
      serial_type = (u32) * (&aKey1[idx1]);
      if (serial_type >= 0x80)
        sqlite3GetVarint32((&aKey1[idx1]), (u32 *)&(serial_type));
      if (serial_type < 12 || (serial_type & 0x01)) {
        rc = -1;
      } else {
        int nStr = (serial_type - 12) / 2;
        if ((d1 + nStr) > (unsigned)nKey1) {
          pPKey2->errCode = (u8)sqlite3CorruptError(92757);
          return 0;
        } else if (pRhs->flags & 0x0400) {
          if (!isAllZero((const char *)&aKey1[d1], nStr)) {
            rc = 1;
          } else {
            rc = nStr - pRhs->u.nZero;
          }
        } else {
          int nCmp = ((nStr) < (pRhs->n) ? (nStr) : (pRhs->n));
          rc = memcmp(&aKey1[d1], pRhs->z, nCmp);
          if (rc == 0)
            rc = nStr - pRhs->n;
        }
      }
    }

    else {
      serial_type = aKey1[idx1];
      if (serial_type == 0 || serial_type == 10 || (serial_type == 7 && serialGet7(&aKey1[d1], &mem1) != 0)) {
      } else {
        rc = 1;
      }
    }

    if (rc != 0) {
      int sortFlags = pPKey2->pKeyInfo->aSortFlags[i];
      if (sortFlags) {
        if ((sortFlags & 0x02) == 0 || ((sortFlags & 0x01) != (serial_type == 0 || (pRhs->flags & 0x0001)))) {
          rc = -rc;
        }
      }

      return rc;
    }

    i++;
    if (i == pPKey2->nField)
      break;
    pRhs++;
    d1 += sqlite3VdbeSerialTypeLen(serial_type);
    if (d1 > (unsigned)nKey1)
      break;
    idx1 += sqlite3VarintLen(serial_type);
    if (idx1 >= (unsigned)szHdr1) {
      pPKey2->errCode = (u8)sqlite3CorruptError(92808);
      return 0;
    }
  }

  pPKey2->eqSeen = 1;
  return pPKey2->default_rc;
}

int sqlite3VdbeRecordCompare(int nKey1, const void *pKey1, UnpackedRecord *pPKey2) {
  return sqlite3VdbeRecordCompareWithSkip(nKey1, pKey1, pPKey2, 0);
}

int vdbeRecordCompareInt(int nKey1, const void *pKey1, UnpackedRecord *pPKey2) {
  const u8 *aKey = &((const u8 *)pKey1)[*(const u8 *)pKey1 & 0x3F];
  int serial_type = ((const u8 *)pKey1)[1];
  int res;
  u32 y;
  u64 x;
  i64 v;
  i64 lhs;

  switch (serial_type) {
    case 1: {
      lhs = ((i8)(aKey)[0]);
      break;
    }
    case 2: {
      lhs = (256 * (i8)((aKey)[0]) | (aKey)[1]);
      break;
    }
    case 3: {
      lhs = (65536 * (i8)((aKey)[0]) | ((aKey)[1] << 8) | (aKey)[2]);
      break;
    }
    case 4: {
      y = (((u32)(aKey)[0] << 24) | ((aKey)[1] << 16) | ((aKey)[2] << 8) | (aKey)[3]);
      lhs = (i64) * (int *)&y;
      break;
    }
    case 5: {
      lhs = (((u32)(aKey + 2)[0] << 24) | ((aKey + 2)[1] << 16) | ((aKey + 2)[2] << 8) | (aKey + 2)[3]) +
            (((i64)1) << 32) * (256 * (i8)((aKey)[0]) | (aKey)[1]);
      break;
    }
    case 6: {
      x = (((u32)(aKey)[0] << 24) | ((aKey)[1] << 16) | ((aKey)[2] << 8) | (aKey)[3]);
      x = (x << 32) | (((u32)(aKey + 4)[0] << 24) | ((aKey + 4)[1] << 16) | ((aKey + 4)[2] << 8) | (aKey + 4)[3]);
      lhs = *(i64 *)&x;
      break;
    }
    case 8:
      lhs = 0;
      break;
    case 9:
      lhs = 1;
      break;

    case 0:
    case 7:
      return sqlite3VdbeRecordCompare(nKey1, pKey1, pPKey2);

    default:
      return sqlite3VdbeRecordCompare(nKey1, pKey1, pPKey2);
  }

  v = pPKey2->u.i;
  if (v > lhs) {
    res = pPKey2->r1;
  } else if (v < lhs) {
    res = pPKey2->r2;
  } else if (pPKey2->nField > 1) {
    res = sqlite3VdbeRecordCompareWithSkip(nKey1, pKey1, pPKey2, 1);
  } else {
    res = pPKey2->default_rc;
    pPKey2->eqSeen = 1;
  }

  return res;
}

int vdbeRecordCompareString(int nKey1, const void *pKey1, UnpackedRecord *pPKey2) {
  const u8 *aKey1 = (const u8 *)pKey1;
  int serial_type;
  int res;

  serial_type = (signed char)(aKey1[1]);

vrcs_restart:
  if (serial_type < 12) {
    if (serial_type < 0) {
      sqlite3GetVarint32(&aKey1[1], (u32 *)&serial_type);
      if (serial_type >= 12)
        goto vrcs_restart;
    }
    res = pPKey2->r1;
  } else if (!(serial_type & 0x01)) {
    res = pPKey2->r2;
  } else {
    int nCmp;
    int nStr;
    int szHdr = aKey1[0];

    nStr = (serial_type - 12) / 2;
    if ((szHdr + nStr) > nKey1) {
      pPKey2->errCode = (u8)sqlite3CorruptError(92971);
      return 0;
    }
    nCmp = ((pPKey2->n) < (nStr) ? (pPKey2->n) : (nStr));
    res = memcmp(&aKey1[szHdr], pPKey2->u.z, nCmp);

    if (res > 0) {
      res = pPKey2->r2;
    } else if (res < 0) {
      res = pPKey2->r1;
    } else {
      res = nStr - pPKey2->n;
      if (res == 0) {
        if (pPKey2->nField > 1) {
          res = sqlite3VdbeRecordCompareWithSkip(nKey1, pKey1, pPKey2, 1);
        } else {
          res = pPKey2->default_rc;
          pPKey2->eqSeen = 1;
        }
      } else if (res > 0) {
        res = pPKey2->r2;
      } else {
        res = pPKey2->r1;
      }
    }
  }

  return res;
}

void sqlite3VdbeCountChanges(Vdbe *v) {
  v->changeCntOn = 1;
}

sqlite3 *sqlite3VdbeDb(Vdbe *v) {
  return v->db;
}

u8 sqlite3VdbePrepareFlags(Vdbe *v) {
  return v->prepFlags;
}

sqlite3_value *sqlite3VdbeGetBoundValue(Vdbe *v, int iVar, u8 aff) {
  if (v) {
    Mem *pMem = &v->aVar[iVar - 1];

    if (0 == (pMem->flags & 0x0001)) {
      sqlite3_value *pRet = sqlite3ValueNew(v->db);
      if (pRet) {
        sqlite3VdbeMemCopy((Mem *)pRet, pMem);
        sqlite3ValueApplyAffinity(pRet, aff, SQLITE_UTF8);
      }
      return pRet;
    }
  }
  return 0;
}

void sqlite3VdbeSetVarmask(Vdbe *v, int iVar) {
  if (iVar >= 32) {
    v->expmask |= 0x80000000;
  } else {
    v->expmask |= ((u32)1 << (iVar - 1));
  }
}

int vdbeSkipField(Bitmask mask, int iCol, Mem *pMem1, Mem *pMem2, int bIntegrity) {
  if (iCol >= ((int)(sizeof(Bitmask) * 8)) || (mask & (((Bitmask)1) << (iCol))) == 0)
    return 0;
  if (bIntegrity == 0)
    return 1;
  if ((pMem1->flags & 0x0008) && (pMem2->flags & 0x0008)) {
    u64 m1, m2;
    memcpy(&m1, &pMem1->u.r, 8);
    memcpy(&m2, &pMem2->u.r, 8);
    if ((m1 < m2 ? m2 - m1 : m1 - m2) <= 2) {
      return 1;
    }
  }
  return 0;
}

void sqlite3VtabImportErrmsg(Vdbe *p, sqlite3_vtab *pVtab) {
  if (pVtab->zErrMsg) {
    sqlite3 *db = p->db;
    sqlite3DbFree(db, p->zErrMsg);
    p->zErrMsg = sqlite3DbStrDup(db, pVtab->zErrMsg);
    sqlite3_free(pVtab->zErrMsg);
    pVtab->zErrMsg = 0;
  }
}

int vdbeSafety(Vdbe *p) {
  if (p->db == 0) {
    sqlite3_log(SQLITE_MISUSE, "API called with finalized prepared statement");
    return 1;
  } else {
    return 0;
  }
}

int vdbeSafetyNotNull(Vdbe *p) {
  if (p == 0) {
    sqlite3_log(SQLITE_MISUSE, "API called with NULL prepared statement");
    return 1;
  } else {
    return vdbeSafety(p);
  }
}

int sqlite3Step(Vdbe *p) {
  sqlite3 *db;
  int rc;

  db = p->db;
  if (p->eVdbeState != 2) {
  restart_step:
    if (p->eVdbeState == 1) {
      if (p->expired) {
        p->rc = SQLITE_SCHEMA;
        rc = SQLITE_ERROR;
        if ((p->prepFlags & 0x80) != 0) {
          rc = sqlite3VdbeTransferError(p);
        }
        goto end_of_step;
      }

      if (db->nVdbeActive == 0) {
        __atomic_store_n((&db->u1.isInterrupted), (0), 0);
      }

      if ((db->mTrace & (SQLITE_TRACE_PROFILE | 0x80)) != 0 && !db->init.busy && p->zSql) {
        sqlite3OsCurrentTimeInt64(db->pVfs, &p->startTime);
      } else {
      }

      db->nVdbeActive++;
      if (p->readOnly == 0)
        db->nVdbeWrite++;
      if (p->bIsReader)
        db->nVdbeRead++;
      p->pc = 0;
      p->eVdbeState = 2;
    } else if (p->eVdbeState == 3) {
      sqlite3_reset((sqlite3_stmt *)p);

      goto restart_step;
    }
  }

  if (p->explain) {
    rc = sqlite3VdbeList(p);
  } else {
    db->nVdbeExec++;
    rc = sqlite3VdbeExec(p);
    db->nVdbeExec--;
  }

  if (rc == SQLITE_ROW) {
    db->errCode = SQLITE_ROW;
    return SQLITE_ROW;
  } else {
    if (((p)->startTime) > 0) {
      invokeProfileCallback(db, p);
    };

    p->pResultRow = 0;
    if (rc == SQLITE_DONE && db->autoCommit) {
      p->rc = doWalCallbacks(db);
      if (p->rc != SQLITE_OK) {
        rc = SQLITE_ERROR;
      }
    } else if (rc != SQLITE_DONE && (p->prepFlags & 0x80) != 0) {
      rc = sqlite3VdbeTransferError(p);
    }
  }

  db->errCode = rc;
  if (SQLITE_NOMEM == sqlite3ApiExit(p->db, p->rc)) {
    p->rc = 7;
    if ((p->prepFlags & 0x80) != 0)
      rc = p->rc;
  }
end_of_step:
  return (rc & db->errMask);
}

void sqlite3VdbeValueListFree(void *pToDelete) {
  sqlite3_free(pToDelete);
}

int vdbeUnbind(Vdbe *p, unsigned int i) {
  Mem *pVar;
  if (vdbeSafetyNotNull(p)) {
    return sqlite3MisuseError(95345);
  }
  sqlite3_mutex_enter(p->db->mutex);
  if (p->eVdbeState != 1) {
    sqlite3Error(p->db, sqlite3MisuseError(95349));
    sqlite3_mutex_leave(p->db->mutex);
    sqlite3_log(SQLITE_MISUSE, "bind on a busy prepared statement: [%s]", p->zSql);
    return sqlite3MisuseError(95353);
  }
  if (i >= (unsigned int)p->nVar) {
    sqlite3Error(p->db, SQLITE_RANGE);
    sqlite3_mutex_leave(p->db->mutex);
    return SQLITE_RANGE;
  }
  pVar = &p->aVar[i];
  sqlite3VdbeMemRelease(pVar);
  pVar->flags = 0x0001;
  p->db->errCode = 0;

  if (p->expmask != 0 && (p->expmask & (i >= 31 ? 0x80000000 : (u32)1 << i)) != 0) {
    p->expired = 1;
  }
  return SQLITE_OK;
}

int sqlite3VdbeParameterIndex(Vdbe *p, const char *zName, int nName) {
  if (p == 0 || zName == 0)
    return 0;
  return sqlite3VListNameToNum(p->pVList, zName, nName);
}

char *sqlite3VdbeExpandSql(Vdbe *p, const char *zRawSql) {
  sqlite3 *db;
  int idx = 0;
  int nextIndex = 1;
  i64 n;
  i64 nToken;
  int i;
  Mem *pVar;
  StrAccum out;

  Mem utf8;

  db = p->db;
  sqlite3StrAccumInit(&out, 0, 0, 0, db->aLimit[SQLITE_LIMIT_LENGTH]);
  if (db->nVdbeExec > 1) {
    while (*zRawSql) {
      const char *zStart = zRawSql;
      while (*(zRawSql++) != '\n' && *zRawSql)
        ;
      sqlite3_str_append(&out, "-- ", 3);

      sqlite3_str_append(&out, zStart, (int)(zRawSql - zStart));
    }
  } else if (p->nVar == 0) {
    sqlite3_str_append(&out, zRawSql, sqlite3Strlen30(zRawSql));
  } else {
    while (zRawSql[0]) {
      n = findNextHostParameter(zRawSql, &nToken);

      sqlite3_str_append(&out, zRawSql, n);
      zRawSql += n;

      if (nToken == 0)
        break;
      if (zRawSql[0] == '?') {
        if (nToken > 1) {
          sqlite3GetInt32(&zRawSql[1], &idx);
        } else {
          idx = nextIndex;
        }
      } else {
        idx = sqlite3VdbeParameterIndex(p, zRawSql, nToken);
      }
      zRawSql += nToken;
      nextIndex = ((idx + 1) > (nextIndex) ? (idx + 1) : (nextIndex));

      pVar = &p->aVar[idx - 1];
      if (pVar->flags & 0x0001) {
        sqlite3_str_append(&out, "NULL", 4);
      } else if (pVar->flags & (0x0004 | 0x0020)) {
        sqlite3_str_appendf(&out, "%lld", pVar->u.i);
      } else if (pVar->flags & 0x0008) {
        sqlite3_str_appendf(&out, "%!.15g", pVar->u.r);
      } else if (pVar->flags & 0x0002) {
        int nOut;

        u8 enc = ((db)->enc);
        if (enc != SQLITE_UTF8) {
          memset(&utf8, 0, sizeof(utf8));
          utf8.db = db;
          sqlite3VdbeMemSetStr(&utf8, pVar->z, pVar->n, enc, ((sqlite3_destructor_type)0));
          if (SQLITE_NOMEM == sqlite3VdbeChangeEncoding(&utf8, SQLITE_UTF8)) {
            out.accError = SQLITE_NOMEM;
            out.nAlloc = 0;
          }
          pVar = &utf8;
        }

        nOut = pVar->n;

        sqlite3_str_appendf(&out, "'%.*q'", nOut, pVar->z);

        if (enc != SQLITE_UTF8)
          sqlite3VdbeMemRelease(&utf8);

      } else if (pVar->flags & 0x0400) {
        sqlite3_str_appendf(&out, "zeroblob(%d)", pVar->u.nZero);
      } else {
        int nOut;

        sqlite3_str_append(&out, "x'", 2);
        nOut = pVar->n;

        for (i = 0; i < nOut; i++) {
          sqlite3_str_appendf(&out, "%02x", pVar->z[i] & 0xff);
        }
        sqlite3_str_append(&out, "'", 1);
      }
    }
  }
  if (out.accError)
    sqlite3_str_reset(&out);
  return sqlite3StrAccumFinish(&out);
}

VdbeCursor *allocateCursor(Vdbe *p, int iCur, int nField, u8 eCurType) {
  Mem *pMem = iCur > 0 ? &p->aMem[p->nMem - iCur] : p->aMem;

  i64 nByte;
  VdbeCursor *pCx = 0;
  nByte = ((((offsetof(VdbeCursor, aType)) + 7) & ~7) + ((nField) + 1) * sizeof(u64));

  if (eCurType == 0)
    nByte += sqlite3BtreeCursorSize();

  if (p->apCsr[iCur]) {
    sqlite3VdbeFreeCursorNN(p, p->apCsr[iCur]);
    p->apCsr[iCur] = 0;
  }

  if (pMem->szMalloc < nByte) {
    if (pMem->szMalloc > 0) {
      sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
    }
    pMem->z = pMem->zMalloc = sqlite3DbMallocRaw(pMem->db, nByte);
    if (pMem->zMalloc == 0) {
      pMem->szMalloc = 0;
      return 0;
    }
    pMem->szMalloc = (int)nByte;
  }

  p->apCsr[iCur] = pCx = (VdbeCursor *)pMem->zMalloc;
  memset(pCx, 0, offsetof(VdbeCursor, pAltCursor));
  pCx->eCurType = eCurType;
  pCx->nField = nField;
  pCx->aOffset = &pCx->aType[nField];
  if (eCurType == 0) {
    pCx->uc.pCursor = (BtCursor *)&pMem->z[((((offsetof(VdbeCursor, aType)) + 7) & ~7) + ((nField) + 1) * sizeof(u64))];
    sqlite3BtreeCursorZero(pCx->uc.pCursor);
  }
  return pCx;
}

Mem *out2Prerelease(Vdbe *p, VdbeOp *pOp) {
  Mem *pOut;

  pOut = &p->aMem[pOp->p2];
  if ((((pOut)->flags & (0x8000 | 0x1000)) != 0)) {
    return out2PrereleaseWithClear(pOut);
  } else {
    pOut->flags = 0x0004;
    return pOut;
  }
}

__attribute__((noinline)) void sqlite3VdbeLogAbort(Vdbe *p, int rc, Op *pOp, Op *aOp) {
  const char *zSql = p->zSql;
  const char *zPrefix = "";
  int pc;
  char zXtra[100];

  if (p->pFrame) {
    if (aOp[0].p4.z != 0) {
      sqlite3_snprintf(sizeof(zXtra), zXtra, "/* %s */ ", aOp[0].p4.z + 3);
      zPrefix = zXtra;
    } else {
      zPrefix = "/* unknown trigger */ ";
    }
  }
  pc = (int)(pOp - aOp);
  sqlite3_log(rc, "statement aborts at %d: %s; [%s%s]", pc, p->zErrMsg, zPrefix, zSql);
}

int sqlite3VdbeExec(Vdbe *p) {
  Op *aOp = p->aOp;
  Op *pOp = aOp;

  int rc = 0;
  sqlite3 *db = p->db;
  u8 resetSchemaOnFault = 0;
  u8 encoding = ((db)->enc);
  int iCompare = 0;
  u64 nVmStep = 0;

  u64 nProgressLimit;

  Mem *aMem = p->aMem;
  Mem *pIn1 = 0;
  Mem *pIn2 = 0;
  Mem *pIn3 = 0;
  Mem *pOut = 0;
  u32 colCacheCtr = 0;

  if (((p->lockMask) != 0)) {
    sqlite3VdbeEnter(p);
  }

  if (db->xProgress) {
    u32 iPrior = p->aCounter[SQLITE_STMTSTATUS_VM_STEP];

    nProgressLimit = db->nProgressOps - (iPrior % db->nProgressOps);
  } else {
    nProgressLimit = (0xffffffff | (((u64)0xffffffff) << 32));
  }

  if (p->rc == SQLITE_NOMEM) {
    goto no_mem;
  }

  p->rc = 0;

  p->iCurrentTime = 0;

  db->busyHandler.nBusy = 0;
  if (__atomic_load_n((&db->u1.isInterrupted), 0))
    goto abort_due_to_interrupt;

  for (pOp = &aOp[p->pc]; 1; pOp++) {
    nVmStep++;

    switch (pOp->opcode) {
      case 9: {
      jump_to_p2_and_check_for_interrupt:
        pOp = &aOp[pOp->p2 - 1];

      check_for_interrupt:
        if (__atomic_load_n((&db->u1.isInterrupted), 0))
          goto abort_due_to_interrupt;

        while (nVmStep >= nProgressLimit && db->xProgress != 0) {
          nProgressLimit += db->nProgressOps;
          if (db->xProgress(db->pProgressArg)) {
            nProgressLimit = (0xffffffff | (((u64)0xffffffff) << 32));
            rc = SQLITE_INTERRUPT;
            goto abort_due_to_error;
          }
        }

        break;
      }

      case 10: {
        pIn1 = &aMem[pOp->p1];

        pIn1->flags = 0x0004;
        pIn1->u.i = (int)(pOp - aOp);
        goto jump_to_p2_and_check_for_interrupt;
      }

      case 69: {
        pIn1 = &aMem[pOp->p1];
        if (pIn1->flags & 0x0004) {
          if (pOp->p3) {
          }
          pOp = &aOp[pIn1->u.i];
        } else if ((pOp->p3)) {
        }
        break;
      }

      case 11: {
        pOut = &aMem[pOp->p1];

        pOut->u.i = pOp->p3 - 1;
        pOut->flags = 0x0004;
        if (pOp->p2 == 0)
          break;

      jump_to_p2:
        pOp = &aOp[pOp->p2 - 1];
        break;
      }

      case 70: {
        VdbeOp *pCaller;
        pIn1 = &aMem[pOp->p1];

        pCaller = &aOp[pIn1->u.i];

        pIn1->u.i = (int)(pOp - p->aOp) - 1;
        pOp = &aOp[pCaller->p2 - 1];
        break;
      }

      case 12: {
        int pcDest;
        pIn1 = &aMem[pOp->p1];

        pIn1->flags = 0x0004;
        pcDest = (int)pIn1->u.i;
        pIn1->u.i = (int)(pOp - aOp);
        pOp = &aOp[pcDest];
        break;
      }

      case 71: {
        pIn3 = &aMem[pOp->p3];

        if ((pIn3->flags & 0x0001) == 0)
          break;

        __attribute__((fallthrough));
      }

      case 72: {
        VdbeFrame *pFrame;
        int pcx;

        if (p->pFrame && pOp->p1 == SQLITE_OK) {
          pFrame = p->pFrame;
          p->pFrame = pFrame->pParent;
          p->nFrame--;
          sqlite3VdbeSetChanges(db, p->nChange);
          pcx = sqlite3VdbeFrameRestore(pFrame);
          if (pOp->p2 == 4) {
            pcx = p->aOp[pcx].p2 - 1;
          }
          aOp = p->aOp;
          aMem = p->aMem;
          pOp = &aOp[pcx];
          break;
        }
        p->rc = pOp->p1;
        p->errorAction = (u8)pOp->p2;

        if (p->rc) {
          if (pOp->p3 > 0 && pOp->p4type == 0) {
            const char *zErr;

            zErr = sqlite3ValueText(&aMem[pOp->p3], SQLITE_UTF8);
            sqlite3VdbeError(p, "%s", zErr);
          } else if (pOp->p5) {
            static const char *const azType[] = {"NOT NULL", "UNIQUE", "CHECK", "FOREIGN KEY"};
            sqlite3VdbeError(p, "%s constraint failed", azType[pOp->p5 - 1]);
            if (pOp->p4.z) {
              p->zErrMsg = sqlite3MPrintf(db, "%z: %s", p->zErrMsg, pOp->p4.z);
            }
          } else {
            sqlite3VdbeError(p, "%s", pOp->p4.z);
          }
          sqlite3VdbeLogAbort(p, pOp->p1, pOp, aOp);
        }
        rc = sqlite3VdbeHalt(p);

        if (rc == SQLITE_BUSY) {
          p->rc = SQLITE_BUSY;
        } else {
          rc = p->rc ? SQLITE_ERROR : SQLITE_DONE;
        }
        goto vdbe_return;
      }

      case 73: {
        pOut = out2Prerelease(p, pOp);
        pOut->u.i = pOp->p1;
        break;
      }

      case 74: {
        pOut = out2Prerelease(p, pOp);

        pOut->u.i = *pOp->p4.pI64;
        break;
      }

      case 154: {
        pOut = out2Prerelease(p, pOp);
        pOut->flags = 0x0008;

        pOut->u.r = *pOp->p4.pReal;
        break;
      }

      case 118: {
        pOut = out2Prerelease(p, pOp);
        pOp->p1 = sqlite3Strlen30(pOp->p4.z);

        if (encoding != SQLITE_UTF8) {
          rc = sqlite3VdbeMemSetStr(pOut, pOp->p4.z, -1, SQLITE_UTF8, ((sqlite3_destructor_type)0));

          if (rc)
            goto too_big;
          if (SQLITE_OK != sqlite3VdbeChangeEncoding(pOut, encoding))
            goto no_mem;

          pOut->szMalloc = 0;
          pOut->flags |= 0x2000;
          if (pOp->p4type == (-7)) {
            sqlite3DbFree(db, pOp->p4.z);
          }
          pOp->p4type = (-7);
          pOp->p4.z = pOut->z;
          pOp->p1 = pOut->n;
        }

        if (pOp->p1 > db->aLimit[SQLITE_LIMIT_LENGTH]) {
          goto too_big;
        }
        pOp->opcode = 75;

        __attribute__((fallthrough));
      }

      case 75: {
        pOut = out2Prerelease(p, pOp);
        pOut->flags = 0x0002 | 0x2000 | 0x0200;
        pOut->z = pOp->p4.z;
        pOut->n = pOp->p1;
        pOut->enc = encoding;

        if (pOp->p3 > 0) {
          pIn3 = &aMem[pOp->p3];

          if (pIn3->u.i == pOp->p5)
            pOut->flags = 0x0010 | 0x2000 | 0x0200;
        }

        break;
      }

      case 76:
      case 77: {
        int cnt;
        u16 nullFlag;
        pOut = out2Prerelease(p, pOp);
        cnt = pOp->p3 - pOp->p2;

        pOut->flags = nullFlag = pOp->p1 ? (0x0001 | 0x0100) : 0x0001;
        pOut->n = 0;

        while (cnt > 0) {
          pOut++;
          sqlite3VdbeMemSetNull(pOut);
          pOut->flags = nullFlag;
          pOut->n = 0;
          cnt--;
        }
        break;
      }

      case 78: {
        pOut = &aMem[pOp->p1];
        pOut->flags = (pOut->flags & ~(0x0000 | 0x003f)) | 0x0001;
        break;
      }

      case 79: {
        pOut = out2Prerelease(p, pOp);
        if (pOp->p4.z == 0) {
          sqlite3VdbeMemSetZeroBlob(pOut, pOp->p1);
          if (sqlite3VdbeMemExpandBlob(pOut))
            goto no_mem;
        } else {
          sqlite3VdbeMemSetStr(pOut, pOp->p4.z, pOp->p1, 0, 0);
        }
        pOut->enc = encoding;
        break;
      }

      case 80: {
        Mem *pVar;

        pVar = &p->aVar[pOp->p1 - 1];
        if (sqlite3VdbeMemTooBig(pVar)) {
          goto too_big;
        }
        pOut = &aMem[pOp->p2];
        if ((((pOut)->flags & (0x8000 | 0x1000)) != 0))
          sqlite3VdbeMemSetNull(pOut);
        memcpy(pOut, pVar, offsetof(Mem, db));
        pOut->flags &= ~(0x1000 | 0x4000);
        pOut->flags |= 0x2000 | 0x0040;
        break;
      }

      case 81: {
        int n;
        int p1;
        int p2;

        n = pOp->p3;
        p1 = pOp->p1;
        p2 = pOp->p2;

        pIn1 = &aMem[p1];
        pOut = &aMem[p2];
        do {
          sqlite3VdbeMemMove(pOut, pIn1);

          if (((pOut)->flags & 0x4000) != 0 && sqlite3VdbeMemMakeWriteable(pOut)) {
            goto no_mem;
          };
          pIn1++;
          pOut++;
        } while (--n);
        break;
      }

      case 82: {
        int n;

        n = pOp->p3;
        pIn1 = &aMem[pOp->p1];
        pOut = &aMem[pOp->p2];

        while (1) {
          sqlite3VdbeMemShallowCopy(pOut, pIn1, 0x4000);
          if (((pOut)->flags & 0x4000) != 0 && sqlite3VdbeMemMakeWriteable(pOut)) {
            goto no_mem;
          };
          if ((pOut->flags & 0x0800) != 0 && (pOp->p5 & 0x0002) != 0) {
            pOut->flags &= ~0x0800;
          }

          if ((n--) == 0)
            break;
          pOut++;
          pIn1++;
        }
        break;
      }

      case 83: {
        pIn1 = &aMem[pOp->p1];
        pOut = &aMem[pOp->p2];

        sqlite3VdbeMemShallowCopy(pOut, pIn1, 0x4000);

        break;
      }

      case 84: {
        pIn1 = &aMem[pOp->p1];

        pOut = &aMem[pOp->p2];
        sqlite3VdbeMemSetInt64(pOut, pIn1->u.i);
        break;
      }

      case 85: {
        if ((rc = sqlite3VdbeCheckFkImmediate(p)) != SQLITE_OK) {
          goto abort_due_to_error;
        }
        break;
      }

      case 86: {
        p->cacheCtr = (p->cacheCtr + 2) | 1;
        p->pResultRow = &aMem[pOp->p1];

        if (db->mallocFailed)
          goto no_mem;
        if (db->mTrace & SQLITE_TRACE_ROW) {
          db->trace.xV2(SQLITE_TRACE_ROW, db->pTraceArg, p, 0);
        }
        p->pc = (int)(pOp - aOp) + 1;
        rc = SQLITE_ROW;
        goto vdbe_return;
      }

      case 112: {
        i64 nByte;
        u16 flags1;
        u16 flags2;

        pIn1 = &aMem[pOp->p1];
        pIn2 = &aMem[pOp->p2];
        pOut = &aMem[pOp->p3];

        flags1 = pIn1->flags;
        if ((flags1 | pIn2->flags) & 0x0001) {
          sqlite3VdbeMemSetNull(pOut);
          break;
        }
        if ((flags1 & (0x0002 | 0x0010)) == 0) {
          if (sqlite3VdbeMemStringify(pIn1, encoding, 0))
            goto no_mem;
          flags1 = pIn1->flags & ~0x0002;
        } else if ((flags1 & 0x0400) != 0) {
          if (sqlite3VdbeMemExpandBlob(pIn1))
            goto no_mem;
          flags1 = pIn1->flags & ~0x0002;
        }
        flags2 = pIn2->flags;
        if ((flags2 & (0x0002 | 0x0010)) == 0) {
          if (sqlite3VdbeMemStringify(pIn2, encoding, 0))
            goto no_mem;
          flags2 = pIn2->flags & ~0x0002;
        } else if ((flags2 & 0x0400) != 0) {
          if (sqlite3VdbeMemExpandBlob(pIn2))
            goto no_mem;
          flags2 = pIn2->flags & ~0x0002;
        }
        nByte = pIn1->n;
        nByte += pIn2->n;
        if (nByte > db->aLimit[SQLITE_LIMIT_LENGTH]) {
          goto too_big;
        }

        if (sqlite3VdbeMemGrow(pOut, (int)nByte + 2, pOut == pIn2)) {
          goto no_mem;
        }
        ((pOut)->flags = ((pOut)->flags & ~(0x0dbf | 0x0400)) | 0x0002);
        if (pOut != pIn2) {
          memcpy(pOut->z, pIn2->z, pIn2->n);

          pIn2->flags = flags2;
        }
        memcpy(&pOut->z[pIn2->n], pIn1->z, pIn1->n);

        pIn1->flags = flags1;
        if (encoding > SQLITE_UTF8)
          nByte &= ~1;
        pOut->z[nByte] = 0;
        pOut->z[nByte + 1] = 0;
        pOut->flags |= 0x0200;
        pOut->n = (int)nByte;
        pOut->enc = encoding;
        break;
      }

      case 107:
      case 108:
      case 109:
      case 110:
      case 111: {
        u16 type1;
        u16 type2;
        i64 iA;
        i64 iB;
        double rA;
        double rB;

        pIn1 = &aMem[pOp->p1];
        type1 = pIn1->flags;
        pIn2 = &aMem[pOp->p2];
        type2 = pIn2->flags;
        pOut = &aMem[pOp->p3];
        if ((type1 & type2 & 0x0004) != 0) {
        int_math:
          iA = pIn1->u.i;
          iB = pIn2->u.i;
          switch (pOp->opcode) {
            case 107:
              if (sqlite3AddInt64(&iB, iA))
                goto fp_math;
              break;
            case 108:
              if (sqlite3SubInt64(&iB, iA))
                goto fp_math;
              break;
            case 109:
              if (sqlite3MulInt64(&iB, iA))
                goto fp_math;
              break;
            case 110: {
              if (iA == 0)
                goto arithmetic_result_is_null;
              if (iA == -1 && iB == (((i64)-1) - (0xffffffff | (((i64)0x7fffffff) << 32))))
                goto fp_math;
              iB /= iA;
              break;
            }
            default: {
              if (iA == 0)
                goto arithmetic_result_is_null;
              if (iA == -1)
                iA = 1;
              iB %= iA;
              break;
            }
          }
          pOut->u.i = iB;
          ((pOut)->flags = ((pOut)->flags & ~(0x0dbf | 0x0400)) | 0x0004);
        } else if (((type1 | type2) & 0x0001) != 0) {
          goto arithmetic_result_is_null;
        } else {
          type1 = numericType(pIn1);
          type2 = numericType(pIn2);
          if ((type1 & type2 & 0x0004) != 0)
            goto int_math;
        fp_math:
          rA = sqlite3VdbeRealValue(pIn1);
          rB = sqlite3VdbeRealValue(pIn2);
          switch (pOp->opcode) {
            case 107:
              rB += rA;
              break;
            case 108:
              rB -= rA;
              break;
            case 109:
              rB *= rA;
              break;
            case 110: {
              if (rA == (double)0)
                goto arithmetic_result_is_null;
              rB /= rA;
              break;
            }
            default: {
              iA = sqlite3VdbeIntValue(pIn1);
              iB = sqlite3VdbeIntValue(pIn2);
              if (iA == 0)
                goto arithmetic_result_is_null;
              if (iA == -1)
                iA = 1;
              rB = (double)(iB % iA);
              break;
            }
          }

          if (sqlite3IsNaN(rB)) {
            goto arithmetic_result_is_null;
          }
          pOut->u.r = rB;
          ((pOut)->flags = ((pOut)->flags & ~(0x0dbf | 0x0400)) | 0x0008);
        }
        break;

      arithmetic_result_is_null:
        sqlite3VdbeMemSetNull(pOut);
        break;
      }

      case 87: {
        if (pOp->p1) {
          sqlite3VdbeMemSetInt64(&aMem[pOp->p1], 0);
        }
        break;
      }

      case 103:
      case 104:
      case 105:
      case 106: {
        i64 iA;
        u64 uA;
        i64 iB;
        u8 op;

        pIn1 = &aMem[pOp->p1];
        pIn2 = &aMem[pOp->p2];
        pOut = &aMem[pOp->p3];
        if ((pIn1->flags | pIn2->flags) & 0x0001) {
          sqlite3VdbeMemSetNull(pOut);
          break;
        }
        iA = sqlite3VdbeIntValue(pIn2);
        iB = sqlite3VdbeIntValue(pIn1);
        op = pOp->opcode;
        if (op == 103) {
          iA &= iB;
        } else if (op == 104) {
          iA |= iB;
        } else if (iB != 0) {
          if (iB < 0) {
            op = 2 * 105 + 1 - op;
            iB = iB > (-64) ? -iB : 64;
          }

          if (iB >= 64) {
            iA = (iA >= 0 || op == 105) ? 0 : -1;
          } else {
            memcpy(&uA, &iA, sizeof(uA));
            if (op == 105) {
              uA <<= iB;
            } else {
              uA >>= iB;

              if (iA < 0)
                uA |= ((((u64)0xffffffff) << 32) | 0xffffffff) << (64 - iB);
            }
            memcpy(&iA, &uA, sizeof(iA));
          }
        }
        pOut->u.i = iA;
        ((pOut)->flags = ((pOut)->flags & ~(0x0dbf | 0x0400)) | 0x0004);
        break;
      }

      case 88: {
        pIn1 = &aMem[pOp->p1];
        sqlite3VdbeMemIntegerify(pIn1);
        *(u64 *)&pIn1->u.i += (u64)pOp->p2;
        break;
      }

      case 13: {
        pIn1 = &aMem[pOp->p1];
        if ((pIn1->flags & 0x0004) == 0) {
          applyAffinity(pIn1, 0x43, encoding);
          if ((pIn1->flags & 0x0004) == 0) {
            if (pOp->p2 == 0) {
              rc = SQLITE_MISMATCH;
              goto abort_due_to_error;
            } else {
              goto jump_to_p2;
            }
          }
        };
        ((pIn1)->flags = ((pIn1)->flags & ~(0x0dbf | 0x0400)) | 0x0004);
        break;
      }

      case 89: {
        pIn1 = &aMem[pOp->p1];
        if (pIn1->flags & (0x0004 | 0x0020)) {
          sqlite3VdbeMemRealify(pIn1);
        }
        break;
      }

      case 90: {
        pIn1 = &aMem[pOp->p1];
        rc = (((pIn1)->flags & 0x0400) ? sqlite3VdbeMemExpandBlob(pIn1) : 0);
        if (rc)
          goto abort_due_to_error;
        rc = sqlite3VdbeMemCast(pIn1, pOp->p2, encoding);
        if (rc)
          goto abort_due_to_error;
        break;
      }

      case 54:
      case 53:
      case 57:
      case 56:
      case 55:
      case 58: {
        int res, res2;
        char affinity;
        u16 flags1;
        u16 flags3;

        pIn1 = &aMem[pOp->p1];
        pIn3 = &aMem[pOp->p3];
        flags1 = pIn1->flags;
        flags3 = pIn3->flags;
        if ((flags1 & flags3 & 0x0004) != 0) {
          if (pIn3->u.i > pIn1->u.i) {
            if (sqlite3aGTb[pOp->opcode]) {
              goto jump_to_p2;
            }
            iCompare = +1;

          } else if (pIn3->u.i < pIn1->u.i) {
            if (sqlite3aLTb[pOp->opcode]) {
              goto jump_to_p2;
            }
            iCompare = -1;

          } else {
            if (sqlite3aEQb[pOp->opcode]) {
              goto jump_to_p2;
            }
            iCompare = 0;
          };
          break;
        }
        if ((flags1 | flags3) & 0x0001) {
          if (pOp->p5 & 0x80) {
            if ((flags1 & flags3 & 0x0001) != 0 && (flags3 & 0x0100) == 0) {
              res = 0;
            } else {
              res = ((flags3 & 0x0001) ? -1 : +1);
            }
          } else {
            if (pOp->p5 & 0x10) {
              goto jump_to_p2;
            }
            iCompare = 1;

            break;
          }
        } else {
          affinity = pOp->p5 & 0x47;
          if (affinity >= 0x43) {
            if ((flags1 | flags3) & 0x0002) {
              if ((flags1 & (0x0004 | 0x0020 | 0x0008 | 0x0002)) == 0x0002) {
                applyNumericAffinity(pIn1, 0);

                flags3 = pIn3->flags;
              }
              if ((flags3 & (0x0004 | 0x0020 | 0x0008 | 0x0002)) == 0x0002) {
                applyNumericAffinity(pIn3, 0);
              }
            }
          } else if (affinity == 0x42 && ((flags1 | flags3) & 0x0002) != 0) {
            if ((flags1 & 0x0002) != 0) {
              pIn1->flags &= ~(0x0004 | 0x0008 | 0x0020);
            } else if ((flags1 & (0x0004 | 0x0008 | 0x0020)) != 0) {
              sqlite3VdbeMemStringify(pIn1, encoding, 1);
              flags1 = (pIn1->flags & ~0x0dbf) | (flags1 & 0x0dbf);
              if (pIn1 == pIn3)
                flags3 = flags1 | 0x0002;
            }
            if ((flags3 & 0x0002) != 0) {
              pIn3->flags &= ~(0x0004 | 0x0008 | 0x0020);
            } else if ((flags3 & (0x0004 | 0x0008 | 0x0020)) != 0) {
              sqlite3VdbeMemStringify(pIn3, encoding, 1);
              flags3 = (pIn3->flags & ~0x0dbf) | (flags3 & 0x0dbf);
            }
          }

          res = sqlite3MemCompare(pIn3, pIn1, pOp->p4.pColl);
        }

        if (res < 0) {
          res2 = sqlite3aLTb[pOp->opcode];
        } else if (res == 0) {
          res2 = sqlite3aEQb[pOp->opcode];
        } else {
          res2 = sqlite3aGTb[pOp->opcode];
        }
        iCompare = res;

        pIn3->flags = flags3;

        pIn1->flags = flags1;

        if (res2) {
          goto jump_to_p2;
        }
        break;
      }

      case 59: {
        if (iCompare == 0)
          goto jump_to_p2;
        break;
      }

      case 91: {
        break;
      }

      case 92: {
        int n;
        int i;
        int p1;
        int p2;
        const KeyInfo *pKeyInfo;
        u32 idx;
        CollSeq *pColl;
        int bRev;
        u32 *aPermute;

        if ((pOp->p5 & 0x01) == 0) {
          aPermute = 0;
        } else {
          aPermute = pOp[-1].p4.ai + 1;
        }
        n = pOp->p3;
        pKeyInfo = pOp->p4.pKeyInfo;

        p1 = pOp->p1;
        p2 = pOp->p2;

        for (i = 0; i < n; i++) {
          idx = aPermute ? aPermute[i] : (u32)i;

          pColl = pKeyInfo->aColl[i];
          bRev = (pKeyInfo->aSortFlags[i] & 0x01);
          iCompare = sqlite3MemCompare(&aMem[p1 + idx], &aMem[p2 + idx], pColl);

          if (iCompare) {
            if ((pKeyInfo->aSortFlags[i] & 0x02) &&
                ((aMem[p1 + idx].flags & 0x0001) || (aMem[p2 + idx].flags & 0x0001))) {
              iCompare = -iCompare;
            }
            if (bRev)
              iCompare = -iCompare;
            break;
          }
        }

        break;
      }

      case 14: {
        if (iCompare < 0) {
          pOp = &aOp[pOp->p1 - 1];
        } else if (iCompare == 0) {
          pOp = &aOp[pOp->p2 - 1];
        } else {
          pOp = &aOp[pOp->p3 - 1];
        }
        break;
      }

      case 44:
      case 43: {
        int v1;
        int v2;

        v1 = sqlite3VdbeBooleanValue(&aMem[pOp->p1], 2);
        v2 = sqlite3VdbeBooleanValue(&aMem[pOp->p2], 2);
        if (pOp->opcode == 44) {
          static const unsigned char and_logic[] = {0, 0, 0, 0, 1, 2, 0, 2, 2};
          v1 = and_logic[v1 * 3 + v2];
        } else {
          static const unsigned char or_logic[] = {0, 1, 2, 1, 1, 1, 2, 1, 2};
          v1 = or_logic[v1 * 3 + v2];
        }
        pOut = &aMem[pOp->p3];
        if (v1 == 2) {
          ((pOut)->flags = ((pOut)->flags & ~(0x0dbf | 0x0400)) | 0x0001);
        } else {
          pOut->u.i = v1;
          ((pOut)->flags = ((pOut)->flags & ~(0x0dbf | 0x0400)) | 0x0004);
        }
        break;
      }

      case 93: {
        sqlite3VdbeMemSetInt64(&aMem[pOp->p2], sqlite3VdbeBooleanValue(&aMem[pOp->p1], pOp->p3) ^ pOp->p4.i);
        break;
      }

      case 19: {
        pIn1 = &aMem[pOp->p1];
        pOut = &aMem[pOp->p2];
        if ((pIn1->flags & 0x0001) == 0) {
          sqlite3VdbeMemSetInt64(pOut, !sqlite3VdbeBooleanValue(pIn1, 0));
        } else {
          sqlite3VdbeMemSetNull(pOut);
        }
        break;
      }

      case 115: {
        pIn1 = &aMem[pOp->p1];
        pOut = &aMem[pOp->p2];
        sqlite3VdbeMemSetNull(pOut);
        if ((pIn1->flags & 0x0001) == 0) {
          pOut->flags = 0x0004;
          pOut->u.i = ~sqlite3VdbeIntValue(pIn1);
        }
        break;
      }

      case 15: {
        u32 iAddr;

        if (p->pFrame) {
          iAddr = (int)(pOp - p->aOp);
          if ((p->pFrame->aOnce[iAddr / 8] & (1 << (iAddr & 7))) != 0) {
            goto jump_to_p2;
          }
          p->pFrame->aOnce[iAddr / 8] |= 1 << (iAddr & 7);
        } else {
          if (p->aOp[0].p1 == pOp->p1) {
            goto jump_to_p2;
          }
        };
        pOp->p1 = p->aOp[0].p1;
        break;
      }

      case 16: {
        int c;
        c = sqlite3VdbeBooleanValue(&aMem[pOp->p1], pOp->p3);
        if (c)
          goto jump_to_p2;
        break;
      }

      case 17: {
        int c;
        c = !sqlite3VdbeBooleanValue(&aMem[pOp->p1], !pOp->p3);
        if (c)
          goto jump_to_p2;
        break;
      }

      case 51: {
        pIn1 = &aMem[pOp->p1];
        if ((pIn1->flags & 0x0001) != 0) {
          goto jump_to_p2;
        }
        break;
      }

      case 18: {
        VdbeCursor *pC;
        u16 typeMask;
        u32 serialType;

        if (pOp->p1 >= 0) {
          pC = p->apCsr[pOp->p1];

          if (pOp->p3 < pC->nHdrParsed) {
            serialType = pC->aType[pOp->p3];
            if (serialType >= 12) {
              if (serialType & 1) {
                typeMask = 0x04;
              } else {
                typeMask = 0x08;
              }
            } else {
              static const unsigned char aMask[] = {0x10, 0x01, 0x01, 0x01, 0x01, 0x01,
                                                    0x01, 0x2,  0x01, 0x01, 0x10, 0x10};
              typeMask = aMask[serialType];
            }
          } else {
            typeMask = 1 << (pOp->p4.i - 1);
          }
        } else {
          typeMask = 1 << (sqlite3_value_type((sqlite3_value *)&aMem[pOp->p3]) - 1);
        };
        if (typeMask & pOp->p5) {
          goto jump_to_p2;
        }
        break;
      }

      case 94: {
        if ((aMem[pOp->p1].flags & 0x0001) != 0 || (aMem[pOp->p3].flags & 0x0001) != 0) {
          sqlite3VdbeMemSetNull(aMem + pOp->p2);
        } else {
          sqlite3VdbeMemSetInt64(aMem + pOp->p2, 0);
        }
        break;
      }

      case 52: {
        pIn1 = &aMem[pOp->p1];
        if ((pIn1->flags & 0x0001) == 0) {
          goto jump_to_p2;
        }
        break;
      }

      case 20: {
        VdbeCursor *pC;

        pC = p->apCsr[pOp->p1];
        if (pC && pC->nullRow) {
          sqlite3VdbeMemSetNull(aMem + pOp->p3);
          goto jump_to_p2;
        }
        break;
      }

      case 96: {
        u32 p2;
        VdbeCursor *pC;
        BtCursor *pCrsr;
        u32 *aOffset;
        int len;
        int i;
        Mem *pDest;
        Mem sMem;
        const u8 *zData;
        const u8 *zHdr;
        const u8 *zEndHdr;
        u64 offset64;
        u32 t;
        Mem *pReg;

        pC = p->apCsr[pOp->p1];
        p2 = (u32)pOp->p2;

      op_column_restart:
        aOffset = pC->aOffset;

        if (pC->cacheStatus != p->cacheCtr) {
          if (pC->nullRow) {
            if (pC->eCurType == 3 && pC->seekResult > 0) {
              pReg = &aMem[pC->seekResult];

              pC->payloadSize = pC->szRow = pReg->n;
              pC->aRow = (u8 *)pReg->z;
            } else {
              pDest = &aMem[pOp->p3];
              sqlite3VdbeMemSetNull(pDest);
              goto op_column_out;
            }
          } else {
            pCrsr = pC->uc.pCursor;
            if (pC->deferredMoveto) {
              u32 iMap;

              if (pC->ub.aAltMap && (iMap = pC->ub.aAltMap[1 + p2]) > 0) {
                pC = pC->pAltCursor;
                p2 = iMap - 1;
                goto op_column_restart;
              }
              rc = sqlite3VdbeFinishMoveto(pC);
              if (rc)
                goto abort_due_to_error;
            } else if (sqlite3BtreeCursorHasMoved(pCrsr)) {
              rc = sqlite3VdbeHandleMovedCursor(pC);
              if (rc)
                goto abort_due_to_error;
              goto op_column_restart;
            }

            pC->payloadSize = sqlite3BtreePayloadSize(pCrsr);
            pC->aRow = sqlite3BtreePayloadFetch(pCrsr, &pC->szRow);
          }
          pC->cacheStatus = p->cacheCtr;
          if ((aOffset[0] = pC->aRow[0]) < 0x80) {
            pC->iHdrOffset = 1;
          } else {
            pC->iHdrOffset = sqlite3GetVarint32(pC->aRow, aOffset);
          }
          pC->nHdrParsed = 0;

          if (pC->szRow < aOffset[0]) {
            pC->aRow = 0;
            pC->szRow = 0;

            if (aOffset[0] > 98307 || aOffset[0] > pC->payloadSize) {
              goto op_column_corrupt;
            }
          } else {
            zData = pC->aRow;

            goto op_column_read_header;
          }
        } else if (sqlite3BtreeCursorHasMoved(pC->uc.pCursor)) {
          rc = sqlite3VdbeHandleMovedCursor(pC);
          if (rc)
            goto abort_due_to_error;
          goto op_column_restart;
        }

        if (pC->nHdrParsed <= p2) {
          if (pC->iHdrOffset < aOffset[0]) {
            if (pC->aRow == 0) {
              memset(&sMem, 0, sizeof(sMem));
              rc = sqlite3VdbeMemFromBtreeZeroOffset(pC->uc.pCursor, aOffset[0], &sMem);
              if (rc != SQLITE_OK)
                goto abort_due_to_error;
              zData = (u8 *)sMem.z;
            } else {
              zData = pC->aRow;
            }

          op_column_read_header:
            i = pC->nHdrParsed;
            offset64 = aOffset[i];
            zHdr = zData + pC->iHdrOffset;
            zEndHdr = zData + aOffset[0];
            do {
              if ((pC->aType[i] = t = zHdr[0]) < 0x80) {
                zHdr++;
                offset64 += sqlite3VdbeOneByteSerialTypeLen(t);
              } else {
                zHdr += sqlite3GetVarint32(zHdr, &t);
                pC->aType[i] = t;
                offset64 += sqlite3VdbeSerialTypeLen(t);
              }
              aOffset[++i] = (u32)(offset64 & 0xffffffff);
            } while ((u32)i <= p2 && zHdr < zEndHdr);

            if ((zHdr >= zEndHdr && (zHdr > zEndHdr || offset64 != pC->payloadSize)) || (offset64 > pC->payloadSize)) {
              if (aOffset[0] == 0) {
                i = 0;
                zHdr = zEndHdr;
              } else {
                if (pC->aRow == 0)
                  sqlite3VdbeMemRelease(&sMem);
                goto op_column_corrupt;
              }
            }

            pC->nHdrParsed = i;
            pC->iHdrOffset = (u32)(zHdr - zData);
            if (pC->aRow == 0)
              sqlite3VdbeMemRelease(&sMem);
          } else {
            t = 0;
          }

          if (pC->nHdrParsed <= p2) {
            pDest = &aMem[pOp->p3];
            if (pOp->p4type == (-11)) {
              sqlite3VdbeMemShallowCopy(pDest, pOp->p4.pMem, 0x2000);
            } else {
              sqlite3VdbeMemSetNull(pDest);
            }
            goto op_column_out;
          }
        } else {
          t = pC->aType[p2];
        }

        pDest = &aMem[pOp->p3];

        if ((((pDest)->flags & (0x8000 | 0x1000)) != 0)) {
          sqlite3VdbeMemSetNull(pDest);
        }

        if (pC->szRow >= aOffset[p2 + 1]) {
          zData = pC->aRow + aOffset[p2];
          if (t < 12) {
            sqlite3VdbeSerialGet(zData, t, pDest);
          } else {
            static const u16 aFlag[] = {0x0010, 0x0002 | 0x0200};
            pDest->n = len = (t - 12) / 2;
            pDest->enc = encoding;
            if (pDest->szMalloc < len + 2) {
              if (len > db->aLimit[SQLITE_LIMIT_LENGTH])
                goto too_big;
              pDest->flags = 0x0001;
              if (sqlite3VdbeMemGrow(pDest, len + 2, 0))
                goto no_mem;
            } else {
              pDest->z = pDest->zMalloc;
            }
            memcpy(pDest->z, zData, len);
            pDest->z[len] = 0;
            pDest->z[len + 1] = 0;
            pDest->flags = aFlag[t & 1];
          }
        } else {
          u8 p5;
          pDest->enc = encoding;

          if (((p5 = (pOp->p5 & 0xc0)) != 0 && (p5 == 0x80 || (t >= 12 && ((t & 1) == 0 || p5 == 0xc0)))) ||
              sqlite3VdbeSerialTypeLen(t) == 0) {
            sqlite3VdbeSerialGet((u8 *)sqlite3CtypeMap, t, pDest);
          } else {
            rc = vdbeColumnFromOverflow(pC, p2, t, aOffset[p2], p->cacheCtr, colCacheCtr, pDest);
            if (rc) {
              if (rc == SQLITE_NOMEM)
                goto no_mem;
              if (rc == SQLITE_TOOBIG)
                goto too_big;
              goto abort_due_to_error;
            }
          }
        }

      op_column_out:;
        break;

      op_column_corrupt:
        if (aOp[0].p3 > 0) {
          pOp = &aOp[aOp[0].p3 - 1];
          break;
        } else {
          rc = sqlite3CorruptError(99871);
          goto abort_due_to_error;
        }
      }

      case 97: {
        Table *pTab;
        Column *aCol;
        int i;
        int nCol;

        pTab = pOp->p4.pTab;

        aCol = pTab->aCol;
        pIn1 = &aMem[pOp->p1];
        if (pOp->p3 < 2) {
          i = 0;
          nCol = pTab->nCol;
        } else {
          i = pOp->p3 - 2;
          nCol = i + 1;
        }
        for (; i < nCol; i++) {
          if ((aCol[i].colFlags & 0x0060) != 0 && pOp->p3 < 2) {
            if ((aCol[i].colFlags & 0x0020) != 0)
              continue;
            if (pOp->p3) {
              pIn1++;
              continue;
            }
          }

          applyAffinity(pIn1, aCol[i].affinity, encoding);
          if ((pIn1->flags & 0x0001) == 0) {
            switch (aCol[i].eCType) {
              case 2: {
                if ((pIn1->flags & 0x0010) == 0)
                  goto vdbe_type_error;
                break;
              }
              case 4:
              case 3: {
                if ((pIn1->flags & 0x0004) == 0)
                  goto vdbe_type_error;
                break;
              }
              case 6: {
                if ((pIn1->flags & 0x0002) == 0)
                  goto vdbe_type_error;
                break;
              }
              case 5: {
                if (pIn1->flags & 0x0004) {
                  if (pIn1->u.i <= 140737488355327LL && pIn1->u.i >= -140737488355328LL) {
                    pIn1->flags |= 0x0020;
                    pIn1->flags &= ~0x0004;
                  } else {
                    pIn1->u.r = (double)pIn1->u.i;
                    pIn1->flags |= 0x0008;
                    pIn1->flags &= ~0x0004;
                  }
                } else if ((pIn1->flags & (0x0008 | 0x0020)) == 0) {
                  goto vdbe_type_error;
                }
                break;
              }
              default: {
                break;
              }
            }
          };
          pIn1++;
        }

        break;

      vdbe_type_error:
        sqlite3VdbeError(p, "cannot store %s value in %s column %s.%s", vdbeMemTypeName(pIn1),
                         sqlite3StdType[aCol[i].eCType - 1], pTab->zName, aCol[i].zCnName);
        rc = (19 | (12 << 8));
        goto abort_due_to_error;
      }

      case 98: {
        const char *zAffinity;

        zAffinity = pOp->p4.z;

        pIn1 = &aMem[pOp->p1];
        while (1) {
          applyAffinity(pIn1, zAffinity[0], encoding);
          if (zAffinity[0] == 0x45 && (pIn1->flags & 0x0004) != 0) {
            if (pIn1->u.i <= 140737488355327LL && pIn1->u.i >= -140737488355328LL) {
              pIn1->flags |= 0x0020;
              pIn1->flags &= ~0x0004;
            } else {
              pIn1->u.r = (double)pIn1->u.i;
              pIn1->flags |= 0x0008;
              pIn1->flags &= ~(0x0004 | 0x0002);
            }
          };
          zAffinity++;
          if (zAffinity[0] == 0)
            break;
          pIn1++;
        }
        break;
      }

      case 99: {
        Mem *pRec;
        u64 nData;
        int nHdr;
        i64 nByte;
        i64 nZero;
        int nVarint;
        u32 serial_type;
        Mem *pData0;
        Mem *pLast;
        int nField;
        char *zAffinity;
        u32 len;
        u8 *zHdr;
        u8 *zPayload;

        nData = 0;
        nHdr = 0;
        nZero = 0;
        nField = pOp->p1;
        zAffinity = pOp->p4.z;

        pData0 = &aMem[nField];
        nField = pOp->p2;
        pLast = &pData0[nField - 1];

        pOut = &aMem[pOp->p3];

        if (zAffinity) {
          pRec = pData0;
          do {
            applyAffinity(pRec, zAffinity[0], encoding);
            if (zAffinity[0] == 0x45 && (pRec->flags & 0x0004)) {
              pRec->flags |= 0x0020;
              pRec->flags &= ~(0x0004);
            };
            zAffinity++;
            pRec++;

          } while (zAffinity[0]);
        }

        pRec = pLast;
        do {
          if (pRec->flags & 0x0001) {
            if (pRec->flags & 0x0400) {
              pRec->uTemp = 10;
            } else {
              pRec->uTemp = 0;
            }
            nHdr++;
          } else if (pRec->flags & (0x0004 | 0x0020)) {
            i64 i = pRec->u.i;
            u64 uu;
            if (i < 0) {
              uu = ~i;
            } else {
              uu = i;
            }
            nHdr++;
            if (uu <= 127) {
              if ((i & 1) == i && p->minWriteFileFormat >= 4) {
                pRec->uTemp = 8 + (u32)uu;
              } else {
                nData++;
                pRec->uTemp = 1;
              }
            } else if (uu <= 32767) {
              nData += 2;
              pRec->uTemp = 2;
            } else if (uu <= 8388607) {
              nData += 3;
              pRec->uTemp = 3;
            } else if (uu <= 2147483647) {
              nData += 4;
              pRec->uTemp = 4;
            } else if (uu <= 140737488355327LL) {
              nData += 6;
              pRec->uTemp = 5;
            } else {
              nData += 8;
              if (pRec->flags & 0x0020) {
                pRec->u.r = (double)pRec->u.i;
                pRec->flags &= ~0x0020;
                pRec->flags |= 0x0008;
                pRec->uTemp = 7;
              } else {
                pRec->uTemp = 6;
              }
            }
          } else if (pRec->flags & 0x0008) {
            nHdr++;
            nData += 8;
            pRec->uTemp = 7;
          } else {
            len = (u32)pRec->n;
            serial_type = (len * 2) + 12 + ((pRec->flags & 0x0002) != 0);
            if (pRec->flags & 0x0400) {
              serial_type += (u32)pRec->u.nZero * 2;
              if (nData) {
                if (sqlite3VdbeMemExpandBlob(pRec))
                  goto no_mem;
                len += pRec->u.nZero;
              } else {
                nZero += pRec->u.nZero;
              }
            }
            nData += len;
            nHdr += sqlite3VarintLen(serial_type);
            pRec->uTemp = serial_type;
          }
          if (pRec == pData0)
            break;
          pRec--;
        } while (1);

        if (nHdr <= 126) {
          nHdr += 1;
        } else {
          nVarint = sqlite3VarintLen(nHdr);
          nHdr += nVarint;
          if (nVarint < sqlite3VarintLen(nHdr))
            nHdr++;
        }
        nByte = nHdr + nData;

        if (nByte + nZero <= pOut->szMalloc) {
          pOut->z = pOut->zMalloc;
        } else {
          if (nByte + nZero > db->aLimit[SQLITE_LIMIT_LENGTH]) {
            goto too_big;
          }
          if (sqlite3VdbeMemClearAndResize(pOut, (int)nByte)) {
            goto no_mem;
          }
        }
        pOut->n = (int)nByte;
        pOut->flags = 0x0010;
        if (nZero) {
          pOut->u.nZero = nZero;
          pOut->flags |= 0x0400;
        };
        zHdr = (u8 *)pOut->z;
        zPayload = zHdr + nHdr;

        if (nHdr < 0x80) {
          *(zHdr++) = nHdr;
        } else {
          zHdr += sqlite3PutVarint(zHdr, nHdr);
        }

        pRec = pData0;
        while (1) {
          serial_type = pRec->uTemp;

          if (serial_type <= 7) {
            *(zHdr++) = serial_type;
            if (serial_type == 0) {
            } else {
              u64 v;
              if (serial_type == 7) {
                memcpy(&v, &pRec->u.r, sizeof(v));
              } else {
                v = pRec->u.i;
              }
              len = sqlite3SmallTypeSizes[serial_type];

              switch (len) {
                default:
                  zPayload[7] = (u8)(v & 0xff);
                  v >>= 8;
                  zPayload[6] = (u8)(v & 0xff);
                  v >>= 8;
                  __attribute__((fallthrough));
                case 6:
                  zPayload[5] = (u8)(v & 0xff);
                  v >>= 8;
                  zPayload[4] = (u8)(v & 0xff);
                  v >>= 8;
                  __attribute__((fallthrough));
                case 4:
                  zPayload[3] = (u8)(v & 0xff);
                  v >>= 8;
                  __attribute__((fallthrough));
                case 3:
                  zPayload[2] = (u8)(v & 0xff);
                  v >>= 8;
                  __attribute__((fallthrough));
                case 2:
                  zPayload[1] = (u8)(v & 0xff);
                  v >>= 8;
                  __attribute__((fallthrough));
                case 1:
                  zPayload[0] = (u8)(v & 0xff);
              }
              zPayload += len;
            }
          } else if (serial_type < 0x80) {
            *(zHdr++) = serial_type;
            if (serial_type >= 14 && pRec->n > 0) {
              memcpy(zPayload, pRec->z, pRec->n);
              zPayload += pRec->n;
            }
          } else {
            zHdr += sqlite3PutVarint(zHdr, serial_type);
            if (pRec->n) {
              memcpy(zPayload, pRec->z, pRec->n);
              zPayload += pRec->n;
            }
          }
          if (pRec == pLast)
            break;
          pRec++;
        }

        break;
      }

      case 100: {
        i64 nEntry;
        BtCursor *pCrsr;

        pCrsr = p->apCsr[pOp->p1]->uc.pCursor;

        if (pOp->p3) {
          nEntry = sqlite3BtreeRowCountEst(pCrsr);
        } else {
          nEntry = 0;
          rc = sqlite3BtreeCount(db, pCrsr, &nEntry);
          if (rc)
            goto abort_due_to_error;
        }
        pOut = out2Prerelease(p, pOp);
        pOut->u.i = nEntry;
        goto check_for_interrupt;
      }

      case 0: {
        int p1;
        char *zName;
        int nName;
        Savepoint *pNew;
        Savepoint *pSavepoint;
        Savepoint *pTmp;
        int iSavepoint;
        int ii;

        p1 = pOp->p1;
        zName = pOp->p4.z;

        if (p1 == 0) {
          if (db->nVdbeWrite > 0) {
            sqlite3VdbeError(p, "cannot open savepoint - SQL statements in progress");
            rc = SQLITE_BUSY;
          } else {
            nName = sqlite3Strlen30(zName);

            rc = sqlite3VtabSavepoint(db, 0, db->nStatement + db->nSavepoint);
            if (rc != SQLITE_OK)
              goto abort_due_to_error;

            pNew = sqlite3DbMallocRawNN(db, sizeof(Savepoint) + nName + 1);
            if (pNew) {
              pNew->zName = (char *)&pNew[1];
              memcpy(pNew->zName, zName, nName + 1);

              if (db->autoCommit) {
                db->autoCommit = 0;
                db->isTransactionSavepoint = 1;
              } else {
                db->nSavepoint++;
              }

              pNew->pNext = db->pSavepoint;
              db->pSavepoint = pNew;
              pNew->nDeferredCons = db->nDeferredCons;
              pNew->nDeferredImmCons = db->nDeferredImmCons;
            }
          }
        } else {
          iSavepoint = 0;

          for (pSavepoint = db->pSavepoint; pSavepoint && sqlite3StrICmp(pSavepoint->zName, zName);
               pSavepoint = pSavepoint->pNext) {
            iSavepoint++;
          }
          if (!pSavepoint) {
            sqlite3VdbeError(p, "no such savepoint: %s", zName);
            rc = SQLITE_ERROR;
          } else if (db->nVdbeWrite > 0 && p1 == 1) {
            sqlite3VdbeError(p,
                             "cannot release savepoint - "
                             "SQL statements in progress");
            rc = SQLITE_BUSY;
          } else {
            int isTransaction = pSavepoint->pNext == 0 && db->isTransactionSavepoint;
            if (isTransaction && p1 == 1) {
              if ((rc = sqlite3VdbeCheckFkDeferred(p)) != SQLITE_OK) {
                goto vdbe_return;
              }
              db->autoCommit = 1;
              if (sqlite3VdbeHalt(p) == SQLITE_BUSY) {
                p->pc = (int)(pOp - aOp);
                db->autoCommit = 0;
                p->rc = rc = SQLITE_BUSY;
                goto vdbe_return;
              }
              rc = p->rc;
              if (rc) {
                db->autoCommit = 0;
              } else {
                db->isTransactionSavepoint = 0;
              }
            } else {
              int isSchemaChange;
              iSavepoint = db->nSavepoint - iSavepoint - 1;
              if (p1 == 2) {
                isSchemaChange = (db->mDbFlags & 0x0001) != 0;
                for (ii = 0; ii < db->nDb; ii++) {
                  rc = sqlite3BtreeTripAllCursors(db->aDb[ii].pBt, (4 | (2 << 8)), isSchemaChange == 0);
                  if (rc != SQLITE_OK)
                    goto abort_due_to_error;
                }
              } else {
                isSchemaChange = 0;
              }
              for (ii = 0; ii < db->nDb; ii++) {
                rc = sqlite3BtreeSavepoint(db->aDb[ii].pBt, p1, iSavepoint);
                if (rc != SQLITE_OK) {
                  goto abort_due_to_error;
                }
              }
              if (isSchemaChange) {
                sqlite3ExpirePreparedStatements(db, 0);
                sqlite3ResetAllSchemasOfConnection(db);
                db->mDbFlags |= 0x0001;
              }
            }
            if (rc)
              goto abort_due_to_error;

            while (db->pSavepoint != pSavepoint) {
              pTmp = db->pSavepoint;
              db->pSavepoint = pTmp->pNext;
              sqlite3DbFree(db, pTmp);
              db->nSavepoint--;
            }

            if (p1 == 1) {
              db->pSavepoint = pSavepoint->pNext;
              sqlite3DbFree(db, pSavepoint);
              if (!isTransaction) {
                db->nSavepoint--;
              }
            } else {
              db->nDeferredCons = pSavepoint->nDeferredCons;
              db->nDeferredImmCons = pSavepoint->nDeferredImmCons;
            }

            if (!isTransaction || p1 == 2) {
              rc = sqlite3VtabSavepoint(db, p1, iSavepoint);
              if (rc != SQLITE_OK)
                goto abort_due_to_error;
            }
          }
        }
        if (rc)
          goto abort_due_to_error;
        if (p->eVdbeState == 3) {
          rc = SQLITE_DONE;
          goto vdbe_return;
        }
        break;
      }

      case 1: {
        int desiredAutoCommit;
        int iRollback;

        desiredAutoCommit = pOp->p1;
        iRollback = pOp->p2;

        if (desiredAutoCommit != db->autoCommit) {
          if (iRollback) {
            sqlite3RollbackAll(db, (4 | (2 << 8)));
            db->autoCommit = 1;
          } else if (desiredAutoCommit && db->nVdbeWrite > 0) {
            sqlite3VdbeError(p,
                             "cannot commit transaction - "
                             "SQL statements in progress");
            rc = SQLITE_BUSY;
            goto abort_due_to_error;
          } else if ((rc = sqlite3VdbeCheckFkDeferred(p)) != SQLITE_OK) {
            goto vdbe_return;
          } else {
            db->autoCommit = (u8)desiredAutoCommit;
          }
          if (sqlite3VdbeHalt(p) == SQLITE_BUSY) {
            p->pc = (int)(pOp - aOp);
            db->autoCommit = (u8)(1 - desiredAutoCommit);
            p->rc = rc = SQLITE_BUSY;
            goto vdbe_return;
          }
          sqlite3CloseSavepoints(db);
          if (p->rc == SQLITE_OK) {
            rc = SQLITE_DONE;
          } else {
            rc = SQLITE_ERROR;
          }
          goto vdbe_return;
        } else {
          sqlite3VdbeError(p, (!desiredAutoCommit) ? "cannot start a transaction within a transaction"
                                                   : ((iRollback) ? "cannot rollback - no transaction is active"
                                                                  : "cannot commit - no transaction is active"));

          rc = SQLITE_ERROR;
          goto abort_due_to_error;
        }
      }

      case 2: {
        Btree *pBt;
        Db *pDb;
        int iMeta = 0;

        if (pOp->p2 && (db->flags & (0x00100000 | ((u64)(0x00002) << 32))) != 0) {
          if (db->flags & 0x00100000) {
            rc = SQLITE_READONLY;
          } else {
            rc = SQLITE_CORRUPT;
          }
          goto abort_due_to_error;
        }
        pDb = &db->aDb[pOp->p1];
        pBt = pDb->pBt;

        if (pBt) {
          rc = sqlite3BtreeBeginTrans(pBt, pOp->p2, &iMeta);
          if (rc != SQLITE_OK) {
            if ((rc & 0xff) == SQLITE_BUSY) {
              p->pc = (int)(pOp - aOp);
              p->rc = rc;
              goto vdbe_return;
            }
            goto abort_due_to_error;
          }

          if (p->usesStmtJournal && pOp->p2 && (db->autoCommit == 0 || db->nVdbeRead > 1)) {
            if (p->iStatement == 0) {
              db->nStatement++;
              p->iStatement = db->nSavepoint + db->nStatement;
            }

            rc = sqlite3VtabSavepoint(db, 0, p->iStatement - 1);
            if (rc == SQLITE_OK) {
              rc = sqlite3BtreeBeginStmt(pBt, p->iStatement);
            }

            p->nStmtDefCons = db->nDeferredCons;
            p->nStmtDefImmCons = db->nDeferredImmCons;
          }
        }

        if (rc == SQLITE_OK && pOp->p5 && (iMeta != pOp->p3 || pDb->pSchema->iGeneration != pOp->p4.i)) {
          sqlite3DbFree(db, p->zErrMsg);
          p->zErrMsg = sqlite3DbStrDup(db, "database schema has changed");

          if (db->aDb[pOp->p1].pSchema->schema_cookie != iMeta) {
            sqlite3ResetOneSchema(db, pOp->p1);
          }
          p->expired = 1;
          rc = SQLITE_SCHEMA;

          p->changeCntOn = 0;
        }
        if (rc)
          goto abort_due_to_error;
        break;
      }

      case 101: {
        int iMeta;
        int iDb;
        int iCookie;

        iDb = pOp->p1;
        iCookie = pOp->p3;

        sqlite3BtreeGetMeta(db->aDb[iDb].pBt, iCookie, (u32 *)&iMeta);
        pOut = out2Prerelease(p, pOp);
        pOut->u.i = iMeta;
        break;
      }

      case 102: {
        Db *pDb;

        pDb = &db->aDb[pOp->p1];

        rc = sqlite3BtreeUpdateMeta(pDb->pBt, pOp->p2, pOp->p3);
        if (pOp->p2 == 1) {
          *(u32 *)&pDb->pSchema->schema_cookie = *(u32 *)&pOp->p3 - pOp->p5;
          db->mDbFlags |= 0x0001;
          sqlite3FkClearTriggerCache(db, pOp->p1);
        } else if (pOp->p2 == 2) {
          pDb->pSchema->file_format = pOp->p3;
        }
        if (pOp->p1 == 1) {
          sqlite3ExpirePreparedStatements(db, 0);
          p->expired = 0;
        }
        if (rc)
          goto abort_due_to_error;
        break;
      }

      case 113: {
        int nField;
        KeyInfo *pKeyInfo;
        u32 p2;
        int iDb;
        int wrFlag;
        Btree *pX;
        VdbeCursor *pCur;
        Db *pDb;

        pCur = p->apCsr[pOp->p1];
        if (pCur && pCur->pgnoRoot == (u32)pOp->p2) {
          sqlite3BtreeClearCursor(pCur->uc.pCursor);
          goto open_cursor_set_hints;
        }

        case 114:
        case 116:
          if (p->expired == 1) {
            rc = (4 | (2 << 8));
            goto abort_due_to_error;
          }

          nField = 0;
          pKeyInfo = 0;
          p2 = (u32)pOp->p2;
          iDb = pOp->p3;

          pDb = &db->aDb[iDb];
          pX = pDb->pBt;

          if (pOp->opcode == 116) {
            wrFlag = 0x00000004 | (pOp->p5 & 0x08);

            if (pDb->pSchema->file_format < p->minWriteFileFormat) {
              p->minWriteFileFormat = pDb->pSchema->file_format;
            }
            if (pOp->p5 & 0x10) {
              pIn2 = &aMem[p2];

              sqlite3VdbeMemIntegerify(pIn2);
              p2 = (int)pIn2->u.i;
            }
          } else {
            wrFlag = 0;
          }
          if (pOp->p4type == (-9)) {
            pKeyInfo = pOp->p4.pKeyInfo;

            nField = pKeyInfo->nAllField;
          } else if (pOp->p4type == (-3)) {
            nField = pOp->p4.i;
          }

          pCur = allocateCursor(p, pOp->p1, nField, 0);
          if (pCur == 0)
            goto no_mem;
          pCur->iDb = iDb;
          pCur->nullRow = 1;
          pCur->isOrdered = 1;
          pCur->pgnoRoot = p2;

          rc = sqlite3BtreeCursor(pX, p2, wrFlag, pKeyInfo, pCur->uc.pCursor);
          pCur->pKeyInfo = pKeyInfo;

          pCur->isTable = pOp->p4type != (-9);

        open_cursor_set_hints:;
          sqlite3BtreeCursorHintFlags(pCur->uc.pCursor, (pOp->p5 & (0x01 | 0x02)));
          if (rc)
            goto abort_due_to_error;
          break;
      }

      case 117: {
        VdbeCursor *pOrig;
        VdbeCursor *pCx;

        pOrig = p->apCsr[pOp->p2];

        pCx = allocateCursor(p, pOp->p1, pOrig->nField, 0);
        if (pCx == 0)
          goto no_mem;
        pCx->nullRow = 1;
        pCx->isEphemeral = 1;
        pCx->pKeyInfo = pOrig->pKeyInfo;
        pCx->isTable = pOrig->isTable;
        pCx->pgnoRoot = pOrig->pgnoRoot;
        pCx->isOrdered = pOrig->isOrdered;
        pCx->ub.pBtx = pOrig->ub.pBtx;
        pCx->noReuse = 1;
        pOrig->noReuse = 1;
        rc = sqlite3BtreeCursor(pCx->ub.pBtx, pCx->pgnoRoot, 0x00000004, pCx->pKeyInfo, pCx->uc.pCursor);

        break;
      }

      case 119:
      case 120: {
        VdbeCursor *pCx;
        KeyInfo *pKeyInfo;

        static const int vfsFlags =
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXCLUSIVE | SQLITE_OPEN_DELETEONCLOSE | 0x00000400;

        if (pOp->p3 > 0) {
          aMem[pOp->p3].n = 0;
          aMem[pOp->p3].z = "";
        }
        pCx = p->apCsr[pOp->p1];
        if (pCx && !pCx->noReuse && (pOp->p2 <= pCx->nField)) {
          pCx->seqCount = 0;
          pCx->cacheStatus = 0;
          rc = sqlite3BtreeClearTable(pCx->ub.pBtx, pCx->pgnoRoot, 0);
        } else {
          pCx = allocateCursor(p, pOp->p1, pOp->p2, 0);
          if (pCx == 0)
            goto no_mem;
          pCx->isEphemeral = 1;
          rc = sqlite3BtreeOpen(db->pVfs, 0, db, &pCx->ub.pBtx, 1 | 4 | pOp->p5, vfsFlags);
          if (rc == SQLITE_OK) {
            rc = sqlite3BtreeBeginTrans(pCx->ub.pBtx, 1, 0);
            if (rc == SQLITE_OK) {
              if ((pCx->pKeyInfo = pKeyInfo = pOp->p4.pKeyInfo) != 0) {
                rc = sqlite3BtreeCreateTable(pCx->ub.pBtx, &pCx->pgnoRoot, 2 | pOp->p5);
                if (rc == SQLITE_OK) {
                  rc = sqlite3BtreeCursor(pCx->ub.pBtx, pCx->pgnoRoot, 0x00000004, pKeyInfo, pCx->uc.pCursor);
                }
                pCx->isTable = 0;
              } else {
                pCx->pgnoRoot = 1;
                rc = sqlite3BtreeCursor(pCx->ub.pBtx, 1, 0x00000004, 0, pCx->uc.pCursor);
                pCx->isTable = 1;
              }
            }
            pCx->isOrdered = (pOp->p5 != 8);

            if (rc) {
              sqlite3BtreeClose(pCx->ub.pBtx);
              p->apCsr[pOp->p1] = 0;
            } else {
            }
          }
        }
        if (rc)
          goto abort_due_to_error;
        pCx->nullRow = 1;
        break;
      }

      case 121: {
        VdbeCursor *pCx;

        pCx = allocateCursor(p, pOp->p1, pOp->p2, 1);
        if (pCx == 0)
          goto no_mem;
        pCx->pKeyInfo = pOp->p4.pKeyInfo;

        rc = sqlite3VdbeSorterInit(db, pOp->p3, pCx);
        if (rc)
          goto abort_due_to_error;
        break;
      }

      case 122: {
        VdbeCursor *pC;

        pC = p->apCsr[pOp->p1];

        if ((pC->seqCount++) == 0) {
          goto jump_to_p2;
        }
        break;
      }

      case 123: {
        VdbeCursor *pCx;

        pCx = allocateCursor(p, pOp->p1, pOp->p3, 3);
        if (pCx == 0)
          goto no_mem;
        pCx->nullRow = 1;
        pCx->seekResult = pOp->p2;
        pCx->isTable = 1;

        pCx->uc.pCursor = sqlite3BtreeFakeValidCursor();

        break;
      }

      case 124: {
        sqlite3VdbeFreeCursor(p, p->apCsr[pOp->p1]);
        p->apCsr[pOp->p1] = 0;
        break;
      }

      case 21:
      case 22:
      case 23:
      case 24: {
        int res;
        int oc;
        VdbeCursor *pC;
        UnpackedRecord r;
        int nField;
        i64 iKey;
        int eqOnly;

        pC = p->apCsr[pOp->p1];

        oc = pOp->opcode;
        eqOnly = 0;
        pC->nullRow = 0;

        pC->deferredMoveto = 0;
        pC->cacheStatus = 0;
        if (pC->isTable) {
          u16 flags3, newType;

          pIn3 = &aMem[pOp->p3];
          flags3 = pIn3->flags;
          if ((flags3 & (0x0004 | 0x0008 | 0x0020 | 0x0002)) == 0x0002) {
            applyNumericAffinity(pIn3, 0);
          }
          iKey = sqlite3VdbeIntValue(pIn3);
          newType = pIn3->flags;
          pIn3->flags = flags3;

          if ((newType & (0x0004 | 0x0020)) == 0) {
            int c;
            if ((newType & 0x0008) == 0) {
              if ((newType & 0x0001) || oc >= 23) {
                goto jump_to_p2;
              } else {
                rc = sqlite3BtreeLast(pC->uc.pCursor, &res);
                if (rc != SQLITE_OK)
                  goto abort_due_to_error;
                goto seek_not_found;
              }
            }
            c = sqlite3IntFloatCompare(iKey, pIn3->u.r);

            if (c > 0) {
              if ((oc & 0x0001) == (24 & 0x0001))
                oc--;
            }

            else if (c < 0) {
              if ((oc & 0x0001) == (21 & 0x0001))
                oc++;
            }
          }
          rc = sqlite3BtreeTableMoveto(pC->uc.pCursor, (u64)iKey, 0, &res);
          pC->movetoTarget = iKey;
          if (rc != SQLITE_OK) {
            goto abort_due_to_error;
          }
        } else {
          if (sqlite3BtreeCursorHasHint(pC->uc.pCursor, 0x00000002)) {
            eqOnly = 1;
          }

          nField = pOp->p4.i;

          r.pKeyInfo = pC->pKeyInfo;
          r.nField = (u16)nField;

          r.default_rc = ((1 & (oc - 21)) ? -1 : +1);

          r.aMem = &aMem[pOp->p3];

          r.eqSeen = 0;
          rc = sqlite3BtreeIndexMoveto(pC->uc.pCursor, &r, &res);
          if (rc != SQLITE_OK) {
            goto abort_due_to_error;
          }
          if (eqOnly && r.eqSeen == 0) {
            goto seek_not_found;
          }
        }

        if (oc >= 23) {
          if (res < 0 || (res == 0 && oc == 24)) {
            res = 0;
            rc = sqlite3BtreeNext(pC->uc.pCursor, 0);
            if (rc != SQLITE_OK) {
              if (rc == SQLITE_DONE) {
                rc = SQLITE_OK;
                res = 1;
              } else {
                goto abort_due_to_error;
              }
            }
          } else {
            res = 0;
          }
        } else {
          if (res > 0 || (res == 0 && oc == 21)) {
            res = 0;
            rc = sqlite3BtreePrevious(pC->uc.pCursor, 0);
            if (rc != SQLITE_OK) {
              if (rc == SQLITE_DONE) {
                rc = SQLITE_OK;
                res = 1;
              } else {
                goto abort_due_to_error;
              }
            }
          } else {
            res = sqlite3BtreeEof(pC->uc.pCursor);
          }
        }
      seek_not_found:;
        if (res) {
          goto jump_to_p2;
        } else if (eqOnly) {
          pOp++;
        }
        break;
      }

      case 126: {
        VdbeCursor *pC;
        int res;
        int nStep;
        UnpackedRecord r;

        pC = p->apCsr[pOp[1].p1];

        if (!sqlite3BtreeCursorIsValidNN(pC->uc.pCursor)) {
          break;
        }
        nStep = pOp->p1;

        r.pKeyInfo = pC->pKeyInfo;
        r.nField = (u16)pOp[1].p4.i;
        r.default_rc = 0;
        r.aMem = &aMem[pOp[1].p3];

        res = 0;
        while (1) {
          rc = sqlite3VdbeIdxKeyCompare(db, pC, &r, &res);
          if (rc)
            goto abort_due_to_error;
          if (res > 0 && pOp->p5 == 0) {
          seekscan_search_fail:;
            pOp++;
            goto jump_to_p2;
          }
          if (res >= 0) {
            goto jump_to_p2;
            break;
          }
          if (nStep <= 0) {
            break;
          }
          nStep--;
          pC->cacheStatus = 0;
          rc = sqlite3BtreeNext(pC->uc.pCursor, 0);
          if (rc) {
            if (rc == SQLITE_DONE) {
              rc = SQLITE_OK;
              goto seekscan_search_fail;
            } else {
              goto abort_due_to_error;
            }
          }
        }

        break;
      }

      case 127: {
        VdbeCursor *pC;

        pC = p->apCsr[pOp->p1];

        if (pC->seekHit < pOp->p2) {
          pC->seekHit = pOp->p2;
        } else if (pC->seekHit > pOp->p3) {
          pC->seekHit = pOp->p3;
        }
        break;
      }

      case 25: {
        VdbeCursor *pCur;

        pCur = p->apCsr[pOp->p1];
        if (pCur == 0 || pCur->nullRow) {
          goto jump_to_p2_and_check_for_interrupt;
        }
        break;
      }

      case 26: {
        VdbeCursor *pC;

        pC = p->apCsr[pOp->p1];

        if (pC->seekHit >= pOp->p4.i)
          break;

        __attribute__((fallthrough));
      }
      case 27:
      case 28:
      case 29: {
        int alreadyExists;
        int ii;
        VdbeCursor *pC;
        UnpackedRecord *pIdxKey;
        UnpackedRecord r;

        pC = p->apCsr[pOp->p1];

        r.aMem = &aMem[pOp->p3];

        r.nField = (u16)pOp->p4.i;
        if (r.nField > 0) {
          r.pKeyInfo = pC->pKeyInfo;
          r.default_rc = 0;

          rc = sqlite3BtreeIndexMoveto(pC->uc.pCursor, &r, &pC->seekResult);
        } else {
          rc = (((r.aMem)->flags & 0x0400) ? sqlite3VdbeMemExpandBlob(r.aMem) : 0);

          if (rc)
            goto no_mem;
          pIdxKey = sqlite3VdbeAllocUnpackedRecord(pC->pKeyInfo);
          if (pIdxKey == 0)
            goto no_mem;
          sqlite3VdbeRecordUnpack(r.aMem->n, r.aMem->z, pIdxKey);
          pIdxKey->default_rc = 0;
          rc = sqlite3BtreeIndexMoveto(pC->uc.pCursor, pIdxKey, &pC->seekResult);
          sqlite3DbFreeNN(db, pIdxKey);
        }
        if (rc != SQLITE_OK) {
          goto abort_due_to_error;
        }
        alreadyExists = (pC->seekResult == 0);
        pC->nullRow = 1 - alreadyExists;
        pC->deferredMoveto = 0;
        pC->cacheStatus = 0;
        if (pOp->opcode == 29) {
          if (alreadyExists)
            goto jump_to_p2;
        } else {
          if (!alreadyExists) {
            goto jump_to_p2;
          }
          if (pOp->opcode == 27) {
            for (ii = 0; ii < r.nField; ii++) {
              if (r.aMem[ii].flags & 0x0001) {
                goto jump_to_p2;
              }
            }
          };
          if (pOp->opcode == 26) {
            pC->seekHit = pOp->p4.i;
          }
        }
        break;
      }

      case 30: {
        VdbeCursor *pC;
        BtCursor *pCrsr;
        int res;
        u64 iKey;

        pIn3 = &aMem[pOp->p3];
        if ((pIn3->flags & (0x0004 | 0x0020)) == 0) {
          Mem x = pIn3[0];
          applyAffinity(&x, 0x43, encoding);
          if ((x.flags & 0x0004) == 0)
            goto jump_to_p2;
          iKey = x.u.i;
          goto notExistsWithKey;
        }

        __attribute__((fallthrough));
        case 31:
          pIn3 = &aMem[pOp->p3];

          iKey = pIn3->u.i;
        notExistsWithKey:
          pC = p->apCsr[pOp->p1];

          pCrsr = pC->uc.pCursor;

          res = 0;
          rc = sqlite3BtreeTableMoveto(pCrsr, iKey, 0, &res);

          pC->movetoTarget = iKey;
          pC->nullRow = 0;
          pC->cacheStatus = 0;
          pC->deferredMoveto = 0;
          pC->seekResult = res;
          if (res != 0) {
            if (pOp->p2 == 0) {
              rc = sqlite3CorruptError(102153);
            } else {
              goto jump_to_p2;
            }
          }
          if (rc)
            goto abort_due_to_error;
          break;
      }

      case 128: {
        pOut = out2Prerelease(p, pOp);
        pOut->u.i = p->apCsr[pOp->p1]->seqCount++;
        break;
      }

      case 129: {
        i64 v;
        VdbeCursor *pC;
        int res;
        int cnt;

        Mem *pMem;
        VdbeFrame *pFrame;

        v = 0;
        res = 0;
        pOut = out2Prerelease(p, pOp);

        pC = p->apCsr[pOp->p1];

        {
          if (!pC->useRandomRowid) {
            rc = sqlite3BtreeLast(pC->uc.pCursor, &res);
            if (rc != SQLITE_OK) {
              goto abort_due_to_error;
            }
            if (res) {
              v = 1;
            } else {
              v = sqlite3BtreeIntegerKey(pC->uc.pCursor);
              if (v >= (i64)((((u64)0x7fffffff) << 32) | (u64)0xffffffff)) {
                pC->useRandomRowid = 1;
              } else {
                v++;
              }
            }
          }

          if (pOp->p3) {
            if (p->pFrame) {
              for (pFrame = p->pFrame; pFrame->pParent; pFrame = pFrame->pParent)
                ;

              pMem = &pFrame->aMem[pOp->p3];
            } else {
              pMem = &aMem[pOp->p3];
            }

            sqlite3VdbeMemIntegerify(pMem);

            if (pMem->u.i == (i64)((((u64)0x7fffffff) << 32) | (u64)0xffffffff) || pC->useRandomRowid) {
              rc = SQLITE_FULL;
              goto abort_due_to_error;
            }
            if (v < pMem->u.i + 1) {
              v = pMem->u.i + 1;
            }
            pMem->u.i = v;
          }

          if (pC->useRandomRowid) {
            cnt = 0;
            do {
              sqlite3_randomness(sizeof(v), &v);
              v &= ((i64)((((u64)0x7fffffff) << 32) | (u64)0xffffffff) >> 1);
              v++;
            } while (((rc = sqlite3BtreeTableMoveto(pC->uc.pCursor, (u64)v, 0, &res)) == SQLITE_OK) && (res == 0) &&
                     (++cnt < 100));
            if (rc)
              goto abort_due_to_error;
            if (res == 0) {
              rc = SQLITE_FULL;
              goto abort_due_to_error;
            }
          }
          pC->deferredMoveto = 0;
          pC->cacheStatus = 0;
        }
        pOut->u.i = v;
        break;
      }

      case 130: {
        Mem *pData;
        Mem *pKey;
        VdbeCursor *pC;
        int seekResult;
        const char *zDb;
        Table *pTab;
        BtreePayload x;

        pData = &aMem[pOp->p2];

        pC = p->apCsr[pOp->p1];

        pKey = &aMem[pOp->p3];

        x.nKey = pKey->u.i;

        if (pOp->p4type == (-5) && ((db)->xUpdateCallback)) {
          zDb = db->aDb[(int)(pC->iDb)].zDbSName;
          pTab = pOp->p4.pTab;

        } else {
          pTab = 0;
          zDb = 0;
        }

        if (pOp->p5 & 0x01) {
          p->nChange++;
          if (pOp->p5 & 0x20)
            db->lastRowid = x.nKey;
        }

        x.pData = pData->z;
        x.nData = pData->n;
        seekResult = ((pOp->p5 & 0x10) ? pC->seekResult : 0);
        if (pData->flags & 0x0400) {
          x.nZero = pData->u.nZero;
        } else {
          x.nZero = 0;
        }
        x.pKey = 0;

        rc = sqlite3BtreeInsert(pC->uc.pCursor, &x, (pOp->p5 & (0x08 | 0x02 | 0x80)), seekResult);
        pC->deferredMoveto = 0;
        pC->cacheStatus = 0;
        colCacheCtr++;

        if (rc)
          goto abort_due_to_error;
        if (pTab) {
          db->xUpdateCallback(db->pUpdateArg, (pOp->p5 & 0x04) ? SQLITE_UPDATE : SQLITE_INSERT, zDb, pTab->zName,
                              x.nKey);
        }
        break;
      }

      case 131: {
        VdbeCursor *pDest;
        VdbeCursor *pSrc;
        i64 iKey;

        pDest = p->apCsr[pOp->p1];
        pSrc = p->apCsr[pOp->p2];
        iKey = pOp->p3 ? aMem[pOp->p3].u.i : 0;
        rc = sqlite3BtreeTransferRow(pDest->uc.pCursor, pSrc->uc.pCursor, iKey);
        if (rc != SQLITE_OK)
          goto abort_due_to_error;
        break;
      };

      case 132: {
        VdbeCursor *pC;
        const char *zDb;
        Table *pTab;
        int opflags;

        opflags = pOp->p2;

        pC = p->apCsr[pOp->p1];

        if (pOp->p4type == (-5) && ((db)->xUpdateCallback)) {
          zDb = db->aDb[(int)(pC->iDb)].zDbSName;
          pTab = pOp->p4.pTab;
          if ((pOp->p5 & 0x02) != 0 && pC->isTable) {
            pC->movetoTarget = sqlite3BtreeIntegerKey(pC->uc.pCursor);
          }
        } else {
          zDb = 0;
          pTab = 0;
        }

        rc = sqlite3BtreeDelete(pC->uc.pCursor, pOp->p5);
        pC->cacheStatus = 0;
        colCacheCtr++;
        pC->seekResult = 0;
        if (rc)
          goto abort_due_to_error;

        if (opflags & 0x01) {
          p->nChange++;
          if (db->xUpdateCallback && (pTab != 0) && (((pTab)->tabFlags & 0x00000080) == 0)) {
            db->xUpdateCallback(db->pUpdateArg, SQLITE_DELETE, zDb, pTab->zName, pC->movetoTarget);
          }
        }

        break;
      }

      case 133: {
        sqlite3VdbeSetChanges(db, p->nChange);
        p->nChange = 0;
        break;
      }

      case 134: {
        VdbeCursor *pC;
        int res;
        int nKeyCol;

        pC = p->apCsr[pOp->p1];

        pIn3 = &aMem[pOp->p3];
        nKeyCol = pOp->p4.i;
        res = 0;
        rc = sqlite3VdbeSorterCompare(pC, pIn3, nKeyCol, &res);
        if (rc)
          goto abort_due_to_error;
        if (res)
          goto jump_to_p2;
        break;
      };

      case 135: {
        VdbeCursor *pC;

        pOut = &aMem[pOp->p2];
        pC = p->apCsr[pOp->p1];

        rc = sqlite3VdbeSorterRowkey(pC, pOut);

        if (rc)
          goto abort_due_to_error;
        p->apCsr[pOp->p3]->cacheStatus = 0;
        break;
      }

      case 136: {
        VdbeCursor *pC;
        BtCursor *pCrsr;
        u32 n;

        pOut = out2Prerelease(p, pOp);

        pC = p->apCsr[pOp->p1];

        pCrsr = pC->uc.pCursor;

        n = sqlite3BtreePayloadSize(pCrsr);
        if (n > (u32)db->aLimit[SQLITE_LIMIT_LENGTH]) {
          goto too_big;
        };
        rc = sqlite3VdbeMemFromBtreeZeroOffset(pCrsr, n, pOut);
        if (rc)
          goto abort_due_to_error;
        if (!pOp->p3)
          if (((pOut)->flags & 0x4000) != 0 && sqlite3VdbeMemMakeWriteable(pOut)) {
            goto no_mem;
          };
        break;
      }

      case 137: {
        VdbeCursor *pC;
        i64 v;
        sqlite3_vtab *pVtab;
        const sqlite3_module *pModule;

        pOut = out2Prerelease(p, pOp);

        pC = p->apCsr[pOp->p1];

        if (pC->nullRow) {
          pOut->flags = 0x0001;
          break;
        } else if (pC->deferredMoveto) {
          v = pC->movetoTarget;

        } else if (pC->eCurType == 2) {
          pVtab = pC->uc.pVCur->pVtab;
          pModule = pVtab->pModule;

          rc = pModule->xRowid(pC->uc.pVCur, &v);
          sqlite3VtabImportErrmsg(p, pVtab);
          if (rc)
            goto abort_due_to_error;

        } else {
          rc = sqlite3VdbeCursorRestore(pC);
          if (rc)
            goto abort_due_to_error;
          if (pC->nullRow) {
            pOut->flags = 0x0001;
            break;
          }
          v = sqlite3BtreeIntegerKey(pC->uc.pCursor);
        }
        pOut->u.i = v;
        break;
      }

      case 138: {
        VdbeCursor *pC;

        pC = p->apCsr[pOp->p1];
        if (pC == 0) {
          pC = allocateCursor(p, pOp->p1, 1, 3);
          if (pC == 0)
            goto no_mem;
          pC->seekResult = 0;
          pC->isTable = 1;
          pC->noReuse = 1;
          pC->uc.pCursor = sqlite3BtreeFakeValidCursor();
        }
        pC->nullRow = 1;
        pC->cacheStatus = 0;
        if (pC->eCurType == 0) {
          sqlite3BtreeClearCursor(pC->uc.pCursor);
        }

        break;
      }

      case 139:
      case 32: {
        VdbeCursor *pC;
        BtCursor *pCrsr;
        int res;

        pC = p->apCsr[pOp->p1];

        pCrsr = pC->uc.pCursor;
        res = 0;

        if (pOp->opcode == 139) {
          pC->seekResult = -1;
          if (sqlite3BtreeCursorIsValidNN(pCrsr)) {
            break;
          }
        }
        rc = sqlite3BtreeLast(pCrsr, &res);
        pC->nullRow = (u8)res;
        pC->deferredMoveto = 0;
        pC->cacheStatus = 0;
        if (rc)
          goto abort_due_to_error;
        if (pOp->p2 > 0) {
          if (res)
            goto jump_to_p2;
        }
        break;
      }

      case 33: {
        VdbeCursor *pC;
        BtCursor *pCrsr;
        int res;
        i64 sz;

        pC = p->apCsr[pOp->p1];

        pCrsr = pC->uc.pCursor;

        rc = sqlite3BtreeFirst(pCrsr, &res);
        if (rc)
          goto abort_due_to_error;
        if (res != 0) {
          sz = -1;
        } else {
          sz = sqlite3BtreeRowCountEst(pCrsr);

          sz = sqlite3LogEst((u64)sz);
        }
        res = sz >= pOp->p3 && sz <= pOp->p4.i;
        if (res)
          goto jump_to_p2;
        break;
      }

      case 34:
      case 35: {
        p->aCounter[SQLITE_STMTSTATUS_SORT]++;

        __attribute__((fallthrough));
      }

      case 36: {
        VdbeCursor *pC;
        BtCursor *pCrsr;
        int res;

        pC = p->apCsr[pOp->p1];

        res = 1;

        if ((pC)->eCurType == 1) {
          rc = sqlite3VdbeSorterRewind(pC, &res);
        } else {
          pCrsr = pC->uc.pCursor;

          rc = sqlite3BtreeFirst(pCrsr, &res);
          pC->deferredMoveto = 0;
          pC->cacheStatus = 0;
        }
        if (rc)
          goto abort_due_to_error;
        pC->nullRow = (u8)res;
        if (pOp->p2 > 0) {
          if (res)
            goto jump_to_p2;
        }
        break;
      }

      case 37: {
        VdbeCursor *pC;
        BtCursor *pCrsr;
        int res;

        pC = p->apCsr[pOp->p1];

        pCrsr = pC->uc.pCursor;

        rc = sqlite3BtreeIsEmpty(pCrsr, &res);
        if (rc)
          goto abort_due_to_error;
        if (res)
          goto jump_to_p2;
        break;
      }

      case 38: {
        VdbeCursor *pC;

        pC = p->apCsr[pOp->p1];

        rc = sqlite3VdbeSorterNext(db, pC);
        goto next_tail;

        case 39:
          pC = p->apCsr[pOp->p1];

          rc = sqlite3BtreePrevious(pC->uc.pCursor, pOp->p3);
          goto next_tail;

        case 40:
          pC = p->apCsr[pOp->p1];

          rc = sqlite3BtreeNext(pC->uc.pCursor, pOp->p3);

        next_tail:
          pC->cacheStatus = 0;
          if (rc == SQLITE_OK) {
            pC->nullRow = 0;
            p->aCounter[pOp->p5]++;

            goto jump_to_p2_and_check_for_interrupt;
          }
          if (rc != SQLITE_DONE)
            goto abort_due_to_error;
          rc = SQLITE_OK;
          pC->nullRow = 1;
          goto check_for_interrupt;
      }

      case 140: {
        VdbeCursor *pC;
        BtreePayload x;

        pC = p->apCsr[pOp->p1];

        pIn2 = &aMem[pOp->p2];

        if (pOp->p5 & 0x01)
          p->nChange++;

        rc = (((pIn2)->flags & 0x0400) ? sqlite3VdbeMemExpandBlob(pIn2) : 0);
        if (rc)
          goto abort_due_to_error;
        x.nKey = pIn2->n;
        x.pKey = pIn2->z;
        x.aMem = aMem + pOp->p3;
        x.nMem = (u16)pOp->p4.i;
        rc = sqlite3BtreeInsert(pC->uc.pCursor, &x, (pOp->p5 & (0x08 | 0x02 | 0x80)),
                                ((pOp->p5 & 0x10) ? pC->seekResult : 0));

        pC->cacheStatus = 0;
        if (rc)
          goto abort_due_to_error;
        break;
      }

      case 141: {
        VdbeCursor *pC;

        pC = p->apCsr[pOp->p1];

        pIn2 = &aMem[pOp->p2];

        rc = (((pIn2)->flags & 0x0400) ? sqlite3VdbeMemExpandBlob(pIn2) : 0);
        if (rc)
          goto abort_due_to_error;
        rc = sqlite3VdbeSorterWrite(pC, pIn2);
        if (rc)
          goto abort_due_to_error;
        break;
      }

      case 142: {
        VdbeCursor *pC;
        BtCursor *pCrsr;
        int res;
        UnpackedRecord r;

        pC = p->apCsr[pOp->p1];

        pCrsr = pC->uc.pCursor;

        r.pKeyInfo = pC->pKeyInfo;
        r.nField = (u16)pOp->p3;
        r.default_rc = 0;
        r.aMem = &aMem[pOp->p2];
        rc = sqlite3BtreeIndexMoveto(pCrsr, &r, &res);
        if (rc)
          goto abort_due_to_error;
        if (res != 0) {
          rc = sqlite3VdbeFindIndexKey(pCrsr, pOp->p4.pIdx, &r, &res, 0);
          if (rc != SQLITE_OK)
            goto abort_due_to_error;
          if (res != 0) {
            if (!sqlite3WritableSchema(db)) {
              rc = sqlite3ReportError((11 | (3 << 8)), 103270, "index corruption");
              goto abort_due_to_error;
            }
            pC->cacheStatus = 0;
            pC->seekResult = 0;
            break;
          }
        }
        rc = sqlite3BtreeDelete(pCrsr, 0x04);
        if (rc)
          goto abort_due_to_error;

        pC->cacheStatus = 0;
        pC->seekResult = 0;
        break;
      }

      case 143:
      case 144: {
        VdbeCursor *pC;
        VdbeCursor *pTabCur;
        i64 rowid;

        pC = p->apCsr[pOp->p1];

        rc = sqlite3VdbeCursorRestore(pC);

        if (rc != SQLITE_OK)
          goto abort_due_to_error;

        if (!pC->nullRow) {
          rowid = 0;
          rc = sqlite3VdbeIdxRowid(db, pC->uc.pCursor, &rowid);
          if (rc != SQLITE_OK) {
            goto abort_due_to_error;
          }
          if (pOp->opcode == 143) {
            pTabCur = p->apCsr[pOp->p3];

            pTabCur->nullRow = 0;
            pTabCur->movetoTarget = rowid;
            pTabCur->deferredMoveto = 1;
            pTabCur->cacheStatus = 0;

            pTabCur->ub.aAltMap = pOp->p4.ai;

            pTabCur->pAltCursor = pC;
          } else {
            pOut = out2Prerelease(p, pOp);
            pOut->u.i = rowid;
          }
        } else {
          sqlite3VdbeMemSetNull(&aMem[pOp->p2]);
        }
        break;
      }

      case 145: {
        VdbeCursor *pC;

        pC = p->apCsr[pOp->p1];
        if (pC->deferredMoveto) {
          rc = sqlite3VdbeFinishMoveto(pC);
          if (rc)
            goto abort_due_to_error;
        }
        break;
      }

      case 41:
      case 42:
      case 45:
      case 46: {
        VdbeCursor *pC;
        int res;
        UnpackedRecord r;

        pC = p->apCsr[pOp->p1];

        r.pKeyInfo = pC->pKeyInfo;
        r.nField = (u16)pOp->p4.i;
        if (pOp->opcode < 45) {
          r.default_rc = -1;
        } else {
          r.default_rc = 0;
        }
        r.aMem = &aMem[pOp->p3];

        {
          i64 nCellKey = 0;
          BtCursor *pCur;
          Mem m;

          pCur = pC->uc.pCursor;

          nCellKey = sqlite3BtreePayloadSize(pCur);

          if (nCellKey <= 0 || nCellKey > 0x7fffffff) {
            rc = sqlite3CorruptError(103482);
            goto abort_due_to_error;
          }
          sqlite3VdbeMemInit(&m, db, 0);
          rc = sqlite3VdbeMemFromBtreeZeroOffset(pCur, (u32)nCellKey, &m);
          if (rc)
            goto abort_due_to_error;
          res = sqlite3VdbeRecordCompareWithSkip(m.n, m.z, &r, 0);
          sqlite3VdbeMemReleaseMalloc(&m);
        }

        if ((pOp->opcode & 1) == (45 & 1)) {
          res = -res;
        } else {
          res++;
        };

        if (res > 0)
          goto jump_to_p2;
        break;
      }

      case 146: {
        int iMoved;
        int iDb;

        pOut = out2Prerelease(p, pOp);
        pOut->flags = 0x0001;
        if (db->nVdbeRead > db->nVDestroy + 1) {
          rc = SQLITE_LOCKED;
          p->errorAction = 2;
          goto abort_due_to_error;
        } else {
          iDb = pOp->p3;

          iMoved = 0;
          rc = sqlite3BtreeDropTable(db->aDb[iDb].pBt, pOp->p1, &iMoved);
          pOut->flags = 0x0004;
          pOut->u.i = iMoved;
          if (rc)
            goto abort_due_to_error;

          if (iMoved != 0) {
            sqlite3RootPageMoved(db, iDb, iMoved, pOp->p1);

            resetSchemaOnFault = iDb + 1;
          }
        }
        break;
      }

      case 147: {
        i64 nChange;

        nChange = 0;

        rc = sqlite3BtreeClearTable(db->aDb[pOp->p2].pBt, (u32)pOp->p1, &nChange);
        if (pOp->p3) {
          p->nChange += nChange;
          if (pOp->p3 > 0) {
            aMem[pOp->p3].u.i += nChange;
          }
        }
        if (rc)
          goto abort_due_to_error;
        break;
      }

      case 148: {
        VdbeCursor *pC;

        pC = p->apCsr[pOp->p1];

        if ((pC)->eCurType == 1) {
          sqlite3VdbeSorterReset(db, pC->uc.pSorter);
        } else {
          rc = sqlite3BtreeClearTableOfCursor(pC->uc.pCursor);
          if (rc)
            goto abort_due_to_error;
        }
        break;
      }

      case 149: {
        Pgno pgno;
        Db *pDb;

        pOut = out2Prerelease(p, pOp);
        pgno = 0;

        pDb = &db->aDb[pOp->p1];

        rc = sqlite3BtreeCreateTable(pDb->pBt, &pgno, pOp->p3);
        if (rc)
          goto abort_due_to_error;
        pOut->u.i = pgno;
        break;
      }

      case 150: {
        char *zErr;

        sqlite3_xauth xAuth;

        u8 mTrace;
        int savedAnalysisLimit;

        db->nSqlExec++;
        zErr = 0;

        xAuth = db->xAuth;

        mTrace = db->mTrace;
        savedAnalysisLimit = db->nAnalysisLimit;
        if (pOp->p1 & 0x0001) {
          db->xAuth = 0;

          db->mTrace = 0;
        }
        if (pOp->p1 & 0x0002) {
          db->nAnalysisLimit = pOp->p2;
        }
        rc = sqlite3_exec(db, pOp->p4.z, 0, 0, &zErr);
        db->nSqlExec--;

        db->xAuth = xAuth;

        db->mTrace = mTrace;
        db->nAnalysisLimit = savedAnalysisLimit;
        if (zErr || rc) {
          sqlite3VdbeError(p, "%s", zErr);
          sqlite3_free(zErr);
          if (rc == SQLITE_NOMEM)
            goto no_mem;
          goto abort_due_to_error;
        }
        break;
      }

      case 151: {
        int iDb;
        const char *zSchema;
        char *zSql;
        InitData initData;

        iDb = pOp->p1;

        if (pOp->p4.z == 0) {
          sqlite3SchemaClear(db->aDb[iDb].pSchema);
          db->mDbFlags &= ~0x0010;
          rc = sqlite3InitOne(db, iDb, &p->zErrMsg, pOp->p5);
          db->mDbFlags |= 0x0001;
          p->expired = 0;
        } else {
          zSchema = "sqlite_master";
          initData.db = db;
          initData.iDb = iDb;
          initData.pzErrMsg = &p->zErrMsg;
          initData.mInitFlags = 0;
          initData.mxPage = sqlite3BtreeLastPage(db->aDb[iDb].pBt);
          zSql = sqlite3MPrintf(db, "SELECT*FROM\"%w\".%s WHERE %s ORDER BY rowid", db->aDb[iDb].zDbSName, zSchema,
                                pOp->p4.z);
          if (zSql == 0) {
            rc = 7;
          } else {
            db->init.busy = 1;
            initData.rc = SQLITE_OK;
            initData.nInitRow = 0;

            rc = sqlite3_exec(db, zSql, sqlite3InitCallback, &initData, 0);
            if (rc == SQLITE_OK)
              rc = initData.rc;
            if (rc == SQLITE_OK && initData.nInitRow == 0) {
              rc = sqlite3CorruptError(103775);
            }
            sqlite3DbFreeNN(db, zSql);
            db->init.busy = 0;
          }
        }
        if (rc) {
          sqlite3ResetAllSchemasOfConnection(db);
          if (rc == SQLITE_NOMEM) {
            goto no_mem;
          }
          goto abort_due_to_error;
        }
        break;
      }

      case 152: {
        rc = sqlite3AnalysisLoad(db, pOp->p1);
        if (rc)
          goto abort_due_to_error;
        break;
      }

      case 153: {
        sqlite3UnlinkAndDeleteTable(db, pOp->p1, pOp->p4.z);
        break;
      }

      case 155: {
        sqlite3UnlinkAndDeleteIndex(db, pOp->p1, pOp->p4.z);
        break;
      }

      case 156: {
        sqlite3UnlinkAndDeleteTrigger(db, pOp->p1, pOp->p4.z);
        break;
      }

      case 157: {
        int nRoot;
        Pgno *aRoot;
        int nErr;
        char *z;
        Mem *pnErr;

        nRoot = pOp->p2;
        aRoot = pOp->p4.ai;

        pnErr = &aMem[pOp->p1];

        pIn1 = &aMem[pOp->p1 + 1];

        rc = sqlite3BtreeIntegrityCheck(db, db->aDb[pOp->p5].pBt, &aRoot[1], &aMem[pOp->p3], nRoot, (int)pnErr->u.i + 1,
                                        &nErr, &z);
        sqlite3VdbeMemSetNull(pIn1);
        if (nErr == 0) {
        } else if (rc) {
          sqlite3_free(z);
          goto abort_due_to_error;
        } else {
          pnErr->u.i -= nErr - 1;
          sqlite3VdbeMemSetStr(pIn1, z, -1, SQLITE_UTF8, sqlite3_free);
        };
        sqlite3VdbeChangeEncoding(pIn1, encoding);
        goto check_for_interrupt;
      }

      case 47: {
        VdbeCursor *pC;
        int res;
        UnpackedRecord r;

        pC = p->apCsr[pOp->p1];

        memset(&r, 0, sizeof(r));
        r.aMem = &aMem[pOp->p3];
        r.nField = pOp->p4.pIdx->nColumn;
        r.pKeyInfo = pC->pKeyInfo;

        rc = sqlite3VdbeFindIndexKey(pC->uc.pCursor, pOp->p4.pIdx, &r, &res, 1);
        if (rc || res != 0) {
          rc = SQLITE_OK;
          goto jump_to_p2;
        }
        pC->nullRow = 0;
        break;
      };

      case 158: {
        pIn1 = &aMem[pOp->p1];
        pIn2 = &aMem[pOp->p2];

        if ((pIn1->flags & 0x0010) == 0) {
          if (sqlite3VdbeMemSetRowSet(pIn1))
            goto no_mem;
        }

        sqlite3RowSetInsert((RowSet *)pIn1->z, pIn2->u.i);
        break;
      }

      case 48: {
        i64 val;

        pIn1 = &aMem[pOp->p1];

        if ((pIn1->flags & 0x0010) == 0 || sqlite3RowSetNext((RowSet *)pIn1->z, &val) == 0) {
          sqlite3VdbeMemSetNull(pIn1);
          goto jump_to_p2_and_check_for_interrupt;
        } else {
          sqlite3VdbeMemSetInt64(&aMem[pOp->p3], val);
        }
        goto check_for_interrupt;
      }

      case 49: {
        int iSet;
        int exists;

        pIn1 = &aMem[pOp->p1];
        pIn3 = &aMem[pOp->p3];
        iSet = pOp->p4.i;

        if ((pIn1->flags & 0x0010) == 0) {
          if (sqlite3VdbeMemSetRowSet(pIn1))
            goto no_mem;
        }

        if (iSet) {
          exists = sqlite3RowSetTest((RowSet *)pIn1->z, iSet, pIn3->u.i);
          if (exists)
            goto jump_to_p2;
        }
        if (iSet >= 0) {
          sqlite3RowSetInsert((RowSet *)pIn1->z, pIn3->u.i);
        }
        break;
      }

      case 50: {
        int nMem;
        i64 nByte;
        Mem *pRt;
        Mem *pMem;
        Mem *pEnd;
        VdbeFrame *pFrame;
        SubProgram *pProgram;
        void *t;

        pProgram = pOp->p4.pProgram;
        pRt = &aMem[pOp->p3];

        if (pOp->p5) {
          t = pProgram->token;
          for (pFrame = p->pFrame; pFrame && pFrame->token != t; pFrame = pFrame->pParent)
            ;
          if (pFrame)
            break;
        }

        if (p->nFrame >= db->aLimit[SQLITE_LIMIT_TRIGGER_DEPTH]) {
          rc = SQLITE_ERROR;
          sqlite3VdbeError(p, "too many levels of trigger recursion");
          goto abort_due_to_error;
        }

        if ((pRt->flags & 0x0010) == 0) {
          nMem = pProgram->nMem + pProgram->nCsr;

          if (pProgram->nCsr == 0)
            nMem++;
          nByte = (((sizeof(VdbeFrame)) + 7) & ~7) + nMem * sizeof(Mem) + pProgram->nCsr * sizeof(VdbeCursor *) +
                  (7 + (i64)pProgram->nOp) / 8;
          pFrame = sqlite3DbMallocZero(db, nByte);
          if (!pFrame) {
            goto no_mem;
          }
          sqlite3VdbeMemRelease(pRt);
          pRt->flags = 0x0010 | 0x1000;
          pRt->z = (char *)pFrame;
          pRt->n = (int)nByte;
          pRt->xDel = sqlite3VdbeFrameMemDel;

          pFrame->v = p;
          pFrame->nChildMem = nMem;
          pFrame->nChildCsr = pProgram->nCsr;
          pFrame->pc = (int)(pOp - aOp);
          pFrame->aMem = p->aMem;
          pFrame->nMem = p->nMem;
          pFrame->apCsr = p->apCsr;
          pFrame->nCursor = p->nCursor;
          pFrame->aOp = p->aOp;
          pFrame->nOp = p->nOp;
          pFrame->token = pProgram->token;

          pEnd = &((Mem *)&((u8 *)pFrame)[(((sizeof(VdbeFrame)) + 7) & ~7)])[pFrame->nChildMem];
          for (pMem = ((Mem *)&((u8 *)pFrame)[(((sizeof(VdbeFrame)) + 7) & ~7)]); pMem != pEnd; pMem++) {
            pMem->flags = 0x0000;
            pMem->db = db;
          }
        } else {
          pFrame = (VdbeFrame *)pRt->z;
        }

        p->nFrame++;
        pFrame->pParent = p->pFrame;
        pFrame->lastRowid = db->lastRowid;
        pFrame->nChange = p->nChange;
        pFrame->nDbChange = p->db->nChange;

        pFrame->pAuxData = p->pAuxData;
        p->pAuxData = 0;
        p->nChange = 0;
        p->pFrame = pFrame;
        p->aMem = aMem = ((Mem *)&((u8 *)pFrame)[(((sizeof(VdbeFrame)) + 7) & ~7)]);
        p->nMem = pFrame->nChildMem;
        p->nCursor = (u16)pFrame->nChildCsr;
        p->apCsr = (VdbeCursor **)&aMem[p->nMem];
        pFrame->aOnce = (u8 *)&p->apCsr[pProgram->nCsr];
        memset(pFrame->aOnce, 0, (pProgram->nOp + 7) / 8);
        p->aOp = aOp = pProgram->aOp;
        p->nOp = pProgram->nOp;

        pOp = &aOp[-1];
        goto check_for_interrupt;
      }

      case 159: {
        VdbeFrame *pFrame;
        Mem *pIn;
        pOut = out2Prerelease(p, pOp);
        pFrame = p->pFrame;
        pIn = &pFrame->aMem[pOp->p1 + pFrame->aOp[pFrame->pc].p1];
        sqlite3VdbeMemShallowCopy(pOut, pIn, 0x4000);
        break;
      }

      case 160: {
        if (pOp->p1) {
          db->nDeferredCons += pOp->p2;
        } else {
          if (db->flags & 0x00080000) {
            db->nDeferredImmCons += pOp->p2;
          } else {
            p->nFkConstraint += pOp->p2;
          }
        }
        break;
      }

      case 60: {
        if (pOp->p1) {
          if (db->nDeferredCons == 0 && db->nDeferredImmCons == 0)
            goto jump_to_p2;
        } else {
          if (p->nFkConstraint == 0 && db->nDeferredImmCons == 0)
            goto jump_to_p2;
        }
        break;
      }

      case 161: {
        VdbeFrame *pFrame;
        if (p->pFrame) {
          for (pFrame = p->pFrame; pFrame->pParent; pFrame = pFrame->pParent)
            ;
          pIn1 = &pFrame->aMem[pOp->p1];
        } else {
          pIn1 = &aMem[pOp->p1];
        }

        sqlite3VdbeMemIntegerify(pIn1);
        pIn2 = &aMem[pOp->p2];
        sqlite3VdbeMemIntegerify(pIn2);
        if (pIn1->u.i < pIn2->u.i) {
          pIn1->u.i = pIn2->u.i;
        }
        break;
      }

      case 61: {
        pIn1 = &aMem[pOp->p1];

        if (pIn1->u.i > 0) {
          pIn1->u.i -= pOp->p3;
          goto jump_to_p2;
        }
        break;
      }

      case 162: {
        i64 x;
        pIn1 = &aMem[pOp->p1];
        pIn3 = &aMem[pOp->p3];
        pOut = out2Prerelease(p, pOp);

        x = pIn1->u.i;
        if (x <= 0 || sqlite3AddInt64(&x, pIn3->u.i > 0 ? pIn3->u.i : 0)) {
          pOut->u.i = -1;
        } else {
          pOut->u.i = x;
        }
        break;
      }

      case 62: {
        pIn1 = &aMem[pOp->p1];

        if (pIn1->u.i) {
          if (pIn1->u.i > 0)
            pIn1->u.i--;
          goto jump_to_p2;
        }
        break;
      }

      case 63: {
        pIn1 = &aMem[pOp->p1];

        if (pIn1->u.i > (((i64)-1) - (0xffffffff | (((i64)0x7fffffff) << 32))))
          pIn1->u.i--;
        if (pIn1->u.i == 0)
          goto jump_to_p2;
        break;
      }

      case 163:
      case 164: {
        int n;
        sqlite3_context *pCtx;
        u64 nAlloc;

        n = pOp->p5;

        nAlloc = ((offsetof(sqlite3_context, argv) + (n) * sizeof(sqlite3_value *)));
        pCtx = sqlite3DbMallocRawNN(db, nAlloc + sizeof(Mem));
        if (pCtx == 0)
          goto no_mem;
        pCtx->pOut = (Mem *)((u8 *)pCtx + nAlloc);

        sqlite3VdbeMemInit(pCtx->pOut, db, 0x0001);
        pCtx->pMem = 0;
        pCtx->pFunc = pOp->p4.pFunc;
        pCtx->iOp = (int)(pOp - aOp);
        pCtx->pVdbe = p;
        pCtx->skipFlag = 0;
        pCtx->isError = 0;
        pCtx->enc = encoding;
        pCtx->argc = n;
        pOp->p4type = (-16);
        pOp->p4.pCtx = pCtx;

        pOp->opcode = 165;

        __attribute__((fallthrough));
      }
      case 165: {
        int i;
        sqlite3_context *pCtx;
        Mem *pMem;

        pCtx = pOp->p4.pCtx;
        pMem = &aMem[pOp->p3];

        if (pCtx->pMem != pMem) {
          pCtx->pMem = pMem;
          for (i = pCtx->argc - 1; i >= 0; i--)
            pCtx->argv[i] = &aMem[pOp->p2 + i];
        }

        pMem->n++;

        if (pOp->p1) {
          (pCtx->pFunc->xInverse)(pCtx, pCtx->argc, pCtx->argv);
        } else
          (pCtx->pFunc->xSFunc)(pCtx, pCtx->argc, pCtx->argv);

        if (pCtx->isError) {
          if (pCtx->isError > 0) {
            sqlite3VdbeError(p, "%s", sqlite3_value_text(pCtx->pOut));
            rc = pCtx->isError;
          }
          if (pCtx->skipFlag) {
            i = pOp[-1].p1;
            if (i)
              sqlite3VdbeMemSetInt64(&aMem[i], 1);
            pCtx->skipFlag = 0;
          }
          sqlite3VdbeMemRelease(pCtx->pOut);
          pCtx->pOut->flags = 0x0001;
          pCtx->isError = 0;
          if (rc)
            goto abort_due_to_error;
        }

        break;
      }

      case 166:
      case 167: {
        Mem *pMem;

        pMem = &aMem[pOp->p1];

        if (pOp->p3) {
          rc = sqlite3VdbeMemAggValue(pMem, &aMem[pOp->p3], pOp->p4.pFunc);
          pMem = &aMem[pOp->p3];
        } else {
          rc = sqlite3VdbeMemFinalize(pMem, pOp->p4.pFunc);
        }

        if (rc) {
          sqlite3VdbeError(p, "%s", sqlite3_value_text(pMem));
          goto abort_due_to_error;
        }
        sqlite3VdbeChangeEncoding(pMem, encoding);
        break;
      }

      case 3: {
        int i;
        int aRes[3];
        Mem *pMem;

        aRes[0] = 0;
        aRes[1] = aRes[2] = -1;

        rc = sqlite3Checkpoint(db, pOp->p1, pOp->p2, &aRes[1], &aRes[2]);
        if (rc) {
          if (rc != SQLITE_BUSY)
            goto abort_due_to_error;
          rc = SQLITE_OK;
          aRes[0] = 1;
        }
        for (i = 0, pMem = &aMem[pOp->p3]; i < 3; i++, pMem++) {
          sqlite3VdbeMemSetInt64(pMem, (i64)aRes[i]);
        }
        break;
      };

      case 4: {
        Btree *pBt;
        Pager *pPager;
        int eNew;
        int eOld;

        const char *zFilename;

        pOut = out2Prerelease(p, pOp);
        eNew = pOp->p3;

        pBt = db->aDb[pOp->p1].pBt;
        pPager = sqlite3BtreePager(pBt);
        eOld = sqlite3PagerGetJournalMode(pPager);
        if (eNew == (-1))
          eNew = eOld;

        if (!sqlite3PagerOkToChangeJournalMode(pPager))
          eNew = eOld;

        zFilename = sqlite3PagerFilename(pPager, 1);

        if (eNew == 5 && (sqlite3Strlen30(zFilename) == 0 || !sqlite3PagerWalSupported(pPager))) {
          eNew = eOld;
        }

        if ((eNew != eOld) && (eOld == 5 || eNew == 5)) {
          if (!db->autoCommit || db->nVdbeRead > 1) {
            rc = SQLITE_ERROR;
            sqlite3VdbeError(p, "cannot change %s wal mode from within a transaction", (eNew == 5 ? "into" : "out of"));
            goto abort_due_to_error;
          } else {
            if (eOld == 5) {
              rc = sqlite3PagerCloseWal(pPager, db);
              if (rc == SQLITE_OK) {
                sqlite3PagerSetJournalMode(pPager, eNew);
              }
            } else if (eOld == 4) {
              sqlite3PagerSetJournalMode(pPager, 2);
            }

            if (rc == SQLITE_OK) {
              rc = sqlite3BtreeSetVersion(pBt, (eNew == 5 ? 2 : 1));
            }
          }
        }

        if (rc)
          eNew = eOld;
        eNew = sqlite3PagerSetJournalMode(pPager, eNew);

        pOut->flags = 0x0002 | 0x2000 | 0x0200;
        pOut->z = (char *)sqlite3JournalModename(eNew);
        pOut->n = sqlite3Strlen30(pOut->z);
        pOut->enc = SQLITE_UTF8;
        sqlite3VdbeChangeEncoding(pOut, encoding);
        if (rc)
          goto abort_due_to_error;
        break;
      };

      case 5: {
        rc = sqlite3RunVacuum(&p->zErrMsg, db, pOp->p1, pOp->p2 ? &aMem[pOp->p2] : 0);
        if (rc)
          goto abort_due_to_error;
        break;
      }

      case 64: {
        Btree *pBt;

        pBt = db->aDb[pOp->p1].pBt;
        rc = sqlite3BtreeIncrVacuum(pBt);
        if (rc) {
          if (rc != SQLITE_DONE)
            goto abort_due_to_error;
          rc = SQLITE_OK;
          goto jump_to_p2;
        }
        break;
      }

      case 168: {
        if (!pOp->p1) {
          sqlite3ExpirePreparedStatements(db, pOp->p2);
        } else {
          p->expired = pOp->p2 + 1;
        }
        break;
      }

      case 169: {
        VdbeCursor *pC;

        pC = p->apCsr[pOp->p1];

        sqlite3BtreeCursorPin(pC->uc.pCursor);
        break;
      }

      case 170: {
        VdbeCursor *pC;

        pC = p->apCsr[pOp->p1];

        sqlite3BtreeCursorUnpin(pC->uc.pCursor);
        break;
      }

      case 171: {
        u8 isWriteLock = (u8)pOp->p3;
        if (isWriteLock || 0 == (db->flags & ((u64)(0x00004) << 32))) {
          int p1 = pOp->p1;

          rc = sqlite3BtreeLockTable(db->aDb[p1].pBt, pOp->p2, isWriteLock);
          if (rc) {
            if ((rc & 0xFF) == SQLITE_LOCKED) {
              const char *z = pOp->p4.z;
              sqlite3VdbeError(p, "database table is locked: %s", z);
            }
            goto abort_due_to_error;
          }
        }
        break;
      }

      case 172: {
        VTable *pVTab;
        pVTab = pOp->p4.pVtab;
        rc = sqlite3VtabBegin(db, pVTab);
        if (pVTab)
          sqlite3VtabImportErrmsg(p, pVTab->pVtab);
        if (rc)
          goto abort_due_to_error;
        break;
      }

      case 173: {
        Mem sMem;
        const char *zTab;

        memset(&sMem, 0, sizeof(sMem));
        sMem.db = db;

        rc = sqlite3VdbeMemCopy(&sMem, &aMem[pOp->p2]);

        zTab = (const char *)sqlite3_value_text(&sMem);

        if (zTab) {
          rc = sqlite3VtabCallCreate(db, pOp->p1, zTab, &p->zErrMsg);
        }
        sqlite3VdbeMemRelease(&sMem);
        if (rc)
          goto abort_due_to_error;
        break;
      }

      case 174: {
        db->nVDestroy++;
        rc = sqlite3VtabCallDestroy(db, pOp->p1, pOp->p4.z);
        db->nVDestroy--;

        if (rc)
          goto abort_due_to_error;
        break;
      }

      case 175: {
        VdbeCursor *pCur;
        sqlite3_vtab_cursor *pVCur;
        sqlite3_vtab *pVtab;
        const sqlite3_module *pModule;

        pCur = p->apCsr[pOp->p1];
        if (pCur != 0 && (pCur->eCurType == 2) && (pCur->uc.pVCur->pVtab == pOp->p4.pVtab->pVtab)) {
          break;
        }
        pVCur = 0;
        pVtab = pOp->p4.pVtab->pVtab;
        if (pVtab == 0 || (pVtab->pModule == 0)) {
          rc = SQLITE_LOCKED;
          goto abort_due_to_error;
        }
        pModule = pVtab->pModule;
        rc = pModule->xOpen(pVtab, &pVCur);
        sqlite3VtabImportErrmsg(p, pVtab);
        if (rc)
          goto abort_due_to_error;

        pVCur->pVtab = pVtab;

        pCur = allocateCursor(p, pOp->p1, 0, 2);
        if (pCur) {
          pCur->uc.pVCur = pVCur;
          pVtab->nRef++;
        } else {
          pModule->xClose(pVCur);
          goto no_mem;
        }
        break;
      }

      case 176: {
        Table *pTab;
        sqlite3_vtab *pVtab;
        const sqlite3_module *pModule;
        char *zErr = 0;

        pOut = &aMem[pOp->p2];
        sqlite3VdbeMemSetNull(pOut);

        pTab = pOp->p4.pTab;

        if (pTab->u.vtab.p == 0)
          break;
        pVtab = pTab->u.vtab.p->pVtab;

        pModule = pVtab->pModule;

        sqlite3VtabLock(pTab->u.vtab.p);

        rc = pModule->xIntegrity(pVtab, db->aDb[pOp->p1].zDbSName, pTab->zName, pOp->p3, &zErr);
        sqlite3VtabUnlock(pTab->u.vtab.p);
        if (rc) {
          sqlite3_free(zErr);
          goto abort_due_to_error;
        }
        if (zErr) {
          sqlite3VdbeMemSetStr(pOut, zErr, -1, SQLITE_UTF8, sqlite3_free);
        }
        break;
      }

      case 177: {
        VdbeCursor *pC;
        ValueList *pRhs;

        pC = p->apCsr[pOp->p1];
        pRhs = sqlite3_malloc64(sizeof(*pRhs));
        if (pRhs == 0)
          goto no_mem;
        pRhs->pCsr = pC->uc.pCursor;
        pRhs->pOut = &aMem[pOp->p3];
        pOut = out2Prerelease(p, pOp);
        pOut->flags = 0x0001;
        sqlite3VdbeMemSetPointer(pOut, pRhs, "ValueList", sqlite3VdbeValueListFree);
        break;
      }

      case 6: {
        int nArg;
        int iQuery;
        const sqlite3_module *pModule;
        Mem *pQuery;
        Mem *pArgc;
        sqlite3_vtab_cursor *pVCur;
        sqlite3_vtab *pVtab;
        VdbeCursor *pCur;
        int res;
        int i;
        Mem **apArg;

        pQuery = &aMem[pOp->p3];
        pArgc = &pQuery[1];
        pCur = p->apCsr[pOp->p1];

        pVCur = pCur->uc.pVCur;
        pVtab = pVCur->pVtab;
        pModule = pVtab->pModule;

        nArg = (int)pArgc->u.i;
        iQuery = (int)pQuery->u.i;

        apArg = p->apArg;

        for (i = 0; i < nArg; i++) {
          apArg[i] = &pArgc[i + 1];
        }
        rc = pModule->xFilter(pVCur, iQuery, pOp->p4.z, nArg, apArg);
        sqlite3VtabImportErrmsg(p, pVtab);
        if (rc)
          goto abort_due_to_error;
        res = pModule->xEof(pVCur);
        pCur->nullRow = 0;
        if (res)
          goto jump_to_p2;
        break;
      }

      case 178: {
        sqlite3_vtab *pVtab;
        const sqlite3_module *pModule;
        Mem *pDest;
        sqlite3_context sContext;
        FuncDef nullFunc;

        VdbeCursor *pCur = p->apCsr[pOp->p1];

        pDest = &aMem[pOp->p3];
        if (pCur->nullRow) {
          sqlite3VdbeMemSetNull(pDest);
          break;
        }

        pVtab = pCur->uc.pVCur->pVtab;
        pModule = pVtab->pModule;

        memset(&sContext, 0, sizeof(sContext));
        sContext.pOut = pDest;
        sContext.enc = encoding;
        nullFunc.pUserData = 0;
        nullFunc.funcFlags = SQLITE_RESULT_SUBTYPE;
        sContext.pFunc = &nullFunc;

        if (pOp->p5 & 0x01) {
          sqlite3VdbeMemSetNull(pDest);
          pDest->flags = 0x0001 | 0x0400;
          pDest->u.nZero = 0;
        } else {
          ((pDest)->flags = ((pDest)->flags & ~(0x0dbf | 0x0400)) | 0x0001);
        }
        rc = pModule->xColumn(pCur->uc.pVCur, &sContext, pOp->p2);
        sqlite3VtabImportErrmsg(p, pVtab);
        if (sContext.isError > 0) {
          sqlite3VdbeError(p, "%s", sqlite3_value_text(pDest));
          rc = sContext.isError;
        }
        sqlite3VdbeChangeEncoding(pDest, encoding);

        if (rc)
          goto abort_due_to_error;
        break;
      }

      case 65: {
        sqlite3_vtab *pVtab;
        const sqlite3_module *pModule;
        int res;
        VdbeCursor *pCur;

        pCur = p->apCsr[pOp->p1];

        if (pCur->nullRow) {
          break;
        }
        pVtab = pCur->uc.pVCur->pVtab;
        pModule = pVtab->pModule;

        rc = pModule->xNext(pCur->uc.pVCur);
        sqlite3VtabImportErrmsg(p, pVtab);
        if (rc)
          goto abort_due_to_error;
        res = pModule->xEof(pCur->uc.pVCur);
        if (!res) {
          goto jump_to_p2_and_check_for_interrupt;
        }
        goto check_for_interrupt;
      }

      case 179: {
        sqlite3_vtab *pVtab;
        Mem *pName;
        int isLegacy;

        isLegacy = (db->flags & 0x04000000);
        db->flags |= 0x04000000;
        pVtab = pOp->p4.pVtab->pVtab;
        pName = &aMem[pOp->p1];

        rc = sqlite3VdbeChangeEncoding(pName, SQLITE_UTF8);
        if (rc)
          goto abort_due_to_error;
        rc = pVtab->pModule->xRename(pVtab, pName->z);
        if (isLegacy == 0)
          db->flags &= ~(u64)0x04000000;
        sqlite3VtabImportErrmsg(p, pVtab);
        p->expired = 0;
        if (rc)
          goto abort_due_to_error;
        break;
      }

      case 7: {
        sqlite3_vtab *pVtab;
        const sqlite3_module *pModule;
        int nArg;
        int i;
        sqlite_int64 rowid = 0;
        Mem **apArg;
        Mem *pX;

        if (db->mallocFailed)
          goto no_mem;
        pVtab = pOp->p4.pVtab->pVtab;
        if (pVtab == 0 || (pVtab->pModule == 0)) {
          rc = SQLITE_LOCKED;
          goto abort_due_to_error;
        }
        pModule = pVtab->pModule;
        nArg = pOp->p2;

        if ((pModule->xUpdate)) {
          u8 vtabOnConflict = db->vtabOnConflict;
          apArg = p->apArg;
          pX = &aMem[pOp->p3];

          for (i = 0; i < nArg; i++) {
            apArg[i] = pX;
            pX++;
          }
          db->vtabOnConflict = pOp->p5;
          rc = pModule->xUpdate(pVtab, nArg, apArg, &rowid);
          db->vtabOnConflict = vtabOnConflict;
          sqlite3VtabImportErrmsg(p, pVtab);
          if (rc == SQLITE_OK && pOp->p1) {
            db->lastRowid = rowid;
          }
          if ((rc & 0xff) == SQLITE_CONSTRAINT && pOp->p4.pVtab->bConstraint) {
            if (pOp->p5 == 4) {
              rc = SQLITE_OK;
            } else {
              p->errorAction = ((pOp->p5 == 5) ? 2 : pOp->p5);
            }
          } else {
            p->nChange++;
          }
          if (rc)
            goto abort_due_to_error;
        }
        break;
      }

      case 180: {
        pOut = out2Prerelease(p, pOp);
        pOut->u.i = sqlite3BtreeLastPage(db->aDb[pOp->p1].pBt);
        break;
      }

      case 181: {
        unsigned int newMax;
        Btree *pBt;

        pOut = out2Prerelease(p, pOp);
        pBt = db->aDb[pOp->p1].pBt;
        newMax = 0;
        if (pOp->p3) {
          newMax = sqlite3BtreeLastPage(pBt);
          if (newMax < (unsigned)pOp->p3)
            newMax = (unsigned)pOp->p3;
        }
        pOut->u.i = sqlite3BtreeMaxPageCount(pBt, newMax);
        break;
      }

      case 67:
      case 68: {
        int i;
        sqlite3_context *pCtx;

        pCtx = pOp->p4.pCtx;

        pOut = &aMem[pOp->p3];
        if (pCtx->pOut != pOut) {
          pCtx->pVdbe = p;
          pCtx->pOut = pOut;
          pCtx->enc = encoding;
          for (i = pCtx->argc - 1; i >= 0; i--)
            pCtx->argv[i] = &aMem[pOp->p2 + i];
        }

        ((pOut)->flags = ((pOut)->flags & ~(0x0dbf | 0x0400)) | 0x0001);

        (*pCtx->pFunc->xSFunc)(pCtx, pCtx->argc, pCtx->argv);

        if (pCtx->isError) {
          if (pCtx->isError > 0) {
            sqlite3VdbeError(p, "%s", sqlite3_value_text(pOut));
            rc = pCtx->isError;
          }
          sqlite3VdbeDeleteAuxData(db, &p->pAuxData, pCtx->iOp, pOp->p1);
          pCtx->isError = 0;
          if (rc)
            goto abort_due_to_error;
        }

        break;
      }

      case 182: {
        pIn1 = &aMem[pOp->p1];
        pIn1->flags &= ~0x0800;
        break;
      }

      case 183: {
        pIn1 = &aMem[pOp->p1];
        pOut = &aMem[pOp->p2];
        if (pIn1->flags & 0x0800) {
          sqlite3VdbeMemSetInt64(pOut, pIn1->eSubtype);
        } else {
          sqlite3VdbeMemSetNull(pOut);
        }
        break;
      }

      case 184: {
        pIn1 = &aMem[pOp->p1];
        pOut = &aMem[pOp->p2];
        if (pIn1->flags & 0x0001) {
          pOut->flags &= ~0x0800;
        } else {
          pOut->flags |= 0x0800;
          pOut->eSubtype = (u8)(pIn1->u.i & 0xff);
        }
        break;
      }

      case 185: {
        u64 h;

        pIn1 = &aMem[pOp->p1];

        h = filterHash(aMem, pOp);

        h %= (pIn1->n * 8);
        pIn1->z[h / 8] |= 1 << (h & 7);
        break;
      }

      case 66: {
        u64 h;

        pIn1 = &aMem[pOp->p1];

        h = filterHash(aMem, pOp);

        h %= (pIn1->n * 8);
        if ((pIn1->z[h / 8] & (1 << (h & 7))) == 0) {
          p->aCounter[SQLITE_STMTSTATUS_FILTER_HIT]++;
          goto jump_to_p2;
        } else {
          p->aCounter[SQLITE_STMTSTATUS_FILTER_MISS]++;
        }
        break;
      }

      case 186:
      case 8: {
        int i;

        char *zTrace;

        if ((db->mTrace & (SQLITE_TRACE_STMT | 0x40)) != 0 && p->minWriteFileFormat != 254 &&
            (zTrace = (pOp->p4.z ? pOp->p4.z : p->zSql)) != 0) {
          if (db->mTrace & 0x40) {
            char *z = sqlite3VdbeExpandSql(p, zTrace);
            db->trace.xLegacy(db->pTraceArg, z);
            sqlite3_free(z);
          } else if (db->nVdbeExec > 1) {
            char *z = sqlite3MPrintf(db, "-- %s", zTrace);
            (void)db->trace.xV2(SQLITE_TRACE_STMT, db->pTraceArg, p, z);
            sqlite3DbFree(db, z);
          } else {
            (void)db->trace.xV2(SQLITE_TRACE_STMT, db->pTraceArg, p, zTrace);
          }
        }

        if (pOp->p1 >= sqlite3Config.iOnceResetThreshold) {
          if (pOp->opcode == 186)
            break;
          for (i = 1; i < p->nOp; i++) {
            if (p->aOp[i].opcode == 15)
              p->aOp[i].p1 = 0;
          }
          pOp->p1 = 0;
        }
        pOp->p1++;
        p->aCounter[SQLITE_STMTSTATUS_RUN]++;
        goto jump_to_p2;
      }

      default: {
        break;
      }
    }
  }

abort_due_to_error:
  if (db->mallocFailed) {
    rc = 7;
  } else if (rc == (10 | (33 << 8))) {
    rc = sqlite3CorruptError(105897);
  }

  if (p->zErrMsg == 0 && rc != (10 | (12 << 8))) {
    sqlite3VdbeError(p, "%s", sqlite3ErrStr(rc));
  }
  p->rc = rc;
  sqlite3SystemError(db, rc);
  sqlite3VdbeLogAbort(p, rc, pOp, aOp);
  if (p->eVdbeState == 2)
    sqlite3VdbeHalt(p);
  if (rc == (10 | (12 << 8)))
    sqlite3OomFault(db);
  if (rc == SQLITE_CORRUPT && db->autoCommit == 0) {
    db->flags |= ((u64)(0x00002) << 32);
  }
  rc = SQLITE_ERROR;
  if (resetSchemaOnFault > 0) {
    sqlite3ResetOneSchema(db, resetSchemaOnFault - 1);
  }

vdbe_return:
  while (nVmStep >= nProgressLimit && db->xProgress != 0) {
    nProgressLimit += db->nProgressOps;
    if (db->xProgress(db->pProgressArg)) {
      nProgressLimit = (0xffffffff | (((u64)0xffffffff) << 32));
      rc = SQLITE_INTERRUPT;
      goto abort_due_to_error;
    }
  }

  p->aCounter[SQLITE_STMTSTATUS_VM_STEP] += (int)nVmStep;
  if (((p->lockMask) != 0)) {
    sqlite3VdbeLeave(p);
  }

  return rc;

too_big:
  sqlite3VdbeError(p, "string or blob too big");
  rc = SQLITE_TOOBIG;
  goto abort_due_to_error;

no_mem:
  sqlite3OomFault(db);
  sqlite3VdbeError(p, "out of memory");
  rc = 7;
  goto abort_due_to_error;

abort_due_to_interrupt:
  rc = SQLITE_INTERRUPT;
  goto abort_due_to_error;
}

MergeEngine *vdbeMergeEngineNew(int nReader) {
  int N = 2;
  i64 nByte;
  MergeEngine *pNew;

  while (N < nReader)
    N += N;
  nByte = sizeof(MergeEngine) + N * (sizeof(int) + sizeof(PmaReader));

  pNew = sqlite3FaultSim(100) ? 0 : (MergeEngine *)sqlite3MallocZero(nByte);
  if (pNew) {
    pNew->nTree = N;
    pNew->pTask = 0;
    pNew->aReadr = (PmaReader *)&pNew[1];
    pNew->aTree = (int *)&pNew->aReadr[N];
  }
  return pNew;
}

void *vdbeIncrPopulateThread(void *pCtx) {
  IncrMerger *pIncr = (IncrMerger *)pCtx;
  void *pRet = ((void *)(intptr_t)(vdbeIncrPopulate(pIncr)));
  pIncr->pTask->bDone = 1;
  return pRet;
}

void *vdbePmaReaderBgIncrInit(void *pCtx) {
  PmaReader *pReader = (PmaReader *)pCtx;
  void *pRet = ((void *)(intptr_t)(vdbePmaReaderIncrMergeInit(pReader, 1)));
  pReader->pIncr->pTask->bDone = 1;
  return pRet;
}

void sqlite3SetHasNullFlag(Vdbe *v, int iCur, int regHasNull) {
  int addr1;
  sqlite3VdbeAddOp2(v, 73, 0, regHasNull);
  addr1 = sqlite3VdbeAddOp1(v, 36, iCur);
  sqlite3VdbeAddOp3(v, 96, iCur, 0, regHasNull);
  sqlite3VdbeChangeP5(v, 0x80);
  sqlite3VdbeJumpHere(v, addr1);
}

void codeReal(Vdbe *v, const char *z, int negateFlag, int iMem) {
  if ((z != 0)) {
    double value;
    sqlite3AtoF(z, &value);

    if (negateFlag)
      value = -value;
    sqlite3VdbeAddOp4Dup8(v, 154, 0, iMem, 0, (u8 *)&value, (-13));
  }
}

void sqlite3ExprCodeGetColumnOfTable(Vdbe *v, Table *pTab, int iTabCur, int iCol, int regOut) {
  Column *pCol;

  if (iCol < 0 || iCol == pTab->iPKey) {
    sqlite3VdbeAddOp2(v, 137, iTabCur, regOut);
  } else {
    int op;
    int x;
    if ((pTab)->eTabType == 1) {
      op = 178;
      x = iCol;

    } else if ((pCol = &pTab->aCol[iCol])->colFlags & 0x0020) {
      Parse *pParse = sqlite3VdbeParser(v);
      if (pCol->colFlags & 0x0100) {
        sqlite3ErrorMsg(pParse, "generated column loop on \"%s\"", pCol->zCnName);
      } else {
        int savedSelfTab = pParse->iSelfTab;
        pCol->colFlags |= 0x0100;
        pParse->iSelfTab = iTabCur + 1;
        sqlite3ExprCodeGeneratedColumn(pParse, pTab, pCol, regOut);
        pParse->iSelfTab = savedSelfTab;
        pCol->colFlags &= ~0x0100;
      }
      return;

    } else if (!(((pTab)->tabFlags & 0x00000080) == 0)) {
      x = sqlite3TableColumnToIndex(sqlite3PrimaryKeyIndex(pTab), iCol);
      op = 96;
    } else {
      x = sqlite3TableColumnToStorage(pTab, iCol);
      op = 96;
    }
    sqlite3VdbeAddOp3(v, op, iTabCur, x, regOut);
    sqlite3ColumnDefault(v, pTab, iCol, regOut);
  }
}

void setDoNotMergeFlagOnCopy(Vdbe *v) {
  if (sqlite3VdbeGetLastOp(v)->opcode == 82) {
    sqlite3VdbeChangeP5(v, 1);
  }
}

void sqlite3CodeChangeCount(Vdbe *v, int regCounter, const char *zColName) {
  sqlite3VdbeAddOp0(v, 85);
  sqlite3VdbeAddOp2(v, 86, regCounter, 1);
  sqlite3VdbeSetNumCols(v, 1);
  sqlite3VdbeSetColName(v, 0, 0, zColName, ((sqlite3_destructor_type)0));
}

void sqlite3TableAffinity(Vdbe *v, Table *pTab, int iReg) {
  int i;
  char *zColAff;
  if (pTab->tabFlags & 0x00010000) {
    if (iReg == 0) {
      VdbeOp *pPrev;
      int p3;
      sqlite3VdbeAppendP4(v, pTab, (-5));
      pPrev = sqlite3VdbeGetLastOp(v);

      pPrev->opcode = 97;
      p3 = pPrev->p3;
      pPrev->p3 = 0;
      sqlite3VdbeAddOp3(v, 99, pPrev->p1, pPrev->p2, p3);
    } else {
      sqlite3VdbeAddOp2(v, 97, iReg, pTab->nNVCol);
      sqlite3VdbeAppendP4(v, pTab, (-5));
    }
    return;
  }
  zColAff = pTab->zColAff;
  if (zColAff == 0) {
    zColAff = sqlite3TableAffinityStr(0, pTab);
    if (!zColAff) {
      sqlite3OomFault(sqlite3VdbeDb(v));
      return;
    }
    pTab->zColAff = zColAff;
  }

  i = (strlen(zColAff) & 0x3fffffff);
  if (i) {
    if (iReg) {
      sqlite3VdbeAddOp4(v, 98, iReg, i, 0, zColAff, i);
    } else {
      sqlite3VdbeChangeP4(v, -1, zColAff, i);
    }
  }
}

void setPragmaResultColumnNames(Vdbe *v, const PragmaName *pPragma) {
  u8 n = pPragma->nPragCName;
  sqlite3VdbeSetNumCols(v, n == 0 ? 1 : n);
  if (n == 0) {
    sqlite3VdbeSetColName(v, 0, 0, pPragma->zName, ((sqlite3_destructor_type)0));
  } else {
    int i, j;
    for (i = 0, j = pPragma->iPragCName; i < n; i++, j++) {
      sqlite3VdbeSetColName(v, i, 0, pragCName[j], ((sqlite3_destructor_type)0));
    }
  }
}

void returnSingleInt(Vdbe *v, i64 value) {
  sqlite3VdbeAddOp4Dup8(v, 74, 0, 1, 0, (const u8 *)&value, (-14));
  sqlite3VdbeAddOp2(v, 86, 1, 1);
}

void returnSingleText(Vdbe *v, const char *zValue) {
  if (zValue) {
    sqlite3VdbeLoadString(v, 1, (const char *)zValue);
    sqlite3VdbeAddOp2(v, 86, 1, 1);
  }
}

void pragmaFunclistLine(Vdbe *v, FuncDef *p, int isBuiltin, int showInternFuncs) {
  u32 mask = SQLITE_DETERMINISTIC | SQLITE_DIRECTONLY | SQLITE_SUBTYPE | SQLITE_INNOCUOUS | 0x00040000;
  if (showInternFuncs)
    mask = 0xffffffff;
  for (; p; p = p->pNext) {
    const char *zType;
    static const char *azEnc[] = {0, "utf8", "utf16le", "utf16be"};

    if (p->xSFunc == 0)
      continue;
    if ((p->funcFlags & 0x00040000) != 0 && showInternFuncs == 0) {
      continue;
    }
    if (p->xValue != 0) {
      zType = "w";
    } else if (p->xFinalize != 0) {
      zType = "a";
    } else {
      zType = "s";
    }
    sqlite3VdbeMultiLoad(v, 1, "sissii", p->zName, isBuiltin, zType, azEnc[p->funcFlags & 0x0003], p->nArg,
                         (p->funcFlags & mask) ^ SQLITE_INNOCUOUS);
  }
}

int integrityCheckResultRow(Vdbe *v) {
  int addr;
  sqlite3VdbeAddOp2(v, 86, 3, 1);
  addr = sqlite3VdbeAddOp3(v, 61, 1, sqlite3VdbeCurrentAddr(v) + 2, 1);
  sqlite3VdbeAddOp0(v, 72);
  return addr;
}

int sqlite3Reprepare(Vdbe *p) {
  int rc;
  sqlite3_stmt *pNew;
  const char *zSql;
  sqlite3 *db;
  u8 prepFlags;

  zSql = sqlite3_sql((sqlite3_stmt *)p);

  db = sqlite3VdbeDb(p);

  prepFlags = sqlite3VdbePrepareFlags(p);
  rc = sqlite3LockAndPrepare(db, zSql, -1, prepFlags, p, &pNew, 0);
  if (rc) {
    if (rc == SQLITE_NOMEM) {
      sqlite3OomFault(db);
    }

    return rc;
  } else {
  }
  sqlite3VdbeSwap((Vdbe *)pNew, p);
  sqlite3TransferBindings(pNew, (sqlite3_stmt *)p);
  sqlite3VdbeResetStepResult((Vdbe *)pNew);
  sqlite3VdbeFinalize((Vdbe *)pNew);
  return SQLITE_OK;
}

void codeOffset(Vdbe *v, int iOffset, int iContinue) {
  if (iOffset > 0) {
    sqlite3VdbeAddOp3(v, 61, iOffset, iContinue, 1);
  }
}

void sqlite3ColumnDefault(Vdbe *v, Table *pTab, int i, int iReg) {
  Column *pCol;

  pCol = &pTab->aCol[i];
  if (pCol->iDflt) {
    sqlite3_value *pValue = 0;
    u8 enc = ((sqlite3VdbeDb(v))->enc);

    sqlite3ValueFromExpr(sqlite3VdbeDb(v), sqlite3ColumnExpr(pTab, pCol), enc, pCol->affinity, &pValue);
    if (pValue) {
      sqlite3VdbeAppendP4(v, pValue, (-11));
    }
  }

  if (pCol->affinity == 0x45 && !((pTab)->eTabType == 1)) {
    sqlite3VdbeAddOp1(v, 89, iReg);
  }
}

void whereLikeOptimizationStringFixup(Vdbe *v, WhereLevel *pLevel, WhereTerm *pTerm) {
  if (pTerm->wtFlags & 0x0100) {
    VdbeOp *pOp;

    pOp = sqlite3VdbeGetLastOp(v);

    pOp->p3 = (int)(pLevel->iLikeRepCntr >> 1);
    pOp->p5 = (u8)(pLevel->iLikeRepCntr & 1);
  }
}

void sqlite3WhereMinMaxOptEarlyOut(Vdbe *v, WhereInfo *pWInfo) {
  WhereLevel *pInner;
  int i;
  if (!pWInfo->bOrderedInnerLoop)
    return;
  if (pWInfo->nOBSat == 0)
    return;
  for (i = pWInfo->nLevel - 1; i >= 0; i--) {
    pInner = &pWInfo->a[i];
    if ((pInner->pWLoop->wsFlags & 0x00000004) != 0) {
      sqlite3VdbeGoto(v, pInner->addrNxt);
      return;
    }
  }
  sqlite3VdbeGoto(v, pWInfo->iBreak);
}