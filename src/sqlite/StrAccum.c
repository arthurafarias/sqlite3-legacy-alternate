#define _GNU_SOURCE 1

#include <stdio.h>
#include <string.h>

#include "sqlite/StrAccum.h"

#include "sqlite/Index.h"
#include "sqlite/WhereLoop.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_str.h"
#include "sqlite/sqlite3_value.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
const char hexdigits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};


void sqlite3StrAccumSetError(StrAccum *p, u8 eError) {

  p->accError = eError;
  if (p->mxAlloc)
    sqlite3_str_reset(p);
  if (eError == 18)
    sqlite3ErrorToParser(p->db, eError);
}

int sqlite3StrAccumEnlarge(StrAccum *p, i64 N) {
  char *zNew;

  if (p->accError) {
    ;
    ;
    return 0;
  }
  if (p->mxAlloc == 0) {
    sqlite3StrAccumSetError(p, 18);
    return p->nAlloc - p->nChar - 1;
  } else {
    char *zOld = (((p)->printfFlags & 0x04) != 0) ? p->zText : 0;
    i64 szNew = p->nChar + N + 1;
    if (szNew + p->nChar <= p->mxAlloc) {

      szNew += p->nChar;
    }
    if (szNew > p->mxAlloc) {
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, 18);
      return 0;
    } else {
      p->nAlloc = (int)szNew;
    }
    if (p->db) {
      zNew = sqlite3DbRealloc(p->db, zOld, p->nAlloc);
    } else {
      zNew = sqlite3Realloc(zOld, p->nAlloc);
    }
    if (zNew) {

      ((void)(0))

          ;
      if (!(((p)->printfFlags & 0x04) != 0) && p->nChar > 0)
        memcpy(zNew, p->zText, p->nChar);
      p->zText = zNew;
      p->nAlloc = sqlite3DbMallocSize(p->db, zNew);
      p->printfFlags |= 0x04;
    } else {
      sqlite3_str_reset(p);
      sqlite3StrAccumSetError(p, 7);
      return 0;
    }
  }

  return (int)N;
}

int sqlite3StrAccumEnlargeIfNeeded(StrAccum *p, i64 N) {
  if (N + p->nChar >= p->nAlloc) {
    sqlite3StrAccumEnlarge(p, N);
  }
  return p->accError;
}

void __attribute__((noinline)) enlargeAndAppend(StrAccum *p, const char *z, int N) {
  N = sqlite3StrAccumEnlarge(p, N);
  if (N > 0) {
    memcpy(&p->zText[p->nChar], z, N);
    p->nChar += N;
  }
}

__attribute__((noinline)) char *strAccumFinishRealloc(StrAccum *p) {
  char *zText;

  zText = sqlite3DbMallocRaw(p->db, 1 + (u64)p->nChar);
  if (zText) {
    memcpy(zText, p->zText, p->nChar + 1);
    p->printfFlags |= 0x04;
  } else {
    sqlite3StrAccumSetError(p, 7);
  }
  p->zText = zText;
  return zText;
}

char *sqlite3StrAccumFinish(StrAccum *p) {
  if (p->zText) {
    p->zText[p->nChar] = 0;
    if (p->mxAlloc > 0 && !(((p)->printfFlags & 0x04) != 0)) {
      return strAccumFinishRealloc(p);
    }
  }
  return p->zText;
}

void sqlite3_str_reset(StrAccum *p) {
  if ((((p)->printfFlags & 0x04) != 0)) {
    sqlite3DbFree(p->db, p->zText);
    p->printfFlags &= ~0x04;
  }
  p->nAlloc = 0;
  p->nChar = 0;
  p->zText = 0;
}

void sqlite3StrAccumInit(StrAccum *p, sqlite3 *db, char *zBase, int n, int mx) {
  p->zText = zBase;
  p->db = db;
  p->nAlloc = n;
  p->mxAlloc = mx;
  p->nChar = 0;
  p->accError = 0;
  p->printfFlags = 0;
}

void sqlite3_str_appendf(StrAccum *p, const char *zFormat, ...) {
  va_list ap;

  va_start(

      ap, zFormat

  )

      ;
  sqlite3_str_vappendf(p, zFormat, ap);

  va_end(

      ap

  )

      ;
}

void sqlite3QuoteValue(StrAccum *pStr, sqlite3_value *pValue, int bEscape) {

  switch (sqlite3_value_type(pValue)) {
  case 2: {

    sqlite3_str_appendf(pStr, "%!0.17g", sqlite3_value_double(pValue));
    break;
  }
  case 1: {
    sqlite3_str_appendf(pStr, "%lld", sqlite3_value_int64(pValue));
    break;
  }
  case 4: {
    char const *zBlob = sqlite3_value_blob(pValue);
    i64 nBlob = sqlite3_value_bytes(pValue);

    ((void)(0))

        ;
    sqlite3StrAccumEnlarge(pStr, nBlob * 2 + 4);
    if (pStr->accError == 0) {
      char *zText = pStr->zText;
      int i;
      for (i = 0; i < nBlob; i++) {
        zText[(i * 2) + 2] = hexdigits[(zBlob[i] >> 4) & 0x0F];
        zText[(i * 2) + 3] = hexdigits[(zBlob[i]) & 0x0F];
      }
      zText[(nBlob * 2) + 2] = '\'';
      zText[(nBlob * 2) + 3] = '\0';
      zText[0] = 'X';
      zText[1] = '\'';
      pStr->nChar = nBlob * 2 + 3;
    }
    break;
  }
  case 3: {
    const unsigned char *zArg = sqlite3_value_text(pValue);
    sqlite3_str_appendf(pStr, bEscape ? "%#Q" : "%Q", zArg);
    break;
  }
  default: {

    ((void)(0))

        ;
    sqlite3_str_append(pStr, "NULL", 4);
    break;
  }
  }
}

void explainAppendTerm(StrAccum *pStr, Index *pIdx, int nTerm, int iTerm, int bAnd, const char *zOp) {
  int i;

  if (bAnd)
    sqlite3_str_append(pStr, " AND ", 5);

  if (nTerm > 1)
    sqlite3_str_append(pStr, "(", 1);
  for (i = 0; i < nTerm; i++) {
    if (i)
      sqlite3_str_append(pStr, ",", 1);
    sqlite3_str_appendall(pStr, explainIndexColumnName(pIdx, iTerm + i));
  }
  if (nTerm > 1)
    sqlite3_str_append(pStr, ")", 1);

  sqlite3_str_append(pStr, zOp, 1);

  if (nTerm > 1)
    sqlite3_str_append(pStr, "(", 1);
  for (i = 0; i < nTerm; i++) {
    if (i)
      sqlite3_str_append(pStr, ",", 1);
    sqlite3_str_append(pStr, "?", 1);
  }
  if (nTerm > 1)
    sqlite3_str_append(pStr, ")", 1);
}

void explainIndexRange(StrAccum *pStr, WhereLoop *pLoop) {
  Index *pIndex = pLoop->u.btree.pIndex;
  u16 nEq = pLoop->u.btree.nEq;
  u16 nSkip = pLoop->nSkip;
  int i, j;

  if (nEq == 0 && (pLoop->wsFlags & (0x00000020 | 0x00000010)) == 0)
    return;
  sqlite3_str_append(pStr, " (", 2);
  for (i = 0; i < nEq; i++) {
    const char *z = explainIndexColumnName(pIndex, i);
    if (i)
      sqlite3_str_append(pStr, " AND ", 5);
    sqlite3_str_appendf(pStr, i >= nSkip ? "%s=?" : "ANY(%s)", z);
  }

  j = i;
  if (pLoop->wsFlags & 0x00000020) {
    explainAppendTerm(pStr, pIndex, pLoop->u.btree.nBtm, j, i, ">");
    i = 1;
  }
  if (pLoop->wsFlags & 0x00000010) {
    explainAppendTerm(pStr, pIndex, pLoop->u.btree.nTop, j, i, "<");
  }
  sqlite3_str_append(pStr, ")", 1);
}