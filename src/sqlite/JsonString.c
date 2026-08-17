#define _GNU_SOURCE 1

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "sqlite/JsonString.h"

#include "sqlite/JsonParse.h"
#include "sqlite/RCStr.h"
#include "sqlite/RowSet.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_context.h"
#include "sqlite/sqlite3_destructor_type.h"
#include "sqlite/sqlite3_value.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
void jsonStringZero(JsonString *p) {
  p->zBuf = p->zSpace;
  p->nAlloc = sizeof(p->zSpace);
  p->nUsed = 0;
  p->bStatic = 1;
}

void jsonStringInit(JsonString *p, sqlite3_context *pCtx) {
  p->pCtx = pCtx;
  p->eErr = 0;
  jsonStringZero(p);
}

void jsonStringReset(JsonString *p) {
  if (!p->bStatic)
    sqlite3RCStrUnref(p->zBuf);
  jsonStringZero(p);
}

void jsonStringOom(JsonString *p) {
  p->eErr |= 0x01;
  if (p->pCtx)
    sqlite3_result_error_nomem(p->pCtx);
  jsonStringReset(p);
}

void jsonStringTooDeep(JsonString *p) {
  p->eErr |= 0x04;

  sqlite3_result_error(p->pCtx, "JSON nested too deep", -1);
  jsonStringReset(p);
}

int jsonStringGrow(JsonString *p, u32 N) {
  u64 nTotal = N < p->nAlloc ? p->nAlloc * 2 : p->nAlloc + N + 10;
  char *zNew;
  if (p->bStatic) {
    if (p->eErr)
      return 1;
    zNew = sqlite3RCStrNew(nTotal);
    if (zNew == 0) {
      jsonStringOom(p);
      return 7;
    }
    memcpy(zNew, p->zBuf, (size_t)p->nUsed);
    p->zBuf = zNew;
    p->bStatic = 0;
  } else {
    p->zBuf = sqlite3RCStrResize(p->zBuf, nTotal);
    if (p->zBuf == 0) {
      p->eErr |= 0x01;
      jsonStringZero(p);
      return 7;
    }
  }
  p->nAlloc = nTotal;
  return 0;
}

__attribute__((noinline)) void jsonStringExpandAndAppend(JsonString *p, const char *zIn, u32 N) {

  if (jsonStringGrow(p, N))
    return;
  memcpy(p->zBuf + p->nUsed, zIn, N);
  p->nUsed += N;
}

void jsonAppendRaw(JsonString *p, const char *zIn, u32 N) {
  if (N == 0)
    return;
  if (N + p->nUsed >= p->nAlloc) {
    jsonStringExpandAndAppend(p, zIn, N);
  } else {
    memcpy(p->zBuf + p->nUsed, zIn, N);
    p->nUsed += N;
  }
}

void jsonAppendRawNZ(JsonString *p, const char *zIn, u32 N) {

  if (N + p->nUsed >= p->nAlloc) {
    jsonStringExpandAndAppend(p, zIn, N);
  } else {
    memcpy(p->zBuf + p->nUsed, zIn, N);
    p->nUsed += N;
  }
}

__attribute__((noinline)) void jsonAppendCharExpand(JsonString *p, char c) {
  if (jsonStringGrow(p, 1))
    return;
  p->zBuf[p->nUsed++] = c;
}

void jsonAppendChar(JsonString *p, char c) {
  if (p->nUsed >= p->nAlloc) {
    jsonAppendCharExpand(p, c);
  } else {
    p->zBuf[p->nUsed++] = c;
  }
}

void jsonStringTrimOneChar(JsonString *p) {
  if (p->eErr == 0) {

    ((void)(0))

        ;
    p->nUsed--;
  }
}

int jsonStringTerminate(JsonString *p) {
  jsonAppendChar(p, 0);
  jsonStringTrimOneChar(p);
  return p->eErr == 0;
}

void jsonAppendSeparator(JsonString *p) {
  char c;
  if (p->nUsed == 0)
    return;
  c = p->zBuf[p->nUsed - 1];
  if (c == '[' || c == '{')
    return;
  jsonAppendChar(p, ',');
}

void jsonAppendControlChar(JsonString *p, u8 c) {
  static const char aSpecial[] = {0, 0, 0, 0, 0, 0, 0, 0, 'b', 't', 'n', 0, 'f', 'r', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  if (aSpecial[c]) {
    p->zBuf[p->nUsed] = '\\';
    p->zBuf[p->nUsed + 1] = aSpecial[c];
    p->nUsed += 2;
  } else {
    p->zBuf[p->nUsed] = '\\';
    p->zBuf[p->nUsed + 1] = 'u';
    p->zBuf[p->nUsed + 2] = '0';
    p->zBuf[p->nUsed + 3] = '0';
    p->zBuf[p->nUsed + 4] = "0123456789abcdef"[c >> 4];
    p->zBuf[p->nUsed + 5] = "0123456789abcdef"[c & 0xf];
    p->nUsed += 6;
  }
}

void jsonAppendString(JsonString *p, const char *zIn, u32 N) {
  u32 k;
  u8 c;
  const u8 *z = (const u8 *)zIn;
  if (z == 0)
    return;
  if ((N + p->nUsed + 2 >= p->nAlloc) && jsonStringGrow(p, N + 2) != 0)
    return;
  p->zBuf[p->nUsed++] = '"';
  while (1) {
    k = 0;

    while (1) {
      if (k + 3 >= N) {
        while (k < N && jsonIsOk[z[k]]) {
          k++;
        }
        break;
      }
      if (!jsonIsOk[z[k]]) {
        break;
      }
      if (!jsonIsOk[z[k + 1]]) {
        k += 1;
        break;
      }
      if (!jsonIsOk[z[k + 2]]) {
        k += 2;
        break;
      }
      if (!jsonIsOk[z[k + 3]]) {
        k += 3;
        break;
      } else {
        k += 4;
      }
    }
    if (k >= N) {
      if (k > 0) {
        memcpy(&p->zBuf[p->nUsed], z, k);
        p->nUsed += k;
      }
      break;
    }
    if (k > 0) {
      memcpy(&p->zBuf[p->nUsed], z, k);
      p->nUsed += k;
      z += k;
      N -= k;
    }
    c = z[0];
    if (c == '"' || c == '\\') {
      if ((p->nUsed + N + 3 > p->nAlloc) && jsonStringGrow(p, N + 3) != 0)
        return;
      p->zBuf[p->nUsed++] = '\\';
      p->zBuf[p->nUsed++] = c;
    } else if (c == '\'') {
      p->zBuf[p->nUsed++] = c;
    } else {
      if ((p->nUsed + N + 7 > p->nAlloc) && jsonStringGrow(p, N + 7) != 0)
        return;
      jsonAppendControlChar(p, c);
    }
    z++;
    N--;
  }
  p->zBuf[p->nUsed++] = '"';
}

void jsonAppendSqlValue(JsonString *p, sqlite3_value *pValue) {
  switch (sqlite3_value_type(pValue)) {
  case 5: {
    jsonAppendRawNZ(p, "null", 4);
    break;
  }
  case 2: {
    jsonPrintf(100, p, "%!0.17g", sqlite3_value_double(pValue));
    break;
  }
  case 1: {
    const char *z = (const char *)sqlite3_value_text(pValue);
    u32 n = (u32)sqlite3_value_bytes(pValue);
    jsonAppendRaw(p, z, n);
    break;
  }
  case 3: {
    const char *z = (const char *)sqlite3_value_text(pValue);
    u32 n = (u32)sqlite3_value_bytes(pValue);
    if (sqlite3_value_subtype(pValue) == 74) {
      jsonAppendRaw(p, z, n);
    } else {
      jsonAppendString(p, z, n);
    }
    break;
  }
  default: {
    JsonParse px;
    memset(&px, 0, sizeof(px));
    if (jsonArgIsJsonb(pValue, &px)) {
      jsonTranslateBlobToText(&px, 0, p);
    } else if (p->eErr == 0) {
      sqlite3_result_error(p->pCtx, "JSON cannot hold BLOB values", -1);
      p->eErr = 0x08;
      jsonStringReset(p);
    }
    break;
  }
  }
}

void jsonReturnString(JsonString *p, JsonParse *pParse, sqlite3_context *ctx) {

  jsonStringTerminate(p);
  if (p->eErr == 0) {
    int flags = ((int)(intptr_t)(sqlite3_user_data(p->pCtx)));
    if (flags & 0x10) {
      jsonReturnStringAsBlob(p);
    } else if (p->bStatic) {
      sqlite3_result_text64(p->pCtx, p->zBuf, p->nUsed, ((sqlite3_destructor_type)-1), 1);
    } else {
      if (pParse && pParse->bJsonIsRCStr == 0 && pParse->nBlobAlloc > 0) {
        int rc;
        pParse->zJson = sqlite3RCStrRef(p->zBuf);
        pParse->nJson = p->nUsed;
        pParse->bJsonIsRCStr = 1;
        rc = jsonCacheInsert(ctx, pParse);
        if (rc == 7) {
          sqlite3_result_error_nomem(ctx);
          jsonStringReset(p);
          return;
        }
      }
      sqlite3_result_text64(p->pCtx, sqlite3RCStrRef(p->zBuf), p->nUsed, sqlite3RCStrUnref, 1);
    }
  } else if (p->eErr & 0x01) {
    sqlite3_result_error_nomem(p->pCtx);
  } else if (p->eErr & 0x04) {

  } else if (p->eErr & 0x02) {
    sqlite3_result_error(p->pCtx, "malformed JSON", -1);
  }
  jsonStringReset(p);
}

void jsonReturnStringAsBlob(JsonString *pStr) {
  JsonParse px;

  memset(&px, 0, sizeof(px));
  px.zJson = pStr->zBuf;
  px.nJson = pStr->nUsed;
  px.db = sqlite3_context_db_handle(pStr->pCtx);
  (void)jsonTranslateTextToBlob(&px, 0);
  if (px.oom) {
    sqlite3DbFree(px.db, px.aBlob);
    sqlite3_result_error_nomem(pStr->pCtx);
  } else {

    ((void)(0))

        ;

    ((void)(0))

        ;
    sqlite3_result_blob(pStr->pCtx, px.aBlob, px.nBlob, ((sqlite3_destructor_type)sqlite3RowSetClear));
  }
}
