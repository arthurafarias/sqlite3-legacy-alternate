#define _GNU_SOURCE 1
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "sqlite/Mem.h"
#include "sqlite/CollSeq.h"
#include "sqlite/FuncDef.h"
#include "sqlite/Op.h"
#include "sqlite/RCStr.h"
#include "sqlite/RowSet.h"
#include "sqlite/Vdbe.h"
#include "sqlite/VdbeOp.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_context.h"
#include "sqlite/sqlite3_destructor_type.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_mem_methods.h"
#include "sqlite/sqlite3_uint64.h"
#include "sqlite/sqlite3_value.h"
#include "sqlite/u16.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/SqliteConfigOption.h"
#include "sqlite/SqliteLimitCategory.h"
#include "sqlite/SqliteResultCode.h"
#include "sqlite/SqliteTextEncoding.h"
int isAllZero(const char *z, int n) {
  int i;
  for (i = 0; i < n; i++) {
    if (z[i])
      return 0;
  }
  return 1;
}

void *sqlite3MemMalloc(int nByte) {
  sqlite3_int64 *p;

  p = malloc(nByte + 8);
  if (p) {
    p[0] = nByte;
    p++;
  } else {
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes of memory", nByte);
  }
  return (void *)p;
}

static void sqlite3MemFree(void *pPrior) {
  sqlite3_int64 *p = (sqlite3_int64 *)pPrior;

  p--;
  free(p);
}

static int sqlite3MemSize(void *pPrior) {
  sqlite3_int64 *p;

  p = (sqlite3_int64 *)pPrior;
  p--;
  return (int)p[0];
}

void *sqlite3MemRealloc(void *pPrior, int nByte) {
  sqlite3_int64 *p = (sqlite3_int64 *)pPrior;

  p--;
  p = realloc((p), (nByte + 8));
  if (p) {
    p[0] = nByte;
    p++;
  } else {
    sqlite3_log(SQLITE_NOMEM, "failed memory resize %u to %u bytes", sqlite3MemSize(pPrior), nByte);
  }
  return (void *)p;
}

static int sqlite3MemRoundup(int n) {
  return (((n) + 7) & ~7);
}

static int sqlite3MemInit(void *NotUsed) {
  (void)(NotUsed);
  return SQLITE_OK;
}

static void sqlite3MemShutdown(void *NotUsed) {
  (void)(NotUsed);
  return;
}

void sqlite3MemSetDefault(void) {
  static const sqlite3_mem_methods defaultMethods = {
      sqlite3MemMalloc,  sqlite3MemFree, sqlite3MemRealloc,  sqlite3MemSize,
      sqlite3MemRoundup, sqlite3MemInit, sqlite3MemShutdown, 0};
  sqlite3_config(SQLITE_CONFIG_MALLOC, &defaultMethods);
}

__attribute__((noinline)) int sqlite3VdbeMemTranslate(Mem *pMem, u8 desiredEnc) {
  sqlite3_int64 len;
  unsigned char *zOut;
  unsigned char *zIn;
  unsigned char *zTerm;
  unsigned char *z;
  unsigned int c;

  if (pMem->enc != SQLITE_UTF8 && desiredEnc != SQLITE_UTF8) {
    u8 temp;
    int rc;
    rc = sqlite3VdbeMemMakeWriteable(pMem);
    if (rc != SQLITE_OK) {
      return 7;
    }
    zIn = (u8 *)pMem->z;
    zTerm = &zIn[pMem->n & ~1];
    while (zIn < zTerm) {
      temp = *zIn;
      *zIn = *(zIn + 1);
      zIn++;
      *zIn++ = temp;
    }
    pMem->enc = desiredEnc;
    goto translate_out;
  }

  if (desiredEnc == SQLITE_UTF8) {
    pMem->n &= ~1;
    len = 2 * (sqlite3_int64)pMem->n + 1;
  } else {
    len = 2 * (sqlite3_int64)pMem->n + 2;
  }

  zIn = (u8 *)pMem->z;
  zTerm = &zIn[pMem->n];
  zOut = sqlite3DbMallocRaw(pMem->db, len);
  if (!zOut) {
    return 7;
  }
  z = zOut;

  if (pMem->enc == SQLITE_UTF8) {
    if (desiredEnc == SQLITE_UTF16LE) {
      while (zIn < zTerm) {
        c = *(zIn++);
        if (c >= 0xc0) {
          c = sqlite3Utf8Trans1[c - 0xc0];
          while (zIn < zTerm && (*zIn & 0xc0) == 0x80) {
            c = (c << 6) + (0x3f & *(zIn++));
          }
          if (c < 0x80 || (c & 0xFFFFF800) == 0xD800 || (c & 0xFFFFFFFE) == 0xFFFE) {
            c = 0xFFFD;
          }
        };
        {
          if (c <= 0xFFFF) {
            *z++ = (u8)(c & 0x00FF);
            *z++ = (u8)((c >> 8) & 0x00FF);
          } else {
            *z++ = (u8)(((c >> 10) & 0x003F) + (((c - 0x10000) >> 10) & 0x00C0));
            *z++ = (u8)(0x00D8 + (((c - 0x10000) >> 18) & 0x03));
            *z++ = (u8)(c & 0x00FF);
            *z++ = (u8)(0x00DC + ((c >> 8) & 0x03));
          }
        };
      }
    } else {
      while (zIn < zTerm) {
        c = *(zIn++);
        if (c >= 0xc0) {
          c = sqlite3Utf8Trans1[c - 0xc0];
          while (zIn < zTerm && (*zIn & 0xc0) == 0x80) {
            c = (c << 6) + (0x3f & *(zIn++));
          }
          if (c < 0x80 || (c & 0xFFFFF800) == 0xD800 || (c & 0xFFFFFFFE) == 0xFFFE) {
            c = 0xFFFD;
          }
        };
        {
          if (c <= 0xFFFF) {
            *z++ = (u8)((c >> 8) & 0x00FF);
            *z++ = (u8)(c & 0x00FF);
          } else {
            *z++ = (u8)(0x00D8 + (((c - 0x10000) >> 18) & 0x03));
            *z++ = (u8)(((c >> 10) & 0x003F) + (((c - 0x10000) >> 10) & 0x00C0));
            *z++ = (u8)(0x00DC + ((c >> 8) & 0x03));
            *z++ = (u8)(c & 0x00FF);
          }
        };
      }
    }
    pMem->n = (int)(z - zOut);
    *z++ = 0;
  } else {
    if (pMem->enc == SQLITE_UTF16LE) {
      while (zIn < zTerm) {
        c = *(zIn++);
        c += (*(zIn++)) << 8;
        if (c >= 0xd800 && c < 0xe000) {
          if (zIn < zTerm) {
            int c2 = (*zIn++);
            c2 += ((*zIn++) << 8);
            c = (c2 & 0x03FF) + ((c & 0x003F) << 10) + (((c & 0x03C0) + 0x0040) << 10);
          }
        }
        {
          if (c < 0x00080) {
            *z++ = (u8)(c & 0xFF);
          } else if (c < 0x00800) {
            *z++ = 0xC0 + (u8)((c >> 6) & 0x1F);
            *z++ = 0x80 + (u8)(c & 0x3F);
          } else if (c < 0x10000) {
            *z++ = 0xE0 + (u8)((c >> 12) & 0x0F);
            *z++ = 0x80 + (u8)((c >> 6) & 0x3F);
            *z++ = 0x80 + (u8)(c & 0x3F);
          } else {
            *z++ = 0xF0 + (u8)((c >> 18) & 0x07);
            *z++ = 0x80 + (u8)((c >> 12) & 0x3F);
            *z++ = 0x80 + (u8)((c >> 6) & 0x3F);
            *z++ = 0x80 + (u8)(c & 0x3F);
          }
        };
      }
    } else {
      while (zIn < zTerm) {
        c = (*(zIn++)) << 8;
        c += *(zIn++);
        if (c >= 0xd800 && c < 0xe000) {
          if (zIn < zTerm) {
            int c2 = ((*zIn++) << 8);
            c2 += (*zIn++);
            c = (c2 & 0x03FF) + ((c & 0x003F) << 10) + (((c & 0x03C0) + 0x0040) << 10);
          }
        }
        {
          if (c < 0x00080) {
            *z++ = (u8)(c & 0xFF);
          } else if (c < 0x00800) {
            *z++ = 0xC0 + (u8)((c >> 6) & 0x1F);
            *z++ = 0x80 + (u8)(c & 0x3F);
          } else if (c < 0x10000) {
            *z++ = 0xE0 + (u8)((c >> 12) & 0x0F);
            *z++ = 0x80 + (u8)((c >> 6) & 0x3F);
            *z++ = 0x80 + (u8)(c & 0x3F);
          } else {
            *z++ = 0xF0 + (u8)((c >> 18) & 0x07);
            *z++ = 0x80 + (u8)((c >> 12) & 0x3F);
            *z++ = 0x80 + (u8)((c >> 6) & 0x3F);
            *z++ = 0x80 + (u8)(c & 0x3F);
          }
        };
      }
    }
    pMem->n = (int)(z - zOut);
  }
  *z = 0;

  c = 0x0002 | 0x0200 | (pMem->flags & (0x003f | 0x0800));
  sqlite3VdbeMemRelease(pMem);
  pMem->flags = c;
  pMem->enc = desiredEnc;
  pMem->z = (char *)zOut;
  pMem->zMalloc = pMem->z;
  pMem->szMalloc = sqlite3DbMallocSize(pMem->db, pMem->z);

translate_out:
  return 0;
}

int sqlite3VdbeMemHandleBom(Mem *pMem) {
  int rc = SQLITE_OK;
  u8 bom = 0;

  if (pMem->n > 1) {
    u8 b1 = *(u8 *)pMem->z;
    u8 b2 = *(((u8 *)pMem->z) + 1);
    if (b1 == 0xFE && b2 == 0xFF) {
      bom = SQLITE_UTF16BE;
    }
    if (b1 == 0xFF && b2 == 0xFE) {
      bom = SQLITE_UTF16LE;
    }
  }

  if (bom) {
    rc = sqlite3VdbeMemMakeWriteable(pMem);
    if (rc == SQLITE_OK) {
      pMem->n -= 2;
      memmove(pMem->z, &pMem->z[2], pMem->n);
      pMem->z[pMem->n] = '\0';
      pMem->z[pMem->n + 1] = '\0';
      pMem->flags |= 0x0200;
      pMem->enc = bom;
    }
  }
  return rc;
}

int sqlite3VdbeChangeEncoding(Mem *pMem, int desiredEnc) {
  int rc;

  if (!(pMem->flags & 0x0002)) {
    pMem->enc = desiredEnc;
    return SQLITE_OK;
  }
  if (pMem->enc == desiredEnc) {
    return SQLITE_OK;
  }

  rc = sqlite3VdbeMemTranslate(pMem, (u8)desiredEnc);

  return rc;
}

__attribute__((noinline)) int sqlite3VdbeMemGrow(Mem *pMem, int n, int bPreserve) {
  if (pMem->szMalloc > 0 && bPreserve && pMem->z == pMem->zMalloc) {
    if (pMem->db) {
      pMem->z = pMem->zMalloc = sqlite3DbReallocOrFree(pMem->db, pMem->z, n);
    } else {
      pMem->zMalloc = sqlite3Realloc(pMem->z, n);
      if (pMem->zMalloc == 0)
        sqlite3_free(pMem->z);
      pMem->z = pMem->zMalloc;
    }
    bPreserve = 0;
  } else {
    if (pMem->szMalloc > 0)
      sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
    pMem->zMalloc = sqlite3DbMallocRaw(pMem->db, n);
  }
  if (pMem->zMalloc == 0) {
    sqlite3VdbeMemSetNull(pMem);
    pMem->z = 0;
    pMem->szMalloc = 0;
    return 7;
  } else {
    pMem->szMalloc = sqlite3DbMallocSize(pMem->db, pMem->zMalloc);
  }

  if (bPreserve && pMem->z) {
    memcpy(pMem->zMalloc, pMem->z, pMem->n);
  }
  if ((pMem->flags & 0x1000) != 0) {
    pMem->xDel((void *)(pMem->z));
  }

  pMem->z = pMem->zMalloc;
  pMem->flags &= ~(0x1000 | 0x4000 | 0x2000);
  return SQLITE_OK;
}

int sqlite3VdbeMemClearAndResize(Mem *pMem, int szNew) {
  if (pMem->szMalloc < szNew) {
    return sqlite3VdbeMemGrow(pMem, szNew, 0);
  }

  pMem->z = pMem->zMalloc;
  pMem->flags &= (0x0001 | 0x0004 | 0x0008 | 0x0020);
  return SQLITE_OK;
}

int sqlite3VdbeMemZeroTerminateIfAble(Mem *pMem) {
  if ((pMem->flags & (0x0002 | 0x0200 | 0x4000 | 0x2000)) != 0x0002) {
    return 0;
  }
  if (pMem->enc != SQLITE_UTF8)
    return 0;

  if (pMem->flags & 0x1000) {
    if (pMem->xDel == sqlite3_free && sqlite3_msize(pMem->z) >= (u64)(pMem->n + 1)) {
      pMem->z[pMem->n] = 0;
      pMem->flags |= 0x0200;
      return 1;
    }
    if (pMem->xDel == sqlite3RCStrUnref) {
      pMem->flags |= 0x0200;
      return 1;
    }
  } else if (pMem->szMalloc >= pMem->n + 1) {
    pMem->z[pMem->n] = 0;
    pMem->flags |= 0x0200;
    return 1;
  }
  return 0;
}

__attribute__((noinline)) int vdbeMemAddTerminator(Mem *pMem) {
  if (sqlite3VdbeMemGrow(pMem, pMem->n + 3, 1)) {
    return 7;
  }
  pMem->z[pMem->n] = 0;
  pMem->z[pMem->n + 1] = 0;
  pMem->z[pMem->n + 2] = 0;
  pMem->flags |= 0x0200;
  return SQLITE_OK;
}

int sqlite3VdbeMemMakeWriteable(Mem *pMem) {
  if ((pMem->flags & (0x0002 | 0x0010)) != 0) {
    if ((((pMem)->flags & 0x0400) ? sqlite3VdbeMemExpandBlob(pMem) : 0))
      return SQLITE_NOMEM;
    if (pMem->szMalloc == 0 || pMem->z != pMem->zMalloc) {
      int rc = vdbeMemAddTerminator(pMem);
      if (rc)
        return rc;
    }
  }
  pMem->flags &= ~0x4000;

  return 0;
}

int sqlite3VdbeMemExpandBlob(Mem *pMem) {
  int nByte;

  nByte = pMem->n + pMem->u.nZero;
  if (nByte <= 0) {
    if ((pMem->flags & 0x0010) == 0)
      return SQLITE_OK;
    nByte = 1;
  }
  if (sqlite3VdbeMemGrow(pMem, nByte, 1)) {
    return 7;
  }

  memset(&pMem->z[pMem->n], 0, pMem->u.nZero);
  pMem->n += pMem->u.nZero;
  pMem->flags &= ~(0x0400 | 0x0200);
  return SQLITE_OK;
}

int sqlite3VdbeMemNulTerminate(Mem *pMem) {
  if ((pMem->flags & (0x0200 | 0x0002)) != 0x0002) {
    return SQLITE_OK;
  } else {
    return vdbeMemAddTerminator(pMem);
  }
}

int sqlite3VdbeMemStringify(Mem *pMem, u8 enc, u8 bForce) {
  const int nByte = 32;

  if (sqlite3VdbeMemClearAndResize(pMem, nByte)) {
    pMem->enc = 0;
    return 7;
  }

  vdbeMemRenderNum(nByte, pMem->z, pMem);

  pMem->enc = SQLITE_UTF8;
  pMem->flags |= 0x0002 | 0x0200;
  if (bForce)
    pMem->flags &= ~(0x0004 | 0x0008 | 0x0020);
  sqlite3VdbeChangeEncoding(pMem, enc);
  return SQLITE_OK;
}

int sqlite3VdbeMemFinalize(Mem *pMem, FuncDef *pFunc) {
  sqlite3_context ctx;
  Mem t;

  memset(&ctx, 0, sizeof(ctx));
  memset(&t, 0, sizeof(t));
  t.flags = 0x0001;
  t.db = pMem->db;
  ctx.pOut = &t;
  ctx.pMem = pMem;
  ctx.pFunc = pFunc;
  ctx.enc = ((t.db)->enc);
  pFunc->xFinalize(&ctx);

  if (pMem->szMalloc > 0)
    sqlite3DbFreeNN(pMem->db, pMem->zMalloc);
  memcpy(pMem, &t, sizeof(t));
  return ctx.isError;
}

int sqlite3VdbeMemAggValue(Mem *pAccum, Mem *pOut, FuncDef *pFunc) {
  sqlite3_context ctx;

  memset(&ctx, 0, sizeof(ctx));
  sqlite3VdbeMemSetNull(pOut);
  ctx.pOut = pOut;
  ctx.pMem = pAccum;
  ctx.pFunc = pFunc;
  ctx.enc = ((pAccum->db)->enc);
  pFunc->xValue(&ctx);
  return ctx.isError;
}

__attribute__((noinline)) void vdbeMemClearExternAndSetNull(Mem *p) {
  if (p->flags & 0x8000) {
    sqlite3VdbeMemFinalize(p, p->u.pDef);
  }
  if (p->flags & 0x1000) {
    p->xDel((void *)p->z);
  }
  p->flags = 0x0001;
}

__attribute__((noinline)) void vdbeMemClear(Mem *p) {
  if ((((p)->flags & (0x8000 | 0x1000)) != 0)) {
    vdbeMemClearExternAndSetNull(p);
  }
  if (p->szMalloc) {
    sqlite3DbFreeNN(p->db, p->zMalloc);
    p->szMalloc = 0;
  }
  p->z = 0;
}

void sqlite3VdbeMemRelease(Mem *p) {
  if ((((p)->flags & (0x8000 | 0x1000)) != 0) || p->szMalloc) {
    vdbeMemClear(p);
  }
}

void sqlite3VdbeMemReleaseMalloc(Mem *p) {
  if (p->szMalloc)
    vdbeMemClear(p);
}

__attribute__((noinline)) i64 memIntValue(const Mem *pMem) {
  i64 value = 0;
  sqlite3Atoi64(pMem->z, &value, pMem->n, pMem->enc);
  return value;
}

i64 sqlite3VdbeIntValue(const Mem *pMem) {
  int flags;

  flags = pMem->flags;
  if (flags & (0x0004 | 0x0020)) {
    return pMem->u.i;
  } else if (flags & 0x0008) {
    return sqlite3RealToI64(pMem->u.r);
  } else if ((flags & (0x0002 | 0x0010)) != 0 && pMem->z != 0) {
    return memIntValue(pMem);
  } else {
    return 0;
  }
}

__attribute__((noinline)) int sqlite3MemRealValueRCSlowPath(Mem *pMem, double *pValue) {
  int rc = SQLITE_OK;
  *pValue = 0.0;
  if (pMem->enc == SQLITE_UTF8) {
    char *zCopy = sqlite3DbStrNDup(pMem->db, pMem->z, pMem->n);
    if (zCopy) {
      rc = sqlite3AtoF(zCopy, pValue);
      sqlite3DbFree(pMem->db, zCopy);
    }
    return rc;
  } else {
    int n, i, j;
    char *zCopy;
    const char *z;

    n = pMem->n & ~1;
    zCopy = sqlite3DbMallocRaw(pMem->db, n / 2 + 2);
    if (zCopy) {
      z = pMem->z;
      if (pMem->enc == SQLITE_UTF16LE) {
        for (i = j = 0; i < n - 1; i += 2, j++) {
          zCopy[j] = z[i];
          if (z[i + 1] != 0)
            break;
        }
      } else {
        for (i = j = 0; i < n - 1; i += 2, j++) {
          if (z[i] != 0)
            break;
          zCopy[j] = z[i + 1];
        }
      }

      zCopy[j] = 0;
      rc = sqlite3AtoF(zCopy, pValue);
      if (i < n)
        rc = -100;
      sqlite3DbFree(pMem->db, zCopy);
    }
    return rc;
  }
}

int sqlite3MemRealValueRC(Mem *pMem, double *pValue) {
  if (pMem->z == 0) {
    *pValue = 0.0;
    return 0;
  } else if (pMem->enc == SQLITE_UTF8 && ((pMem->flags & 0x0200) != 0 || sqlite3VdbeMemZeroTerminateIfAble(pMem))) {
    return sqlite3AtoF(pMem->z, pValue);
  } else if (pMem->n == 0) {
    *pValue = 0.0;
    return 0;
  } else {
    return sqlite3MemRealValueRCSlowPath(pMem, pValue);
  }
}

__attribute__((noinline)) double sqlite3MemRealValueNoRC(Mem *pMem) {
  double r;
  (void)sqlite3MemRealValueRC(pMem, &r);
  return r;
}

double sqlite3VdbeRealValue(Mem *pMem) {
  if (pMem->flags & 0x0008) {
    return pMem->u.r;
  } else if (pMem->flags & (0x0004 | 0x0020)) {
    return (double)pMem->u.i;
  } else if (pMem->flags & (0x0002 | 0x0010)) {
    return sqlite3MemRealValueNoRC(pMem);
  } else {
    return (double)0;
  }
}

int sqlite3VdbeBooleanValue(Mem *pMem, int ifNull) {
  if (pMem->flags & (0x0004 | 0x0020))
    return pMem->u.i != 0;
  if (pMem->flags & 0x0001)
    return ifNull;
  return sqlite3VdbeRealValue(pMem) != 0.0;
}

void sqlite3VdbeIntegerAffinity(Mem *pMem) {
  if (pMem->flags & 0x0020) {
    ((pMem)->flags = ((pMem)->flags & ~(0x0dbf | 0x0400)) | 0x0004);
  } else {
    i64 ix = sqlite3RealToI64(pMem->u.r);

    if (pMem->u.r == ix && ix > (((i64)-1) - (0xffffffff | (((i64)0x7fffffff) << 32))) &&
        ix < (0xffffffff | (((i64)0x7fffffff) << 32))) {
      pMem->u.i = ix;
      ((pMem)->flags = ((pMem)->flags & ~(0x0dbf | 0x0400)) | 0x0004);
    }
  }
}

int sqlite3VdbeMemIntegerify(Mem *pMem) {
  pMem->u.i = sqlite3VdbeIntValue(pMem);
  ((pMem)->flags = ((pMem)->flags & ~(0x0dbf | 0x0400)) | 0x0004);
  return SQLITE_OK;
}

int sqlite3VdbeMemRealify(Mem *pMem) {
  pMem->u.r = sqlite3VdbeRealValue(pMem);
  ((pMem)->flags = ((pMem)->flags & ~(0x0dbf | 0x0400)) | 0x0008);
  return SQLITE_OK;
}

int sqlite3VdbeMemNumerify(Mem *pMem) {
  if ((pMem->flags & (0x0004 | 0x0008 | 0x0020 | 0x0001)) == 0) {
    int rc;
    sqlite3_int64 ix;

    rc = sqlite3MemRealValueRC(pMem, &pMem->u.r);
    if (((rc & 2) == 0 && sqlite3Atoi64(pMem->z, &ix, pMem->n, pMem->enc) < 2) ||
        sqlite3RealSameAsInt(pMem->u.r, (ix = sqlite3RealToI64(pMem->u.r)))) {
      pMem->u.i = ix;
      ((pMem)->flags = ((pMem)->flags & ~(0x0dbf | 0x0400)) | 0x0004);
    } else {
      ((pMem)->flags = ((pMem)->flags & ~(0x0dbf | 0x0400)) | 0x0008);
    }
  }

  pMem->flags &= ~(0x0002 | 0x0010 | 0x0400);
  return SQLITE_OK;
}

int sqlite3VdbeMemCast(Mem *pMem, u8 aff, u8 encoding) {
  if (pMem->flags & 0x0001)
    return SQLITE_OK;
  switch (aff) {
    case 0x41: {
      if ((pMem->flags & 0x0010) == 0) {
        sqlite3ValueApplyAffinity(pMem, 0x42, encoding);

        if (pMem->flags & 0x0002)
          ((pMem)->flags = ((pMem)->flags & ~(0x0dbf | 0x0400)) | 0x0010);
      } else {
        pMem->flags &= ~(0x0dbf & ~0x0010);
      }
      break;
    }
    case 0x43: {
      sqlite3VdbeMemNumerify(pMem);
      break;
    }
    case 0x44: {
      sqlite3VdbeMemIntegerify(pMem);
      break;
    }
    case 0x45: {
      sqlite3VdbeMemRealify(pMem);
      break;
    }
    default: {
      int rc;

      pMem->flags |= (pMem->flags & 0x0010) >> 3;
      sqlite3ValueApplyAffinity(pMem, 0x42, encoding);

      pMem->flags &= ~(0x0004 | 0x0008 | 0x0020 | 0x0010 | 0x0400);
      if (encoding != SQLITE_UTF8)
        pMem->n &= ~1;
      rc = sqlite3VdbeChangeEncoding(pMem, encoding);
      if (rc)
        return rc;
      sqlite3VdbeMemZeroTerminateIfAble(pMem);
    }
  }
  return SQLITE_OK;
}

void sqlite3VdbeMemInit(Mem *pMem, sqlite3 *db, u16 flags) {
  pMem->flags = flags;
  pMem->db = db;
  pMem->szMalloc = 0;
}

void sqlite3VdbeMemSetNull(Mem *pMem) {
  if ((((pMem)->flags & (0x8000 | 0x1000)) != 0)) {
    vdbeMemClearExternAndSetNull(pMem);
  } else {
    pMem->flags = 0x0001;
  }
}

void sqlite3VdbeMemSetZeroBlob(Mem *pMem, int n) {
  sqlite3VdbeMemRelease(pMem);
  pMem->flags = 0x0010 | 0x0400;
  pMem->n = 0;
  if (n < 0)
    n = 0;
  pMem->u.nZero = n;
  pMem->enc = SQLITE_UTF8;
  pMem->z = 0;
}

__attribute__((noinline)) void vdbeReleaseAndSetInt64(Mem *pMem, i64 val) {
  sqlite3VdbeMemSetNull(pMem);
  pMem->u.i = val;
  pMem->flags = 0x0004;
}

void sqlite3VdbeMemSetInt64(Mem *pMem, i64 val) {
  if ((((pMem)->flags & (0x8000 | 0x1000)) != 0)) {
    vdbeReleaseAndSetInt64(pMem, val);
  } else {
    pMem->u.i = val;
    pMem->flags = 0x0004;
  }
}

void sqlite3VdbeMemSetPointer(Mem *pMem, void *pPtr, const char *zPType, void (*xDestructor)(void *)) {
  vdbeMemClear(pMem);
  pMem->u.zPType = zPType ? zPType : "";
  pMem->z = pPtr;
  pMem->flags = 0x0001 | 0x1000 | 0x0800 | 0x0200;
  pMem->eSubtype = 'p';
  pMem->xDel = xDestructor ? xDestructor : sqlite3NoopDestructor;
}

void sqlite3VdbeMemSetDouble(Mem *pMem, double val) {
  sqlite3VdbeMemSetNull(pMem);
  if (!sqlite3IsNaN(val)) {
    pMem->u.r = val;
    pMem->flags = 0x0008;
  }
}

int sqlite3VdbeMemSetRowSet(Mem *pMem) {
  sqlite3 *db = pMem->db;
  RowSet *p;

  sqlite3VdbeMemRelease(pMem);
  p = sqlite3RowSetInit(db);
  if (p == 0)
    return SQLITE_NOMEM;
  pMem->z = (char *)p;
  pMem->flags = 0x0010 | 0x1000;
  pMem->xDel = sqlite3RowSetDelete;
  return SQLITE_OK;
}

int sqlite3VdbeMemTooBig(Mem *p) {
  if (p->flags & (0x0002 | 0x0010)) {
    int n = p->n;
    if (p->flags & 0x0400) {
      n += p->u.nZero;
    }
    return n > p->db->aLimit[SQLITE_LIMIT_LENGTH];
  }
  return 0;
}

__attribute__((noinline)) void vdbeClrCopy(Mem *pTo, const Mem *pFrom, int eType) {
  vdbeMemClearExternAndSetNull(pTo);

  sqlite3VdbeMemShallowCopy(pTo, pFrom, eType);
}

void sqlite3VdbeMemShallowCopy(Mem *pTo, const Mem *pFrom, int srcType) {
  if ((((pTo)->flags & (0x8000 | 0x1000)) != 0)) {
    vdbeClrCopy(pTo, pFrom, srcType);
    return;
  }
  memcpy(pTo, pFrom, offsetof(Mem, db));
  if ((pFrom->flags & 0x2000) == 0) {
    pTo->flags &= ~(0x1000 | 0x2000 | 0x4000);

    pTo->flags |= srcType;
  }
}

int sqlite3VdbeMemCopy(Mem *pTo, const Mem *pFrom) {
  int rc = 0;

  if ((((pTo)->flags & (0x8000 | 0x1000)) != 0))
    vdbeMemClearExternAndSetNull(pTo);
  memcpy(pTo, pFrom, offsetof(Mem, db));
  pTo->flags &= ~0x1000;
  if (pTo->flags & (0x0002 | 0x0010)) {
    if (0 == (pFrom->flags & 0x2000)) {
      pTo->flags |= 0x4000;
      rc = sqlite3VdbeMemMakeWriteable(pTo);
    }
  }

  return rc;
}

void sqlite3VdbeMemMove(Mem *pTo, Mem *pFrom) {
  sqlite3VdbeMemRelease(pTo);
  memcpy(pTo, pFrom, sizeof(Mem));
  pFrom->flags = 0x0001;
  pFrom->szMalloc = 0;
}

int sqlite3VdbeMemSetStr(Mem *pMem, const char *z, i64 n, u8 enc, void (*xDel)(void *)) {
  i64 nByte = n;
  int iLimit;
  u16 flags;

  if (!z) {
    sqlite3VdbeMemSetNull(pMem);
    return SQLITE_OK;
  }

  if (pMem->db) {
    iLimit = pMem->db->aLimit[SQLITE_LIMIT_LENGTH];
  } else {
    iLimit = 1000000000;
  }
  if (nByte < 0) {
    if (enc == SQLITE_UTF8) {
      nByte = strlen(z);
    } else {
      for (nByte = 0; nByte <= iLimit && (z[nByte] | z[nByte + 1]); nByte += 2) {
      }
    }
    flags = 0x0002 | 0x0200;
  } else if (enc == 0) {
    flags = 0x0010;
    enc = SQLITE_UTF8;
  } else {
    flags = 0x0002;
  }
  if (nByte > iLimit) {
    if (xDel && xDel != ((sqlite3_destructor_type)-1)) {
      if (xDel == ((sqlite3_destructor_type)sqlite3RowSetClear)) {
        sqlite3DbFree(pMem->db, (void *)z);
      } else {
        xDel((void *)z);
      }
    }
    sqlite3VdbeMemSetNull(pMem);
    return sqlite3ErrorToParser(pMem->db, SQLITE_TOOBIG);
  }

  if (xDel == ((sqlite3_destructor_type)-1)) {
    i64 nAlloc = nByte;
    if (flags & 0x0200) {
      nAlloc += (enc == SQLITE_UTF8 ? 1 : 2);
    };
    if (sqlite3VdbeMemClearAndResize(pMem, (int)((nAlloc) > (32) ? (nAlloc) : (32)))) {
      return 7;
    }

    memcpy(pMem->z, z, nAlloc);
  } else {
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (char *)z;
    if (xDel == ((sqlite3_destructor_type)sqlite3RowSetClear)) {
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3DbMallocSize(pMem->db, pMem->zMalloc);
    } else {
      pMem->xDel = xDel;
      flags |= ((xDel == ((sqlite3_destructor_type)0)) ? 0x2000 : 0x1000);
    }
  }

  pMem->n = (int)(nByte & 0x7fffffff);
  pMem->flags = flags;
  pMem->enc = enc;

  if (enc > SQLITE_UTF8 && sqlite3VdbeMemHandleBom(pMem)) {
    return 7;
  }

  return SQLITE_OK;
}

int sqlite3VdbeMemSetText(Mem *pMem, const char *z, i64 n, void (*xDel)(void *)) {
  i64 nByte = n;
  u16 flags;

  if (!z) {
    sqlite3VdbeMemSetNull(pMem);
    return SQLITE_OK;
  }

  if (nByte < 0) {
    nByte = strlen(z);
    flags = 0x0002 | 0x0200;
  } else {
    flags = 0x0002;
  }
  if (nByte > (i64)pMem->db->aLimit[SQLITE_LIMIT_LENGTH]) {
    if (xDel && xDel != ((sqlite3_destructor_type)-1)) {
      if (xDel == ((sqlite3_destructor_type)sqlite3RowSetClear)) {
        sqlite3DbFree(pMem->db, (void *)z);
      } else {
        xDel((void *)z);
      }
    }
    sqlite3VdbeMemSetNull(pMem);
    return sqlite3ErrorToParser(pMem->db, SQLITE_TOOBIG);
  }

  if (xDel == ((sqlite3_destructor_type)-1)) {
    i64 nAlloc = nByte + 1;
    if (sqlite3VdbeMemClearAndResize(pMem, (int)((nAlloc) > (32) ? (nAlloc) : (32)))) {
      return 7;
    }

    memcpy(pMem->z, z, nByte);
    pMem->z[nByte] = 0;
  } else {
    sqlite3VdbeMemRelease(pMem);
    pMem->z = (char *)z;
    if (xDel == ((sqlite3_destructor_type)sqlite3RowSetClear)) {
      pMem->zMalloc = pMem->z;
      pMem->szMalloc = sqlite3DbMallocSize(pMem->db, pMem->zMalloc);
      pMem->xDel = 0;
    } else if (xDel == ((sqlite3_destructor_type)0)) {
      pMem->xDel = xDel;
      flags |= 0x2000;
    } else {
      pMem->xDel = xDel;
      flags |= 0x1000;
    }
  }
  pMem->flags = flags;
  pMem->n = (int)(nByte & 0x7fffffff);
  pMem->enc = SQLITE_UTF8;
  return SQLITE_OK;
}

void initMemArray(Mem *p, int N, sqlite3 *db, u16 flags) {
  if (N > 0) {
    do {
      p->flags = flags;
      p->db = db;
      p->szMalloc = 0;

      p++;
    } while ((--N) > 0);
  }
}

void releaseMemArray(Mem *p, int N) {
  if (p && N) {
    Mem *pEnd = &p[N];
    sqlite3 *db = p->db;

    if (db->pnBytesFreed) {
      do {
        if (p->szMalloc)
          sqlite3DbFree(db, p->zMalloc);
      } while ((++p) < pEnd);
      return;
    }
    do {
      if (p->flags & (0x8000 | 0x1000)) {
        sqlite3VdbeMemRelease(p);
        p->flags = 0x0000;
      } else if (p->szMalloc) {
        sqlite3DbNNFreeNN(db, p->zMalloc);
        p->szMalloc = 0;
        p->flags = 0x0000;
      }

    } while ((++p) < pEnd);
  }
}

__attribute__((noinline)) int vdbeCompareMemStringWithEncodingChange(const Mem *pMem1, const Mem *pMem2,
                                                                     const CollSeq *pColl, u8 *prcErr) {
  int rc;
  const void *v1, *v2;
  Mem c1;
  Mem c2;
  sqlite3VdbeMemInit(&c1, pMem1->db, 0x0001);
  sqlite3VdbeMemInit(&c2, pMem1->db, 0x0001);
  sqlite3VdbeMemShallowCopy(&c1, pMem1, 0x4000);
  sqlite3VdbeMemShallowCopy(&c2, pMem2, 0x4000);
  v1 = sqlite3ValueText((sqlite3_value *)&c1, pColl->enc);
  v2 = sqlite3ValueText((sqlite3_value *)&c2, pColl->enc);
  if ((v1 == 0 || v2 == 0)) {
    if (prcErr)
      *prcErr = 7;
    rc = 0;
  } else {
    rc = pColl->xCmp(pColl->pUser, c1.n, v1, c2.n, v2);
  }
  sqlite3VdbeMemReleaseMalloc(&c1);
  sqlite3VdbeMemReleaseMalloc(&c2);
  return rc;
}

int vdbeCompareMemString(const Mem *pMem1, const Mem *pMem2, const CollSeq *pColl, u8 *prcErr) {
  if (pMem1->enc == pColl->enc) {
    return pColl->xCmp(pColl->pUser, pMem1->n, pMem1->z, pMem2->n, pMem2->z);
  } else {
    return vdbeCompareMemStringWithEncodingChange(pMem1, pMem2, pColl, prcErr);
  }
}

__attribute__((noinline)) int sqlite3BlobCompare(const Mem *pB1, const Mem *pB2) {
  int c;
  int n1 = pB1->n;
  int n2 = pB2->n;

  if ((pB1->flags | pB2->flags) & 0x0400) {
    if (pB1->flags & pB2->flags & 0x0400) {
      return pB1->u.nZero - pB2->u.nZero;
    } else if (pB1->flags & 0x0400) {
      if (!isAllZero(pB2->z, pB2->n))
        return -1;
      return pB1->u.nZero - n2;
    } else {
      if (!isAllZero(pB1->z, pB1->n))
        return +1;
      return n1 - pB2->u.nZero;
    }
  }
  c = memcmp(pB1->z, pB2->z, n1 > n2 ? n2 : n1);
  if (c)
    return c;
  return n1 - n2;
}

int sqlite3MemCompare(const Mem *pMem1, const Mem *pMem2, const CollSeq *pColl) {
  int f1, f2;
  int combined_flags;

  f1 = pMem1->flags;
  f2 = pMem2->flags;
  combined_flags = f1 | f2;

  if (combined_flags & 0x0001) {
    return (f2 & 0x0001) - (f1 & 0x0001);
  }

  if (combined_flags & (0x0004 | 0x0008 | 0x0020)) {
    if ((f1 & f2 & (0x0004 | 0x0020)) != 0) {
      if (pMem1->u.i < pMem2->u.i)
        return -1;
      if (pMem1->u.i > pMem2->u.i)
        return +1;
      return 0;
    }
    if ((f1 & f2 & 0x0008) != 0) {
      if (pMem1->u.r < pMem2->u.r)
        return -1;
      if (pMem1->u.r > pMem2->u.r)
        return +1;
      return 0;
    }
    if ((f1 & (0x0004 | 0x0020)) != 0) {
      if ((f2 & 0x0008) != 0) {
        return sqlite3IntFloatCompare(pMem1->u.i, pMem2->u.r);
      } else if ((f2 & (0x0004 | 0x0020)) != 0) {
        if (pMem1->u.i < pMem2->u.i)
          return -1;
        if (pMem1->u.i > pMem2->u.i)
          return +1;
        return 0;
      } else {
        return -1;
      }
    }
    if ((f1 & 0x0008) != 0) {
      if ((f2 & (0x0004 | 0x0020)) != 0) {
        return -sqlite3IntFloatCompare(pMem2->u.i, pMem1->u.r);
      } else {
        return -1;
      }
    }
    return +1;
  }

  if (combined_flags & 0x0002) {
    if ((f1 & 0x0002) == 0) {
      return 1;
    }
    if ((f2 & 0x0002) == 0) {
      return -1;
    }

    if (pColl) {
      return vdbeCompareMemString(pMem1, pMem2, pColl, 0);
    }
  }

  return sqlite3BlobCompare(pMem1, pMem2);
}

int alsoAnInt(Mem *pRec, double rValue, i64 *piValue) {
  i64 iValue;
  iValue = sqlite3RealToI64(rValue);
  if (sqlite3RealSameAsInt(rValue, iValue)) {
    *piValue = iValue;
    return 1;
  }
  return 0 == sqlite3Atoi64(pRec->z, piValue, pRec->n, pRec->enc);
}

void applyNumericAffinity(Mem *pRec, int bTryForInt) {
  double rValue;
  int rc;

  rc = sqlite3MemRealValueRC(pRec, &rValue);
  if (rc <= 0)
    return;
  if ((rc & 2) == 0 && alsoAnInt(pRec, rValue, &pRec->u.i)) {
    pRec->flags |= 0x0004;
  } else {
    pRec->u.r = rValue;
    pRec->flags |= 0x0008;
    if (bTryForInt)
      sqlite3VdbeIntegerAffinity(pRec);
  }

  pRec->flags &= ~0x0002;
}

void applyAffinity(Mem *pRec, char affinity, u8 enc) {
  if (affinity >= 0x43) {
    if ((pRec->flags & 0x0004) == 0) {
      if ((pRec->flags & (0x0008 | 0x0020)) == 0) {
        if (pRec->flags & 0x0002)
          applyNumericAffinity(pRec, 1);
      } else if (affinity <= 0x45) {
        sqlite3VdbeIntegerAffinity(pRec);
      }
    }
  } else if (affinity == 0x42) {
    if (0 == (pRec->flags & 0x0002)) {
      if ((pRec->flags & (0x0008 | 0x0004 | 0x0020))) {
        sqlite3VdbeMemStringify(pRec, enc, 1);
      }
    }
    pRec->flags &= ~(0x0008 | 0x0004 | 0x0020);
  }
}

u16 __attribute__((noinline)) computeNumericType(Mem *pMem) {
  int rc;
  sqlite3_int64 ix;

  if ((((pMem)->flags & 0x0400) ? sqlite3VdbeMemExpandBlob(pMem) : 0)) {
    pMem->u.i = 0;
    return 0x0004;
  }
  rc = sqlite3MemRealValueRC(pMem, &pMem->u.r);
  if (rc <= 0) {
    if ((rc & 2) == 0 && sqlite3Atoi64(pMem->z, &ix, pMem->n, pMem->enc) <= 1) {
      pMem->u.i = ix;
      return 0x0004;
    } else {
      return 0x0008;
    }
  } else if ((rc & 2) == 0 && sqlite3Atoi64(pMem->z, &ix, pMem->n, pMem->enc) == 0) {
    pMem->u.i = ix;
    return 0x0004;
  }
  return 0x0008;
}

u16 numericType(Mem *pMem) {
  if (pMem->flags & (0x0004 | 0x0008 | 0x0020 | 0x0001)) {
    return pMem->flags & (0x0004 | 0x0008 | 0x0020 | 0x0001);
  }

  return computeNumericType(pMem);
  return 0;
}

__attribute__((noinline)) Mem *out2PrereleaseWithClear(Mem *pOut) {
  sqlite3VdbeMemSetNull(pOut);
  pOut->flags = 0x0004;
  return pOut;
}

u64 filterHash(const Mem *aMem, const Op *pOp) {
  int i, mx;
  u64 h = 0;

  for (i = pOp->p3, mx = i + pOp->p4.i; i < mx; i++) {
    const Mem *p = &aMem[i];
    if (p->flags & (0x0004 | 0x0020)) {
      h += p->u.i;
    } else if (p->flags & 0x0008) {
      h += sqlite3VdbeIntValue(p);
    } else if (p->flags & (0x0002 | 0x0010)) {
      h += 4093 + (p->flags & (0x0002 | 0x0010));
    }
  }
  return h;
}

const char *vdbeMemTypeName(Mem *pMem) {
  static const char *azTypes[] = {"INT", "REAL", "TEXT", "BLOB", "NULL"};
  return azTypes[sqlite3_value_type(pMem) - 1];
}