#define _GNU_SOURCE 1
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include "sqlite/sqlite3_value.h"
#include "sqlite/BtCursor.h"
#include "sqlite/JsonParse.h"
#include "sqlite/Mem.h"
#include "sqlite/ValueList.h"
#include "sqlite/Vdbe.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/sqlite_int64.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
#include "sqlite/SqliteFundamentalDatatype.h"
#include "sqlite/SqliteResultCode.h"
#include "sqlite/SqliteTextEncoding.h"
void sqlite3ValueSetNull(sqlite3_value *p) {
  sqlite3VdbeMemSetNull((Mem *)p);
}

void sqlite3MemSetArrayInt64(sqlite3_value *aMem, int iIdx, i64 val) {
  sqlite3VdbeMemSetInt64(&aMem[iIdx], val);
}

__attribute__((noinline)) const void *valueToText(sqlite3_value *pVal, u8 enc) {
  if (pVal->flags & (0x0010 | 0x0002)) {
    if ((((pVal)->flags & 0x0400) ? sqlite3VdbeMemExpandBlob(pVal) : 0))
      return 0;
    pVal->flags |= 0x0002;
    if (pVal->enc != (enc & ~SQLITE_UTF16_ALIGNED)) {
      sqlite3VdbeChangeEncoding(pVal, enc & ~SQLITE_UTF16_ALIGNED);
    }
    if ((enc & SQLITE_UTF16_ALIGNED) != 0 && 1 == (1 & ((int)(intptr_t)(pVal->z)))) {
      if (sqlite3VdbeMemMakeWriteable(pVal) != SQLITE_OK) {
        return 0;
      }
    }
    sqlite3VdbeMemNulTerminate(pVal);
  } else {
    sqlite3VdbeMemStringify(pVal, enc, 0);
  }

  if (pVal->enc == (enc & ~SQLITE_UTF16_ALIGNED)) {
    return pVal->z;
  } else {
    return 0;
  }
}

const void *sqlite3ValueText(sqlite3_value *pVal, u8 enc) {
  if (!pVal)
    return 0;

  if ((pVal->flags & (0x0002 | 0x0200)) == (0x0002 | 0x0200) && pVal->enc == enc) {
    return pVal->z;
  }
  if (pVal->flags & 0x0001) {
    return 0;
  }
  return valueToText(pVal, enc);
}

int sqlite3ValueIsOfClass(const sqlite3_value *pVal, void (*xFree)(void *)) {
  if ((pVal != 0) && ((pVal->flags & (0x0002 | 0x0010)) != 0) && (pVal->flags & 0x1000) != 0 && pVal->xDel == xFree) {
    return 1;
  } else {
    return 0;
  }
}

void sqlite3ValueSetStr(sqlite3_value *v, int n, const void *z, u8 enc, void (*xDel)(void *)) {
  if (v)
    sqlite3VdbeMemSetStr((Mem *)v, (const char*)(z), n, enc, xDel);
}

void sqlite3ValueFree(sqlite3_value *v) {
  if (!v)
    return;
  sqlite3VdbeMemRelease((Mem *)v);
  sqlite3DbFreeNN(((Mem *)v)->db, v);
}

__attribute__((noinline)) int valueBytes(sqlite3_value *pVal, u8 enc) {
  return valueToText(pVal, enc) != 0 ? pVal->n : 0;
}

int sqlite3ValueBytes(sqlite3_value *pVal, u8 enc) {
  Mem *p = (Mem *)pVal;

  if ((p->flags & 0x0002) != 0 && pVal->enc == enc) {
    return p->n;
  }
  if ((p->flags & 0x0002) != 0 && enc != SQLITE_UTF8 && pVal->enc != SQLITE_UTF8) {
    return p->n;
  }
  if ((p->flags & 0x0010) != 0) {
    if (p->flags & 0x0400) {
      return p->n + p->u.nZero;
    } else {
      return p->n;
    }
  }
  if (p->flags & 0x0001)
    return 0;
  return valueBytes(pVal, enc);
}

const void *sqlite3_value_blob(sqlite3_value *pVal) {
  Mem *p = (Mem *)pVal;
  if (p->flags & (0x0010 | 0x0002)) {
    if ((((p)->flags & 0x0400) ? sqlite3VdbeMemExpandBlob(p) : 0) != SQLITE_OK) {
      return 0;
    }
    p->flags |= 0x0010;
    return p->n ? p->z : 0;
  } else {
    return sqlite3_value_text(pVal);
  }
}

int sqlite3_value_bytes(sqlite3_value *pVal) {
  return sqlite3ValueBytes(pVal, SQLITE_UTF8);
}

int sqlite3_value_bytes16(sqlite3_value *pVal) {
  return sqlite3ValueBytes(pVal, 2);
}

double sqlite3_value_double(sqlite3_value *pVal) {
  return sqlite3VdbeRealValue((Mem *)pVal);
}

int sqlite3_value_int(sqlite3_value *pVal) {
  return (int)sqlite3VdbeIntValue((Mem *)pVal);
}

sqlite_int64 sqlite3_value_int64(sqlite3_value *pVal) {
  return sqlite3VdbeIntValue((Mem *)pVal);
}

unsigned int sqlite3_value_subtype(sqlite3_value *pVal) {
  Mem *pMem = (Mem *)pVal;
  return ((pMem->flags & 0x0800) ? pMem->eSubtype : 0);
}

void *sqlite3_value_pointer(sqlite3_value *pVal, const char *zPType) {
  Mem *p = (Mem *)pVal;
  if ((p->flags & (0x0dbf | 0x0200 | 0x0800)) == (0x0001 | 0x0200 | 0x0800) && zPType != 0 && p->eSubtype == 'p' &&
      strcmp(p->u.zPType, zPType) == 0) {
    return (void *)p->z;
  } else {
    return 0;
  }
}

const unsigned char *sqlite3_value_text(sqlite3_value *pVal) {
  return (const unsigned char *)sqlite3ValueText(pVal, SQLITE_UTF8);
}

const void *sqlite3_value_text16(sqlite3_value *pVal) {
  return sqlite3ValueText(pVal, 2);
}

const void *sqlite3_value_text16be(sqlite3_value *pVal) {
  return sqlite3ValueText(pVal, SQLITE_UTF16BE);
}

const void *sqlite3_value_text16le(sqlite3_value *pVal) {
  return sqlite3ValueText(pVal, SQLITE_UTF16LE);
}

int sqlite3_value_type(sqlite3_value *pVal) {
  static const u8 aType[] = {
      SQLITE_BLOB, 5, 3, 5, 1, 5, 1, 5, 2, 5, 2, 5, 1, 5, 1, 5, 4, 5, 3, 5, 1, 5, 1, 5, 2, 5, 2, 5, 1, 5, 1, 5,
      2,           5, 2, 5, 2, 5, 2, 5, 2, 5, 2, 5, 2, 5, 2, 5, 4, 5, 3, 5, 2, 5, 2, 5, 2, 5, 2, 5, 2, 5, 2, 5,
  };

  return aType[pVal->flags & 0x003f];
}

int sqlite3_value_encoding(sqlite3_value *pVal) {
  return pVal->enc;
}

int sqlite3_value_nochange(sqlite3_value *pVal) {
  return (pVal->flags & (0x0001 | 0x0400)) == (0x0001 | 0x0400);
}

int sqlite3_value_frombind(sqlite3_value *pVal) {
  return (pVal->flags & 0x0040) != 0;
}

sqlite3_value *sqlite3_value_dup(const sqlite3_value *pOrig) {
  sqlite3_value *pNew;
  if (pOrig == 0)
    return 0;
  pNew = (sqlite3_value*)(sqlite3_malloc(sizeof(*pNew)));
  if (pNew == 0)
    return 0;
  memset(pNew, 0, sizeof(*pNew));
  memcpy(pNew, pOrig, offsetof(Mem, db));
  pNew->flags &= ~0x1000;
  pNew->db = 0;
  if (pNew->flags & (0x0002 | 0x0010)) {
    pNew->flags &= ~(0x2000 | 0x1000);
    pNew->flags |= 0x4000;
    if (sqlite3VdbeMemMakeWriteable(pNew) != SQLITE_OK) {
      sqlite3ValueFree(pNew);
      pNew = 0;
    }
  } else if (pNew->flags & 0x0001) {
    pNew->flags &= ~(0x0200 | 0x0800);
  }
  return pNew;
}

void sqlite3_value_free(sqlite3_value *pOld) {
  sqlite3ValueFree(pOld);
}

int valueFromValueList(sqlite3_value *pVal, sqlite3_value **ppOut, int bNext) {
  int rc;
  ValueList *pRhs;

  *ppOut = 0;
  if (pVal == 0)
    return sqlite3MisuseError(94729);
  if ((pVal->flags & 0x1000) == 0 || pVal->xDel != sqlite3VdbeValueListFree) {
    return SQLITE_ERROR;
  } else {
    pRhs = (ValueList *)pVal->z;
  }
  if (bNext) {
    rc = sqlite3BtreeNext(pRhs->pCsr, 0);
  } else {
    int dummy = 0;
    rc = sqlite3BtreeFirst(pRhs->pCsr, &dummy);

    if (sqlite3BtreeEof(pRhs->pCsr))
      rc = SQLITE_DONE;
  }
  if (rc == SQLITE_OK) {
    u32 sz;
    Mem sMem;
    memset(&sMem, 0, sizeof(sMem));
    sz = sqlite3BtreePayloadSize(pRhs->pCsr);
    rc = sqlite3VdbeMemFromBtreeZeroOffset(pRhs->pCsr, sz, &sMem);
    if (rc == SQLITE_OK) {
      u8 *zBuf = (u8 *)sMem.z;
      u32 iSerial;
      sqlite3_value *pOut = pRhs->pOut;
      int iOff = 1 + (u8)((*(&zBuf[1]) < (u8)0x80) ? ((iSerial) = (u32) * (&zBuf[1])),
                          1                        : sqlite3GetVarint32((&zBuf[1]), (u32 *)&(iSerial)));
      sqlite3VdbeSerialGet(&zBuf[iOff], iSerial, pOut);
      pOut->enc = ((pOut->db)->enc);
      if ((pOut->flags & 0x4000) != 0 && sqlite3VdbeMemMakeWriteable(pOut)) {
        rc = SQLITE_NOMEM;
      } else {
        *ppOut = pOut;
      }
    }
    sqlite3VdbeMemRelease(&sMem);
  }
  return rc;
}

int sqlite3_vtab_in_first(sqlite3_value *pVal, sqlite3_value **ppOut) {
  return valueFromValueList(pVal, ppOut, 0);
}

int sqlite3_vtab_in_next(sqlite3_value *pVal, sqlite3_value **ppOut) {
  return valueFromValueList(pVal, ppOut, 1);
}

int sqlite3_value_numeric_type(sqlite3_value *pVal) {
  int eType = sqlite3_value_type(pVal);
  if (eType == 3) {
    Mem *pMem = (Mem *)pVal;

    sqlite3_mutex_enter(pMem->db->mutex);
    applyNumericAffinity(pMem, 0);
    sqlite3_mutex_leave(pMem->db->mutex);
    eType = sqlite3_value_type(pVal);
  }
  return eType;
}

void sqlite3ValueApplyAffinity(sqlite3_value *pVal, u8 affinity, u8 enc) {
  applyAffinity((Mem *)pVal, affinity, enc);
}

int jsonArgIsJsonb(sqlite3_value *pArg, JsonParse *p) {
  u32 n, sz = 0;
  u8 c;
  if (sqlite3_value_type(pArg) != SQLITE_BLOB)
    return 0;
  p->aBlob = (u8 *)sqlite3_value_blob(pArg);
  p->nBlob = (u32)sqlite3_value_bytes(pArg);
  if (p->nBlob > 0 && (p->aBlob != 0) && ((c = p->aBlob[0]) & 0x0f) <= 12 && (n = jsonbPayloadSize(p, 0, &sz)) > 0 &&
      sz + n == p->nBlob && ((c & 0x0f) > 2 || sz == 0) &&
      (sz > 7 || (c != 0x7b && c != 0x5b && !(sqlite3CtypeMap[(unsigned char)(c)] & 0x04)) ||
       jsonbValidityCheck(p, 0, p->nBlob, 1) == 0)) {
    return 1;
  }
  p->aBlob = 0;
  p->nBlob = 0;
  return 0;
}
