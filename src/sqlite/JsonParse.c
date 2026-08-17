#define _GNU_SOURCE 1
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sqlite/JsonParse.h"
#include "sqlite/JsonString.h"
#include "sqlite/NanInfName.h"
#include "sqlite/RCStr.h"
#include "sqlite/RowSet.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_context.h"
#include "sqlite/sqlite3_destructor_type.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_uint64.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
/* Private helpers, formerly declared in _Uncategorized.h. */
static int json5Whitespace(const char *zIn);
static int jsonBlobOverwrite(u8 *aOut, const u8 *aIns, u32 nIns, u32 d);
static u32 jsonBytesToBypass(const char *z, u32 n);
static u8 jsonHexToInt(int h);
static u32 jsonHexToInt4(const char *z);
static int jsonIs2Hex(const char *z);
static int jsonIs4Hex(const char *z);
static int jsonIs4HexB(const char *z, int *pOp);
static int jsonLabelCompare(const char *zLeft, u32 nLeft, int rawLeft, const char *zRight, u32 nRight, int rawRight);
static __attribute__((noinline)) int jsonLabelCompareEscaped(const char *zLeft, u32 nLeft, int rawLeft,
                                                             const char *zRight, u32 nRight, int rawRight);
static u32 jsonUnescapeOneChar(const char *z, u32 n, u32 *piOut);

const char *const jsonbType[] = {"null", "true", "false", "integer", "integer", "real", "real", "text", "text",
                                 "text", "text", "array", "object",  "",        "",     "",     ""};

static const char jsonIsSpace[] = {

    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const char jsonSpaces[] = "\011\012\015\040";

const char jsonIsOk[256] = {

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1,
    1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

void jsonPrintf(int N, JsonString *p, const char *zFormat, ...) {
  va_list ap;
  if ((p->nUsed + N >= p->nAlloc) && jsonStringGrow(p, N))
    return;

  va_start(ap, zFormat);
  sqlite3_vsnprintf(N, p->zBuf + p->nUsed, zFormat, ap);

  va_end(ap);
  p->nUsed += (int)strlen(p->zBuf + p->nUsed);
}

static u8 jsonHexToInt(int h) {
  h += 9 * (1 & (h >> 6));

  return (u8)(h & 0xf);
}

static u32 jsonHexToInt4(const char *z) {
  u32 v;
  v = (jsonHexToInt(z[0]) << 12) + (jsonHexToInt(z[1]) << 8) + (jsonHexToInt(z[2]) << 4) + jsonHexToInt(z[3]);
  return v;
}

static int jsonIs2Hex(const char *z) {
  return (sqlite3CtypeMap[(unsigned char)(z[0])] & 0x08) && (sqlite3CtypeMap[(unsigned char)(z[1])] & 0x08);
}

static int jsonIs4Hex(const char *z) {
  return jsonIs2Hex(z) && jsonIs2Hex(&z[2]);
}

static int json5Whitespace(const char *zIn) {
  int n = 0;
  const u8 *z = (u8 *)zIn;
  while (1) {
    switch (z[n]) {
      case 0x09:
      case 0x0a:
      case 0x0b:
      case 0x0c:
      case 0x0d:
      case 0x20: {
        n++;
        break;
      }
      case '/': {
        if (z[n + 1] == '*' && z[n + 2] != 0) {
          int j;
          for (j = n + 3; z[j] != '/' || z[j - 1] != '*'; j++) {
            if (z[j] == 0)
              goto whitespace_done;
          }
          n = j + 1;
          break;
        } else if (z[n + 1] == '/') {
          int j;
          char c;
          for (j = n + 2; (c = z[j]) != 0; j++) {
            if (c == '\n' || c == '\r')
              break;
            if (0xe2 == (u8)c && 0x80 == (u8)z[j + 1] && (0xa8 == (u8)z[j + 2] || 0xa9 == (u8)z[j + 2])) {
              j += 2;
              break;
            }
          }
          n = j;
          if (z[n])
            n++;
          break;
        }
        goto whitespace_done;
      }
      case 0xc2: {
        if (z[n + 1] == 0xa0) {
          n += 2;
          break;
        }
        goto whitespace_done;
      }
      case 0xe1: {
        if (z[n + 1] == 0x9a && z[n + 2] == 0x80) {
          n += 3;
          break;
        }
        goto whitespace_done;
      }
      case 0xe2: {
        if (z[n + 1] == 0x80) {
          u8 c = z[n + 2];
          if (c < 0x80)
            goto whitespace_done;
          if (c <= 0x8a || c == 0xa8 || c == 0xa9 || c == 0xaf) {
            n += 3;
            break;
          }
        } else if (z[n + 1] == 0x81 && z[n + 2] == 0x9f) {
          n += 3;
          break;
        }
        goto whitespace_done;
      }
      case 0xe3: {
        if (z[n + 1] == 0x80 && z[n + 2] == 0x80) {
          n += 3;
          break;
        }
        goto whitespace_done;
      }
      case 0xef: {
        if (z[n + 1] == 0xbb && z[n + 2] == 0xbf) {
          n += 3;
          break;
        }
        goto whitespace_done;
      }
      default: {
        goto whitespace_done;
      }
    }
  }
whitespace_done:
  return n;
}

static const struct NanInfName {
  char c1;
  char c2;
  char n;
  char eType;
  char nRepl;
  char *zMatch;
  char *zRepl;
} aNanInfName[] = {
    {'i', 'I', 3, 5, 7, "inf", "9.0e999"}, {'i', 'I', 8, 5, 7, "infinity", "9.0e999"},
    {'n', 'N', 3, 0, 4, "NaN", "null"},    {'q', 'Q', 4, 0, 4, "QNaN", "null"},
    {'s', 'S', 4, 0, 4, "SNaN", "null"},
};

static int jsonIs4HexB(const char *z, int *pOp) {
  if (z[0] != 'u')
    return 0;
  if (!jsonIs4Hex(&z[1]))
    return 0;
  *pOp = 8;
  return 1;
}

static int jsonBlobOverwrite(u8 *aOut, const u8 *aIns, u32 nIns, u32 d) {
  u32 szPayload;
  u32 i;
  u8 szHdr;

  static const u8 aType[] = {0xc0, 0xd0, 0, 0xe0, 0, 0, 0, 0xf0};

  if ((aIns[0] & 0x0f) <= 2)
    return 0;
  switch (aIns[0] >> 4) {
    default: {
      if (((1 << d) & 0x116) == 0)
        return 0;
      i = d + 1;
      szHdr = 1;
      break;
    }
    case 12: {
      if (((1 << d) & 0x8a) == 0)
        return 0;
      i = d + 2;
      szHdr = 2;
      break;
    }
    case 13: {
      if (d != 2 && d != 6)
        return 0;
      i = d + 3;
      szHdr = 3;
      break;
    }
    case 14: {
      if (d != 4)
        return 0;
      i = 9;
      szHdr = 5;
      break;
    }
    case 15: {
      return 0;
    }
  }

  aOut[0] = (aIns[0] & 0x0f) | aType[i - 2];
  memcpy(&aOut[i], &aIns[szHdr], nIns - szHdr);
  szPayload = nIns - szHdr;
  while (1) {
    i--;
    aOut[i] = szPayload & 0xff;
    if (i == 1)
      break;
    szPayload >>= 8;
  }

  return 1;
}

static u32 jsonBytesToBypass(const char *z, u32 n) {
  u32 i = 0;
  while (i + 1 < n) {
    if (z[i] != '\\')
      return i;
    if (z[i + 1] == '\n') {
      i += 2;
      continue;
    }
    if (z[i + 1] == '\r') {
      if (i + 2 < n && z[i + 2] == '\n') {
        i += 3;
      } else {
        i += 2;
      }
      continue;
    }
    if (0xe2 == (u8)z[i + 1] && i + 3 < n && 0x80 == (u8)z[i + 2] && (0xa8 == (u8)z[i + 3] || 0xa9 == (u8)z[i + 3])) {
      i += 4;
      continue;
    }
    break;
  }
  return i;
}

static u32 jsonUnescapeOneChar(const char *z, u32 n, u32 *piOut) {
  if (n < 2) {
    *piOut = 0x99999;
    return n;
  }
  switch ((u8)z[1]) {
    case 'u': {
      u32 v, vlo;
      if (n < 6) {
        *piOut = 0x99999;
        return n;
      }
      v = jsonHexToInt4(&z[2]);
      if ((v & 0xfc00) == 0xd800 && n >= 12 && z[6] == '\\' && z[7] == 'u' &&
          ((vlo = jsonHexToInt4(&z[8])) & 0xfc00) == 0xdc00) {
        *piOut = ((v & 0x3ff) << 10) + (vlo & 0x3ff) + 0x10000;
        return 12;
      } else {
        *piOut = v;
        return 6;
      }
    }
    case 'b': {
      *piOut = '\b';
      return 2;
    }
    case 'f': {
      *piOut = '\f';
      return 2;
    }
    case 'n': {
      *piOut = '\n';
      return 2;
    }
    case 'r': {
      *piOut = '\r';
      return 2;
    }
    case 't': {
      *piOut = '\t';
      return 2;
    }
    case 'v': {
      *piOut = '\v';
      return 2;
    }
    case '0': {
      *piOut = (n > 2 && (sqlite3CtypeMap[(unsigned char)(z[2])] & 0x04)) ? 0x99999 : 0;

      return 2;
    }
    case '\'':
    case '"':
    case '/':
    case '\\': {
      *piOut = z[1];
      return 2;
    }
    case 'x': {
      if (n < 4) {
        *piOut = 0x99999;
        return n;
      }
      *piOut = (jsonHexToInt(z[2]) << 4) | jsonHexToInt(z[3]);
      return 4;
    }
    case 0xe2:
    case '\r':
    case '\n': {
      u32 nSkip = jsonBytesToBypass(z, n);
      if (nSkip == 0) {
        *piOut = 0x99999;
        return n;
      } else if (nSkip == n) {
        *piOut = 0;
        return n;
      } else if (z[nSkip] == '\\') {
        return nSkip + jsonUnescapeOneChar(&z[nSkip], n - nSkip, piOut);
      } else {
        int sz = sqlite3Utf8ReadLimited((u8 *)&z[nSkip], n - nSkip, piOut);
        return nSkip + sz;
      }
    }
    default: {
      *piOut = 0x99999;
      return 2;
    }
  }
}

static __attribute__((noinline)) int jsonLabelCompareEscaped(const char *zLeft, u32 nLeft, int rawLeft,
                                                             const char *zRight, u32 nRight, int rawRight) {
  u32 cLeft, cRight;

  while (1) {
    if (nLeft == 0) {
      cLeft = 0;
    } else if (rawLeft || zLeft[0] != '\\') {
      cLeft = ((u8 *)zLeft)[0];
      if (cLeft >= 0xc0) {
        int sz = sqlite3Utf8ReadLimited((u8 *)zLeft, nLeft, &cLeft);
        zLeft += sz;
        nLeft -= sz;
      } else {
        zLeft++;
        nLeft--;
      }
    } else {
      u32 n = jsonUnescapeOneChar(zLeft, nLeft, &cLeft);
      zLeft += n;

      nLeft -= n;
    }
    if (nRight == 0) {
      cRight = 0;
    } else if (rawRight || zRight[0] != '\\') {
      cRight = ((u8 *)zRight)[0];
      if (cRight >= 0xc0) {
        int sz = sqlite3Utf8ReadLimited((u8 *)zRight, nRight, &cRight);
        zRight += sz;
        nRight -= sz;
      } else {
        zRight++;
        nRight--;
      }
    } else {
      u32 n = jsonUnescapeOneChar(zRight, nRight, &cRight);
      zRight += n;

      nRight -= n;
    }
    if (cLeft != cRight)
      return 0;
    if (cLeft == 0)
      return 1;
  }
}

static int jsonLabelCompare(const char *zLeft, u32 nLeft, int rawLeft, const char *zRight, u32 nRight, int rawRight) {
  if (rawLeft && rawRight) {
    if (nLeft != nRight)
      return 0;
    return memcmp(zLeft, zRight, nLeft) == 0;
  } else {
    return jsonLabelCompareEscaped(zLeft, nLeft, rawLeft, zRight, nRight, rawRight);
  }
}

void jsonParseReset(JsonParse *pParse) {
  if (pParse->bJsonIsRCStr) {
    sqlite3RCStrUnref(pParse->zJson);
    pParse->zJson = 0;
    pParse->nJson = 0;
    pParse->bJsonIsRCStr = 0;
  }
  if (pParse->nBlobAlloc) {
    sqlite3DbFree(pParse->db, pParse->aBlob);
    pParse->aBlob = 0;
    pParse->nBlob = 0;
    pParse->nBlobAlloc = 0;
  }
}

void jsonParseFree(JsonParse *pParse) {
  if (pParse) {
    if (pParse->nJPRef > 1) {
      pParse->nJPRef--;
    } else {
      jsonParseReset(pParse);
      sqlite3DbFree(pParse->db, pParse);
    }
  }
}

int jsonBlobExpand(JsonParse *pParse, u32 N) {
  u8 *aNew;
  u64 t;

  if (pParse->nBlobAlloc == 0) {
    t = 100;
  } else {
    t = pParse->nBlobAlloc * 2;
  }
  if (t < N)
    t = N + 100;
  aNew = sqlite3DbRealloc(pParse->db, pParse->aBlob, t);
  if (aNew == 0) {
    pParse->oom = 1;
    return 1;
  }

  pParse->aBlob = aNew;
  pParse->nBlobAlloc = (u32)t;
  return 0;
}

int jsonBlobMakeEditable(JsonParse *pParse, u32 nExtra) {
  u8 *aOld;
  u32 nSize;

  if (pParse->oom)
    return 0;
  if (pParse->nBlobAlloc > 0)
    return 1;
  aOld = pParse->aBlob;
  nSize = pParse->nBlob + nExtra;
  pParse->aBlob = 0;
  if (jsonBlobExpand(pParse, nSize)) {
    return 0;
  }

  memcpy(pParse->aBlob, aOld, pParse->nBlob);
  return 1;
}

__attribute__((noinline)) void jsonBlobExpandAndAppendOneByte(JsonParse *pParse, u8 c) {
  jsonBlobExpand(pParse, pParse->nBlob + 1);
  if (pParse->oom == 0) {
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

void jsonBlobAppendOneByte(JsonParse *pParse, u8 c) {
  if (pParse->nBlob >= pParse->nBlobAlloc) {
    jsonBlobExpandAndAppendOneByte(pParse, c);
  } else {
    pParse->aBlob[pParse->nBlob++] = c;
  }
}

__attribute__((noinline)) void jsonBlobExpandAndAppendNode(JsonParse *pParse, u8 eType, u64 szPayload,
                                                           const void *aPayload) {
  if (jsonBlobExpand(pParse, pParse->nBlob + szPayload + 9))
    return;
  jsonBlobAppendNode(pParse, eType, szPayload, aPayload);
}

void jsonBlobAppendNode(JsonParse *pParse, u8 eType, u64 szPayload, const void *aPayload) {
  u8 *a;
  if (pParse->nBlob + szPayload + 9 > pParse->nBlobAlloc) {
    jsonBlobExpandAndAppendNode(pParse, eType, szPayload, aPayload);
    return;
  }

  a = &pParse->aBlob[pParse->nBlob];
  if (szPayload <= 11) {
    a[0] = eType | (szPayload << 4);
    pParse->nBlob += 1;
  } else if (szPayload <= 0xff) {
    a[0] = eType | 0xc0;
    a[1] = szPayload & 0xff;
    pParse->nBlob += 2;
  } else if (szPayload <= 0xffff) {
    a[0] = eType | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
    pParse->nBlob += 3;
  } else {
    a[0] = eType | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
    pParse->nBlob += 5;
  }
  if (aPayload) {
    pParse->nBlob += szPayload;
    memcpy(&pParse->aBlob[pParse->nBlob - szPayload], aPayload, szPayload);
  }
}

int jsonBlobChangePayloadSize(JsonParse *pParse, u32 i, u32 szPayload) {
  u8 *a;
  u8 szType;
  u8 nExtra;
  u8 nNeeded;
  int delta;
  if (pParse->oom)
    return 0;
  a = &pParse->aBlob[i];
  szType = a[0] >> 4;
  if (szType <= 11) {
    nExtra = 0;
  } else if (szType == 12) {
    nExtra = 1;
  } else if (szType == 13) {
    nExtra = 2;
  } else if (szType == 14) {
    nExtra = 4;
  } else {
    nExtra = 8;
  }
  if (szPayload <= 11) {
    nNeeded = 0;
  } else if (szPayload <= 0xff) {
    nNeeded = 1;
  } else if (szPayload <= 0xffff) {
    nNeeded = 2;
  } else {
    nNeeded = 4;
  }
  delta = nNeeded - nExtra;
  if (delta) {
    u32 newSize = pParse->nBlob + delta;
    if (delta > 0) {
      if (newSize > pParse->nBlobAlloc && jsonBlobExpand(pParse, newSize)) {
        return 0;
      }
      a = &pParse->aBlob[i];
      memmove(&a[1 + delta], &a[1], pParse->nBlob - (i + 1));
    } else {
      memmove(&a[1], &a[1 - delta], pParse->nBlob - (i + 1 - delta));
    }
    pParse->nBlob = newSize;
  }
  if (nNeeded == 0) {
    a[0] = (a[0] & 0x0f) | (szPayload << 4);
  } else if (nNeeded == 1) {
    a[0] = (a[0] & 0x0f) | 0xc0;
    a[1] = szPayload & 0xff;
  } else if (nNeeded == 2) {
    a[0] = (a[0] & 0x0f) | 0xd0;
    a[1] = (szPayload >> 8) & 0xff;
    a[2] = szPayload & 0xff;
  } else {
    a[0] = (a[0] & 0x0f) | 0xe0;
    a[1] = (szPayload >> 24) & 0xff;
    a[2] = (szPayload >> 16) & 0xff;
    a[3] = (szPayload >> 8) & 0xff;
    a[4] = szPayload & 0xff;
  }
  return delta;
}

u32 jsonbValidityCheck(const JsonParse *pParse, u32 i, u32 iEnd, u32 iDepth) {
  u32 n, sz, j, k;
  const u8 *z;
  u8 x;
  if (iDepth > 1000)
    return i + 1;
  sz = 0;
  n = jsonbPayloadSize(pParse, i, &sz);
  if (n == 0)
    return i + 1;
  if ((i + n + sz != iEnd))
    return i + 1;
  z = pParse->aBlob;
  x = z[i] & 0x0f;
  switch (x) {
    case 0:
    case 1:
    case 2: {
      return n + sz == 1 ? 0 : i + 1;
    }
    case 3: {
      if (sz < 1)
        return i + 1;
      j = i + n;
      if (z[j] == '-') {
        j++;
        if (sz < 2)
          return i + 1;
      }
      k = i + n + sz;
      while (j < k) {
        if ((sqlite3CtypeMap[(unsigned char)(z[j])] & 0x04)) {
          j++;
        } else {
          return j + 1;
        }
      }
      return 0;
    }
    case 4: {
      if (sz < 3)
        return i + 1;
      j = i + n;
      if (z[j] == '-') {
        if (sz < 4)
          return i + 1;
        j++;
      }
      if (z[j] != '0')
        return i + 1;
      if (z[j + 1] != 'x' && z[j + 1] != 'X')
        return j + 2;
      j += 2;
      k = i + n + sz;
      while (j < k) {
        if ((sqlite3CtypeMap[(unsigned char)(z[j])] & 0x08)) {
          j++;
        } else {
          return j + 1;
        }
      }
      return 0;
    }
    case 5:
    case 6: {
      u8 seen = 0;
      if (sz < 2)
        return i + 1;
      j = i + n;
      k = j + sz;
      if (z[j] == '-') {
        j++;
        if (sz < 3)
          return i + 1;
      }
      if (z[j] == '.') {
        if (x == 5)
          return j + 1;
        if (!(sqlite3CtypeMap[(unsigned char)(z[j + 1])] & 0x04))
          return j + 1;
        j += 2;
        seen = 1;
      } else if (z[j] == '0' && x == 5) {
        if (j + 3 > k)
          return j + 1;
        if (z[j + 1] != '.' && z[j + 1] != 'e' && z[j + 1] != 'E')
          return j + 1;
        j++;
      }
      for (; j < k; j++) {
        if ((sqlite3CtypeMap[(unsigned char)(z[j])] & 0x04))
          continue;
        if (z[j] == '.') {
          if (seen > 0)
            return j + 1;
          if (x == 5 && (j == k - 1 || !(sqlite3CtypeMap[(unsigned char)(z[j + 1])] & 0x04))) {
            return j + 1;
          }
          seen = 1;
          continue;
        }
        if (z[j] == 'e' || z[j] == 'E') {
          if (seen == 2)
            return j + 1;
          if (j == k - 1)
            return j + 1;
          if (z[j + 1] == '+' || z[j + 1] == '-') {
            j++;
            if (j == k - 1)
              return j + 1;
          }
          seen = 2;
          continue;
        }
        return j + 1;
      }
      if (seen == 0)
        return i + 1;
      return 0;
    }
    case 7: {
      j = i + n;
      k = j + sz;
      while (j < k) {
        if (!jsonIsOk[z[j]] && z[j] != '\'')
          return j + 1;
        j++;
      }
      return 0;
    }
    case 8:
    case 9: {
      j = i + n;
      k = j + sz;
      while (j < k) {
        if (!jsonIsOk[z[j]] && z[j] != '\'') {
          if (z[j] == '"') {
            if (x == 8)
              return j + 1;
          } else if (z[j] <= 0x1f) {
            if (x == 8)
              return j + 1;
          } else if ((z[j] != '\\') || j + 1 >= k) {
            return j + 1;
          } else if (_Generic(0 ? ("\"\\/bfnrt") : (void *)1,
                         const void *: (const char *)(strchr("\"\\/bfnrt", z[j + 1])),
                         default: strchr("\"\\/bfnrt", z[j + 1])) != 0) {
            j++;
          } else if (z[j + 1] == 'u') {
            if (j + 5 >= k)
              return j + 1;
            if (!jsonIs4Hex((const char *)&z[j + 2]))
              return j + 1;
            j++;
          } else if (x != 9) {
            return j + 1;
          } else {
            u32 c = 0;
            u32 szC = jsonUnescapeOneChar((const char *)&z[j], k - j, &c);
            if (c == 0x99999)
              return j + 1;
            j += szC - 1;
          }
        }
        j++;
      }
      return 0;
    }
    case 10: {
      return 0;
    }
    case 11: {
      u32 sub;
      j = i + n;
      k = j + sz;
      while (j < k) {
        sz = 0;
        n = jsonbPayloadSize(pParse, j, &sz);
        if (n == 0)
          return j + 1;
        if (j + n + sz > k)
          return j + 1;
        sub = jsonbValidityCheck(pParse, j, j + n + sz, iDepth + 1);
        if (sub)
          return sub;
        j += n + sz;
      }

      return 0;
    }
    case 12: {
      u32 cnt = 0;
      u32 sub;
      j = i + n;
      k = j + sz;
      while (j < k) {
        sz = 0;
        n = jsonbPayloadSize(pParse, j, &sz);
        if (n == 0)
          return j + 1;
        if (j + n + sz > k)
          return j + 1;
        if ((cnt & 1) == 0) {
          x = z[j] & 0x0f;
          if (x < 7 || x > 10)
            return j + 1;
        }
        sub = jsonbValidityCheck(pParse, j, j + n + sz, iDepth + 1);
        if (sub)
          return sub;
        cnt++;
        j += n + sz;
      }

      if ((cnt & 1) != 0)
        return j + 1;
      return 0;
    }
    default: {
      return i + 1;
    }
  }
}

int jsonTranslateTextToBlob(JsonParse *pParse, u32 i) {
  char c;
  u32 j;
  u32 iThis, iStart;
  int x;
  u8 t;
  const char *z = pParse->zJson;
json_parse_restart:
  switch ((u8)z[i]) {
    case '{': {
      iThis = pParse->nBlob;
      jsonBlobAppendNode(pParse, 12, pParse->nJson - i, 0);
      if (++pParse->iDepth > 1000) {
        pParse->iErr = i;
        return -1;
      }
      iStart = pParse->nBlob;
      for (j = i + 1;; j++) {
        u32 iBlob = pParse->nBlob;
        x = jsonTranslateTextToBlob(pParse, j);
        if (x <= 0) {
          int op;
          if (x == (-2)) {
            j = pParse->iErr;
            if (pParse->nBlob != (u32)iStart)
              pParse->hasNonstd = 1;
            break;
          }
          j += json5Whitespace(&z[j]);
          op = 7;
          if ((sqlite3CtypeMap[(unsigned char)(z[j])] & 0x42) || (z[j] == '\\' && jsonIs4HexB(&z[j + 1], &op))) {
            int k = j + 1;
            while (((sqlite3CtypeMap[(unsigned char)(z[k])] & 0x46) && json5Whitespace(&z[k]) == 0) ||
                   (z[k] == '\\' && jsonIs4HexB(&z[k + 1], &op))) {
              k++;
            }

            jsonBlobAppendNode(pParse, op, k - j, &z[j]);
            pParse->hasNonstd = 1;
            x = k;
          } else {
            if (x != -1)
              pParse->iErr = j;
            return -1;
          }
        }
        if (pParse->oom)
          return -1;
        t = pParse->aBlob[iBlob] & 0x0f;
        if (t < 7 || t > 10) {
          pParse->iErr = j;
          return -1;
        }
        j = x;
        if (z[j] == ':') {
          j++;
        } else {
          if ((jsonIsSpace[(unsigned char)z[j]])) {
            do {
              j++;
            } while ((jsonIsSpace[(unsigned char)z[j]]));
            if (z[j] == ':') {
              j++;
              goto parse_object_value;
            }
          }
          x = jsonTranslateTextToBlob(pParse, j);
          if (x != (-5)) {
            if (x != (-1))
              pParse->iErr = j;
            return -1;
          }
          j = pParse->iErr + 1;
        }
      parse_object_value:
        x = jsonTranslateTextToBlob(pParse, j);
        if (x <= 0) {
          if (x != (-1))
            pParse->iErr = j;
          return -1;
        }
        j = x;
        if (z[j] == ',') {
          continue;
        } else if (z[j] == '}') {
          break;
        } else {
          if ((jsonIsSpace[(unsigned char)z[j]])) {
            j += 1 + (u32)strspn(&z[j + 1], jsonSpaces);
            if (z[j] == ',') {
              continue;
            } else if (z[j] == '}') {
              break;
            }
          }
          x = jsonTranslateTextToBlob(pParse, j);
          if (x == (-4)) {
            j = pParse->iErr;
            continue;
          }
          if (x == (-2)) {
            j = pParse->iErr;
            break;
          }
        }
        pParse->iErr = j;
        return -1;
      }
      jsonBlobChangePayloadSize(pParse, iThis, pParse->nBlob - iStart);
      pParse->iDepth--;
      return j + 1;
    }
    case '[': {
      iThis = pParse->nBlob;

      jsonBlobAppendNode(pParse, 11, pParse->nJson - i, 0);
      iStart = pParse->nBlob;
      if (pParse->oom)
        return -1;
      if (++pParse->iDepth > 1000) {
        pParse->iErr = i;
        return -1;
      }
      for (j = i + 1;; j++) {
        x = jsonTranslateTextToBlob(pParse, j);
        if (x <= 0) {
          if (x == (-3)) {
            j = pParse->iErr;
            if (pParse->nBlob != iStart)
              pParse->hasNonstd = 1;
            break;
          }
          if (x != (-1))
            pParse->iErr = j;
          return -1;
        }
        j = x;
        if (z[j] == ',') {
          continue;
        } else if (z[j] == ']') {
          break;
        } else {
          if ((jsonIsSpace[(unsigned char)z[j]])) {
            j += 1 + (u32)strspn(&z[j + 1], jsonSpaces);
            if (z[j] == ',') {
              continue;
            } else if (z[j] == ']') {
              break;
            }
          }
          x = jsonTranslateTextToBlob(pParse, j);
          if (x == (-4)) {
            j = pParse->iErr;
            continue;
          }
          if (x == (-3)) {
            j = pParse->iErr;
            break;
          }
        }
        pParse->iErr = j;
        return -1;
      }
      jsonBlobChangePayloadSize(pParse, iThis, pParse->nBlob - iStart);
      pParse->iDepth--;
      return j + 1;
    }
    case '\'': {
      u8 opcode;
      char cDelim;
      pParse->hasNonstd = 1;
      opcode = 7;
      goto parse_string;
      case '"':
        opcode = 7;
      parse_string:
        cDelim = z[i];
        j = i + 1;
        while (1) {
          if (jsonIsOk[(u8)z[j]]) {
            if (!jsonIsOk[(u8)z[j + 1]]) {
              j += 1;
            } else if (!jsonIsOk[(u8)z[j + 2]]) {
              j += 2;
            } else {
              j += 3;
              continue;
            }
          }
          c = z[j];
          if (c == cDelim) {
            break;
          } else if (c == '\\') {
            c = z[++j];
            if (c == '"' || c == '\\' || c == '/' || c == 'b' || c == 'f' || c == 'n' || c == 'r' || c == 't' ||
                (c == 'u' && jsonIs4Hex(&z[j + 1]))) {
              if (opcode == 7)
                opcode = 8;
            } else if (c == '\'' || c == 'v' || c == '\n' ||
                       (c == '0' && !(sqlite3CtypeMap[(unsigned char)(z[j + 1])] & 0x04)) ||
                       (0xe2 == (u8)c && 0x80 == (u8)z[j + 1] && (0xa8 == (u8)z[j + 2] || 0xa9 == (u8)z[j + 2])) ||
                       (c == 'x' && jsonIs2Hex(&z[j + 1]))) {
              opcode = 9;
              pParse->hasNonstd = 1;
            } else if (c == '\r') {
              if (z[j + 1] == '\n')
                j++;
              opcode = 9;
              pParse->hasNonstd = 1;
            } else {
              pParse->iErr = j;
              return -1;
            }
          } else if (c <= 0x1f) {
            if (c == 0) {
              pParse->iErr = j;
              return -1;
            }

            opcode = 9;
            pParse->hasNonstd = 1;
          } else if (c == '"') {
            opcode = 9;
          }
          j++;
        }
        jsonBlobAppendNode(pParse, opcode, j - 1 - i, &z[i + 1]);
        return j + 1;
    }
    case 't': {
      if (strncmp(z + i, "true", 4) == 0 && !(sqlite3CtypeMap[(unsigned char)(z[i + 4])] & 0x06)) {
        jsonBlobAppendOneByte(pParse, 1);
        return i + 4;
      }
      pParse->iErr = i;
      return -1;
    }
    case 'f': {
      if (strncmp(z + i, "false", 5) == 0 && !(sqlite3CtypeMap[(unsigned char)(z[i + 5])] & 0x06)) {
        jsonBlobAppendOneByte(pParse, 2);
        return i + 5;
      }
      pParse->iErr = i;
      return -1;
    }
    case '+': {
      u8 seenE;
      pParse->hasNonstd = 1;
      t = 0x00;
      goto parse_number;
      case '.':
        if ((sqlite3CtypeMap[(unsigned char)(z[i + 1])] & 0x04)) {
          pParse->hasNonstd = 1;
          t = 0x03;
          seenE = 0;
          goto parse_number_2;
        }
        pParse->iErr = i;
        return -1;
      case '-':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        t = 0x00;
      parse_number:
        seenE = 0;

        c = z[i];

        if (c <= '0') {
          if (c == '0') {
            if ((z[i + 1] == 'x' || z[i + 1] == 'X') && (sqlite3CtypeMap[(unsigned char)(z[i + 2])] & 0x08)) {
              pParse->hasNonstd = 1;
              t = 0x01;
              for (j = i + 3; (sqlite3CtypeMap[(unsigned char)(z[j])] & 0x08); j++) {
              }
              goto parse_number_finish;
            } else if ((sqlite3CtypeMap[(unsigned char)(z[i + 1])] & 0x04)) {
              pParse->iErr = i + 1;
              return -1;
            }
          } else {
            if (!(sqlite3CtypeMap[(unsigned char)(z[i + 1])] & 0x04)) {
              if ((z[i + 1] == 'I' || z[i + 1] == 'i') && sqlite3_strnicmp(&z[i + 1], "inf", 3) == 0) {
                pParse->hasNonstd = 1;
                if (z[i] == '-') {
                  jsonBlobAppendNode(pParse, 5, 6, "-9e999");
                } else {
                  jsonBlobAppendNode(pParse, 5, 5, "9e999");
                }
                return i + (sqlite3_strnicmp(&z[i + 4], "inity", 5) == 0 ? 9 : 4);
              }
              if (z[i + 1] == '.') {
                pParse->hasNonstd = 1;
                t |= 0x01;
                goto parse_number_2;
              }
              pParse->iErr = i;
              return -1;
            }
            if (z[i + 1] == '0') {
              if ((sqlite3CtypeMap[(unsigned char)(z[i + 2])] & 0x04)) {
                pParse->iErr = i + 1;
                return -1;
              } else if ((z[i + 2] == 'x' || z[i + 2] == 'X') && (sqlite3CtypeMap[(unsigned char)(z[i + 3])] & 0x08)) {
                pParse->hasNonstd = 1;
                t |= 0x01;
                for (j = i + 4; (sqlite3CtypeMap[(unsigned char)(z[j])] & 0x08); j++) {
                }
                goto parse_number_finish;
              }
            }
          }
        }

      parse_number_2:
        for (j = i + 1;; j++) {
          c = z[j];
          if ((sqlite3CtypeMap[(unsigned char)(c)] & 0x04))
            continue;
          if (c == '.') {
            if ((t & 0x02) != 0) {
              pParse->iErr = j;
              return -1;
            }
            t |= 0x02;
            continue;
          }
          if (c == 'e' || c == 'E') {
            if (z[j - 1] < '0') {
              if ((z[j - 1] == '.') && (j - 2 >= i) && (sqlite3CtypeMap[(unsigned char)(z[j - 2])] & 0x04)) {
                pParse->hasNonstd = 1;
                t |= 0x01;
              } else {
                pParse->iErr = j;
                return -1;
              }
            }
            if (seenE) {
              pParse->iErr = j;
              return -1;
            }
            t |= 0x02;
            seenE = 1;
            c = z[j + 1];
            if (c == '+' || c == '-') {
              j++;
              c = z[j + 1];
            }
            if (c < '0' || c > '9') {
              pParse->iErr = j;
              return -1;
            }
            continue;
          }
          break;
        }
        if (z[j - 1] < '0') {
          if ((z[j - 1] == '.') && (j - 2 >= i) && (sqlite3CtypeMap[(unsigned char)(z[j - 2])] & 0x04)) {
            pParse->hasNonstd = 1;
            t |= 0x01;
          } else {
            pParse->iErr = j;
            return -1;
          }
        }
      parse_number_finish:
        if (z[i] == '+')
          i++;
        jsonBlobAppendNode(pParse, 3 + t, j - i, &z[i]);
        return j;
    }
    case '}': {
      pParse->iErr = i;
      return -2;
    }
    case ']': {
      pParse->iErr = i;
      return -3;
    }
    case ',': {
      pParse->iErr = i;
      return -4;
    }
    case ':': {
      pParse->iErr = i;
      return -5;
    }
    case 0: {
      return 0;
    }
    case 0x09:
    case 0x0a:
    case 0x0d:
    case 0x20: {
      i += 1 + (u32)strspn(&z[i + 1], jsonSpaces);
      goto json_parse_restart;
    }
    case 0x0b:
    case 0x0c:
    case '/':
    case 0xc2:
    case 0xe1:
    case 0xe2:
    case 0xe3:
    case 0xef: {
      j = json5Whitespace(&z[i]);
      if (j > 0) {
        i += j;
        pParse->hasNonstd = 1;
        goto json_parse_restart;
      }
      pParse->iErr = i;
      return -1;
    }
    case 'n': {
      if (strncmp(z + i, "null", 4) == 0 && !(sqlite3CtypeMap[(unsigned char)(z[i + 4])] & 0x06)) {
        jsonBlobAppendOneByte(pParse, 0);
        return i + 4;
      }

      __attribute__((fallthrough));
    }
    default: {
      u32 k;
      int nn;
      c = z[i];
      for (k = 0; k < sizeof(aNanInfName) / sizeof(aNanInfName[0]); k++) {
        if (c != aNanInfName[k].c1 && c != aNanInfName[k].c2)
          continue;
        nn = aNanInfName[k].n;
        if (sqlite3_strnicmp(&z[i], aNanInfName[k].zMatch, nn) != 0) {
          continue;
        }
        if ((sqlite3CtypeMap[(unsigned char)(z[i + nn])] & 0x06))
          continue;
        if (aNanInfName[k].eType == 5) {
          jsonBlobAppendNode(pParse, 5, 5, "9e999");
        } else {
          jsonBlobAppendOneByte(pParse, 0);
        }
        pParse->hasNonstd = 1;
        return i + nn;
      }
      pParse->iErr = i;
      return -1;
    }
  }
}

int jsonConvertTextToBlob(JsonParse *pParse, sqlite3_context *pCtx) {
  int i;
  const char *zJson = pParse->zJson;
  i = jsonTranslateTextToBlob(pParse, 0);
  if (pParse->oom)
    i = -1;
  if (i > 0) {
    while ((jsonIsSpace[(unsigned char)zJson[i]]))
      i++;
    if (zJson[i]) {
      i += json5Whitespace(&zJson[i]);
      if (zJson[i]) {
        if (pCtx)
          sqlite3_result_error(pCtx, "malformed JSON", -1);
        jsonParseReset(pParse);
        return 1;
      }
      pParse->hasNonstd = 1;
    }
  }
  if (i <= 0) {
    if (pCtx != 0) {
      if (pParse->oom) {
        sqlite3_result_error_nomem(pCtx);
      } else {
        sqlite3_result_error(pCtx, "malformed JSON", -1);
      }
    }
    jsonParseReset(pParse);
    return 1;
  }
  return 0;
}

u32 jsonbPayloadSize(const JsonParse *pParse, u32 i, u32 *pSz) {
  u8 x;
  u32 sz;
  u32 n;
  if (i >= pParse->nBlob) {
    *pSz = 0;
    return 0;
  } else if ((x = pParse->aBlob[i] >> 4) <= 11) {
    sz = x;
    n = 1;
  } else if (x == 12) {
    if (i + 1 >= pParse->nBlob) {
      *pSz = 0;
      return 0;
    }
    sz = pParse->aBlob[i + 1];
    n = 2;
  } else if (x == 13) {
    if (i + 2 >= pParse->nBlob) {
      *pSz = 0;
      return 0;
    }
    sz = (pParse->aBlob[i + 1] << 8) + pParse->aBlob[i + 2];
    n = 3;
  } else if (x == 14) {
    if (i + 4 >= pParse->nBlob) {
      *pSz = 0;
      return 0;
    }
    sz = ((u32)pParse->aBlob[i + 1] << 24) + (pParse->aBlob[i + 2] << 16) + (pParse->aBlob[i + 3] << 8) +
         pParse->aBlob[i + 4];
    n = 5;
  } else {
    if (i + 8 >= pParse->nBlob || pParse->aBlob[i + 1] != 0 || pParse->aBlob[i + 2] != 0 || pParse->aBlob[i + 3] != 0 ||
        pParse->aBlob[i + 4] != 0) {
      *pSz = 0;
      return 0;
    }
    sz = ((u32)pParse->aBlob[i + 5] << 24) + (pParse->aBlob[i + 6] << 16) + (pParse->aBlob[i + 7] << 8) +
         pParse->aBlob[i + 8];
    n = 9;
  }
  if ((i64)i + sz + n > pParse->nBlob && (i64)i + sz + n > pParse->nBlob - pParse->delta) {
    *pSz = 0;
    return 0;
  }
  *pSz = sz;
  return n;
}

u32 jsonTranslateBlobToText(JsonParse *pParse, u32 i, JsonString *pOut) {
  u32 sz, n, j, iEnd;

  n = jsonbPayloadSize(pParse, i, &sz);
  if (n == 0) {
    pOut->eErr |= 0x02;
    return pParse->nBlob + 1;
  }
  switch (pParse->aBlob[i] & 0x0f) {
    case 0: {
      jsonAppendRawNZ(pOut, "null", 4);
      return i + 1;
    }
    case 1: {
      jsonAppendRawNZ(pOut, "true", 4);
      return i + 1;
    }
    case 2: {
      jsonAppendRawNZ(pOut, "false", 5);
      return i + 1;
    }
    case 3:
    case 5: {
      if (sz == 0)
        goto malformed_jsonb;
      jsonAppendRaw(pOut, (const char *)&pParse->aBlob[i + n], sz);
      break;
    }
    case 4: {
      u32 k = 2;
      sqlite3_uint64 u = 0;
      const char *zIn = (const char *)&pParse->aBlob[i + n];
      int bOverflow = 0;
      if (sz == 0)
        goto malformed_jsonb;
      if (zIn[0] == '-') {
        jsonAppendChar(pOut, '-');
        k++;
      } else if (zIn[0] == '+') {
        k++;
      }
      for (; k < sz; k++) {
        if (!(sqlite3CtypeMap[(unsigned char)(zIn[k])] & 0x08)) {
          pOut->eErr |= 0x02;
          break;
        } else if ((u >> 60) != 0) {
          bOverflow = 1;
        } else {
          u = u * 16 + sqlite3HexToInt(zIn[k]);
        }
      }
      jsonPrintf(100, pOut, bOverflow ? "9.0e999" : "%llu", u);
      break;
    }
    case 6: {
      u32 k = 0;
      const char *zIn = (const char *)&pParse->aBlob[i + n];
      if (sz == 0)
        goto malformed_jsonb;
      if (zIn[0] == '-') {
        jsonAppendChar(pOut, '-');
        if (sz <= 1)
          goto malformed_jsonb;
        k = 1;
      }
      if (zIn[k] == '.') {
        jsonAppendChar(pOut, '0');
      }
      for (; k < sz; k++) {
        jsonAppendChar(pOut, zIn[k]);
        if (zIn[k] == '.' && (k + 1 == sz || !(sqlite3CtypeMap[(unsigned char)(zIn[k + 1])] & 0x04))) {
          jsonAppendChar(pOut, '0');
        }
      }
      break;
    }
    case 7:
    case 8: {
      if (pOut->nUsed + sz + 2 <= pOut->nAlloc || jsonStringGrow(pOut, sz + 2) == 0) {
        pOut->zBuf[pOut->nUsed] = '"';
        memcpy(pOut->zBuf + pOut->nUsed + 1, (const char *)&pParse->aBlob[i + n], sz);
        pOut->zBuf[pOut->nUsed + sz + 1] = '"';
        pOut->nUsed += sz + 2;
      }
      break;
    }
    case 9: {
      const char *zIn;
      u32 k;
      u32 sz2 = sz;
      zIn = (const char *)&pParse->aBlob[i + n];
      jsonAppendChar(pOut, '"');
      while (sz2 > 0) {
        for (k = 0; k < sz2 && (jsonIsOk[(u8)zIn[k]] || zIn[k] == '\''); k++) {
        }
        if (k > 0) {
          jsonAppendRawNZ(pOut, zIn, k);
          if (k >= sz2) {
            break;
          }
          zIn += k;
          sz2 -= k;
        }
        if (zIn[0] == '"') {
          jsonAppendRawNZ(pOut, "\\\"", 2);
          zIn++;
          sz2--;
          continue;
        }
        if (zIn[0] <= 0x1f) {
          if (pOut->nUsed + 7 > pOut->nAlloc && jsonStringGrow(pOut, 7))
            break;
          jsonAppendControlChar(pOut, zIn[0]);
          zIn++;
          sz2--;
          continue;
        }

        if (sz2 < 2) {
          pOut->eErr |= 0x02;
          break;
        }
        switch ((u8)zIn[1]) {
          case '\'':
            jsonAppendChar(pOut, '\'');
            break;
          case 'v':
            jsonAppendRawNZ(pOut, "\\u000b", 6);
            break;
          case 'x':
            if (sz2 < 4) {
              pOut->eErr |= 0x02;
              sz2 = 2;
              break;
            }
            jsonAppendRawNZ(pOut, "\\u00", 4);
            jsonAppendRawNZ(pOut, &zIn[2], 2);
            zIn += 2;
            sz2 -= 2;
            break;
          case '0':
            jsonAppendRawNZ(pOut, "\\u0000", 6);
            break;
          case '\r':
            if (sz2 > 2 && zIn[2] == '\n') {
              zIn++;
              sz2--;
            }
            break;
          case '\n':
            break;
          case 0xe2:
            if (sz2 < 4 || 0x80 != (u8)zIn[2] || (0xa8 != (u8)zIn[3] && 0xa9 != (u8)zIn[3])) {
              pOut->eErr |= 0x02;
              sz2 = 2;
              break;
            }
            zIn += 2;
            sz2 -= 2;
            break;
          default:
            jsonAppendRawNZ(pOut, zIn, 2);
            break;
        }

        zIn += 2;
        sz2 -= 2;
      }
      jsonAppendChar(pOut, '"');
      break;
    }
    case 10: {
      jsonAppendString(pOut, (const char *)&pParse->aBlob[i + n], sz);
      break;
    }
    case 11: {
      jsonAppendChar(pOut, '[');
      j = i + n;
      iEnd = j + sz;
      if (++pParse->iDepth > 1000) {
        jsonStringTooDeep(pOut);
      }
      while (j < iEnd && pOut->eErr == 0) {
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, ',');
      }
      pParse->iDepth--;
      if (j > iEnd)
        pOut->eErr |= 0x02;
      if (sz > 0)
        jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, ']');
      break;
    }
    case 12: {
      int x = 0;
      jsonAppendChar(pOut, '{');
      j = i + n;
      iEnd = j + sz;
      if (++pParse->iDepth > 1000) {
        jsonStringTooDeep(pOut);
      }
      while (j < iEnd && pOut->eErr == 0) {
        j = jsonTranslateBlobToText(pParse, j, pOut);
        jsonAppendChar(pOut, (x++ & 1) ? ',' : ':');
      }
      pParse->iDepth--;
      if ((x & 1) != 0 || j > iEnd)
        pOut->eErr |= 0x02;
      if (sz > 0)
        jsonStringTrimOneChar(pOut);
      jsonAppendChar(pOut, '}');
      break;
    }

    default: {
    malformed_jsonb:
      pOut->eErr |= 0x02;
      break;
    }
  }
  return i + n + sz;
}

u32 jsonbArrayCount(JsonParse *pParse, u32 iRoot) {
  u32 n, sz, i, iEnd;
  u32 k = 0;
  n = jsonbPayloadSize(pParse, iRoot, &sz);
  iEnd = iRoot + n + sz;
  for (i = iRoot + n; n > 0 && i < iEnd; i += sz + n, k++) {
    n = jsonbPayloadSize(pParse, i, &sz);
  }
  return k;
}

void jsonAfterEditSizeAdjust(JsonParse *pParse, u32 iRoot) {
  u32 sz = 0;
  u32 nBlob;

  nBlob = pParse->nBlob;
  pParse->nBlob = pParse->nBlobAlloc;
  (void)jsonbPayloadSize(pParse, iRoot, &sz);
  pParse->nBlob = nBlob;
  sz += pParse->delta;
  pParse->delta += jsonBlobChangePayloadSize(pParse, iRoot, sz);
}

void jsonBlobEdit(JsonParse *pParse, u32 iDel, u32 nDel, const u8 *aIns, u32 nIns) {
  i64 d = (i64)nIns - (i64)nDel;

  if (d < 0 && d >= (-8) && aIns != 0 && jsonBlobOverwrite(&pParse->aBlob[iDel], aIns, nIns, (int)-d)) {
    return;
  }
  if (d != 0) {
    if (pParse->nBlob + d > pParse->nBlobAlloc) {
      jsonBlobExpand(pParse, pParse->nBlob + d);
      if (pParse->oom)
        return;
    }
    memmove(&pParse->aBlob[iDel + nIns], &pParse->aBlob[iDel + nDel], pParse->nBlob - (iDel + nDel));
    pParse->nBlob += d;
    pParse->delta += d;
  }
  if (nIns && aIns) {
    memcpy(&pParse->aBlob[iDel], aIns, nIns);
  }
}

u32 jsonCreateEditSubstructure(JsonParse *pParse, JsonParse *pIns, const char *zTail) {
  static const u8 emptyObject[] = {11, 12};
  int rc;
  memset(pIns, 0, sizeof(*pIns));
  pIns->db = pParse->db;
  if (zTail[0] == 0) {
    pIns->aBlob = pParse->aIns;
    pIns->nBlob = pParse->nIns;
    rc = 0;
  } else {
    pIns->nBlob = 1;
    pIns->aBlob = (u8 *)&emptyObject[zTail[0] == '.'];
    pIns->eEdit = pParse->eEdit;
    pIns->nIns = pParse->nIns;
    pIns->aIns = pParse->aIns;
    pIns->iDepth = pParse->iDepth + 1;
    if (pIns->iDepth >= 1000) {
      return 0xfffffffc;
    }
    rc = jsonLookupStep(pIns, 0, zTail, 0);
    pParse->iDepth--;
    pParse->oom |= pIns->oom;
  }
  return rc;
}

u32 jsonLookupStep(JsonParse *pParse, u32 iRoot, const char *zPath, u32 iLabel) {
  u32 i, j, k, nKey, sz, n, iEnd, rc;
  const char *zKey;
  u8 x;

  if (zPath[0] == 0) {
    if (pParse->eEdit && jsonBlobMakeEditable(pParse, pParse->nIns)) {
      n = jsonbPayloadSize(pParse, iRoot, &sz);
      sz += n;
      if (pParse->eEdit == 1) {
        if (iLabel > 0) {
          sz += iRoot - iLabel;
          iRoot = iLabel;
        }
        jsonBlobEdit(pParse, iRoot, sz, 0, 0);
      } else if (pParse->eEdit == 3) {
      } else if (pParse->eEdit == 5) {
        if (zPath[-1] != ']') {
          return 0xfffffffd;
        } else {
          jsonBlobEdit(pParse, iRoot, 0, pParse->aIns, pParse->nIns);
        }
      } else {
        jsonBlobEdit(pParse, iRoot, sz, pParse->aIns, pParse->nIns);
      }
    }
    pParse->iLabel = iLabel;
    return iRoot;
  }
  if (zPath[0] == '.') {
    int rawKey = 1;
    x = pParse->aBlob[iRoot];
    zPath++;
    if (zPath[0] == '"') {
      zKey = zPath + 1;
      for (i = 1; zPath[i] && zPath[i] != '"'; i++) {
        if (zPath[i] == '\\' && zPath[i + 1] != 0)
          i++;
      }
      nKey = i - 1;
      if (zPath[i]) {
        i++;
      } else {
        return 0xfffffffb;
      };
      rawKey = _Generic(0 ? (zKey) : (void *)1,
                   const void *: (const void *)(memchr(zKey, '\\', nKey)),
                   default: memchr(zKey, '\\', nKey)) == 0;
    } else {
      zKey = zPath;
      for (i = 0; zPath[i] && zPath[i] != '.' && zPath[i] != '['; i++) {
      }
      nKey = i;
      if (nKey == 0) {
        return 0xfffffffb;
      }
    }
    if ((x & 0x0f) != 12)
      return 0xfffffffe;
    n = jsonbPayloadSize(pParse, iRoot, &sz);
    j = iRoot + n;
    iEnd = j + sz;
    while (j < iEnd) {
      int rawLabel;
      const char *zLabel;
      x = pParse->aBlob[j] & 0x0f;
      if (x < 7 || x > 10)
        return 0xffffffff;
      n = jsonbPayloadSize(pParse, j, &sz);
      if (n == 0)
        return 0xffffffff;
      k = j + n;
      if (k + sz >= iEnd)
        return 0xffffffff;
      zLabel = (const char *)&pParse->aBlob[k];
      rawLabel = x == 7 || x == 10;
      if (jsonLabelCompare(zKey, nKey, rawKey, zLabel, sz, rawLabel)) {
        u32 v = k + sz;
        if (((pParse->aBlob[v]) & 0x0f) > 12)
          return 0xffffffff;
        n = jsonbPayloadSize(pParse, v, &sz);
        if (n == 0 || v + n + sz > iEnd)
          return 0xffffffff;

        if (++pParse->iDepth >= 1000) {
          return 0xfffffffc;
        }
        rc = jsonLookupStep(pParse, v, &zPath[i], j);
        pParse->iDepth--;
        if (pParse->delta)
          jsonAfterEditSizeAdjust(pParse, iRoot);
        return rc;
      }
      j = k + sz;
      if (((pParse->aBlob[j]) & 0x0f) > 12)
        return 0xffffffff;
      n = jsonbPayloadSize(pParse, j, &sz);
      if (n == 0)
        return 0xffffffff;
      j += n + sz;
    }
    if (j > iEnd)
      return 0xffffffff;
    if (pParse->eEdit >= 3) {
      u32 nIns;
      JsonParse v;
      JsonParse ix;
      if (pParse->eEdit == 5 && sqlite3_strglob("*]", &zPath[i]) != 0) {
        return 0xfffffffd;
      }
      memset(&ix, 0, sizeof(ix));
      ix.db = pParse->db;
      jsonBlobAppendNode(&ix, rawKey ? 10 : 9, nKey, 0);
      pParse->oom |= ix.oom;
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i]);
      if (!((rc) >= 0xfffffffb) && jsonBlobMakeEditable(pParse, ix.nBlob + nKey + v.nBlob)) {
        nIns = ix.nBlob + nKey + v.nBlob;
        jsonBlobEdit(pParse, j, 0, 0, nIns);
        if (!pParse->oom) {
          memcpy(&pParse->aBlob[j], ix.aBlob, ix.nBlob);
          k = j + ix.nBlob;
          memcpy(&pParse->aBlob[k], zKey, nKey);
          k += nKey;
          memcpy(&pParse->aBlob[k], v.aBlob, v.nBlob);
          if ((pParse->delta))
            jsonAfterEditSizeAdjust(pParse, iRoot);
        }
      }
      jsonParseReset(&v);
      jsonParseReset(&ix);
      return rc;
    }
  } else if (zPath[0] == '[') {
    u64 kk = 0;
    x = pParse->aBlob[iRoot] & 0x0f;
    if (x != 11)
      return 0xfffffffe;
    n = jsonbPayloadSize(pParse, iRoot, &sz);
    i = 1;
    while ((sqlite3CtypeMap[(unsigned char)(zPath[i])] & 0x04)) {
      if (kk < 0xffffffff)
        kk = kk * 10 + zPath[i] - '0';

      i++;
    }
    if (i < 2 || zPath[i] != ']') {
      if (zPath[1] == '#') {
        kk = jsonbArrayCount(pParse, iRoot);
        i = 2;
        if (zPath[2] == '-' && (sqlite3CtypeMap[(unsigned char)(zPath[3])] & 0x04)) {
          u64 nn = 0;
          i = 3;
          do {
            if (nn < 0xffffffff)
              nn = nn * 10 + zPath[i] - '0';

            i++;
          } while ((sqlite3CtypeMap[(unsigned char)(zPath[i])] & 0x04));
          if (nn > kk)
            return 0xfffffffe;
          kk -= nn;
        }
        if (zPath[i] != ']') {
          return 0xfffffffb;
        }
      } else {
        return 0xfffffffb;
      }
    }
    j = iRoot + n;
    iEnd = j + sz;
    while (j < iEnd) {
      if (kk == 0) {
        if (++pParse->iDepth >= 1000) {
          return 0xfffffffc;
        }
        rc = jsonLookupStep(pParse, j, &zPath[i + 1], 0);
        pParse->iDepth--;
        if (pParse->delta)
          jsonAfterEditSizeAdjust(pParse, iRoot);
        return rc;
      }
      kk--;
      n = jsonbPayloadSize(pParse, j, &sz);
      if (n == 0)
        return 0xffffffff;
      j += n + sz;
    }
    if (j > iEnd)
      return 0xffffffff;
    if (kk > 0)
      return 0xfffffffe;
    if (pParse->eEdit >= 3) {
      JsonParse v;
      rc = jsonCreateEditSubstructure(pParse, &v, &zPath[i + 1]);
      if (!((rc) >= 0xfffffffb) && jsonBlobMakeEditable(pParse, v.nBlob)) {
        jsonBlobEdit(pParse, j, 0, v.aBlob, v.nBlob);
      }
      jsonParseReset(&v);
      if (pParse->delta)
        jsonAfterEditSizeAdjust(pParse, iRoot);
      return rc;
    }
  } else {
    return 0xfffffffb;
  }
  return 0xfffffffe;
}

void jsonReturnFromBlob(JsonParse *pParse, u32 i, sqlite3_context *pCtx, int eMode) {
  u32 n, sz;
  int rc;
  sqlite3 *db = sqlite3_context_db_handle(pCtx);

  n = jsonbPayloadSize(pParse, i, &sz);
  if (n == 0) {
    sqlite3_result_error(pCtx, "malformed JSON", -1);
    return;
  }
  switch (pParse->aBlob[i] & 0x0f) {
    case 0: {
      if (sz)
        goto returnfromblob_malformed;
      sqlite3_result_null(pCtx);
      break;
    }
    case 1: {
      if (sz)
        goto returnfromblob_malformed;
      sqlite3_result_int(pCtx, 1);
      break;
    }
    case 2: {
      if (sz)
        goto returnfromblob_malformed;
      sqlite3_result_int(pCtx, 0);
      break;
    }
    case 4:
    case 3: {
      sqlite3_int64 iRes = 0;
      char *z;
      int bNeg = 0;
      char x;
      if (sz == 0)
        goto returnfromblob_malformed;
      x = (char)pParse->aBlob[i + n];
      if (x == '-') {
        if (sz < 2)
          goto returnfromblob_malformed;
        n++;
        sz--;
        bNeg = 1;
      }
      z = sqlite3DbStrNDup(db, (const char *)&pParse->aBlob[i + n], (int)sz);
      if (z == 0)
        goto returnfromblob_oom;
      rc = sqlite3DecOrHexToI64(z, &iRes);
      sqlite3DbFree(db, z);
      if (rc == 0) {
        if (iRes < 0) {
          double r;
          r = (double)*(sqlite3_uint64 *)&iRes;
          sqlite3_result_double(pCtx, bNeg ? -r : r);
        } else {
          sqlite3_result_int64(pCtx, bNeg ? -iRes : iRes);
        }
      } else if (rc == 3 && bNeg) {
        sqlite3_result_int64(pCtx, (((i64)-1) - (0xffffffff | (((i64)0x7fffffff) << 32))));
      } else if (rc == 1) {
        goto returnfromblob_malformed;
      } else {
        if (bNeg) {
          n--;
          sz++;
        }
        goto to_double;
      }
      break;
    }
    case 6:
    case 5: {
      double r;
      char *z;
      if (sz == 0)
        goto returnfromblob_malformed;
    to_double:
      z = sqlite3DbStrNDup(db, (const char *)&pParse->aBlob[i + n], (int)sz);
      if (z == 0)
        goto returnfromblob_oom;
      rc = sqlite3AtoF(z, &r);
      sqlite3DbFree(db, z);
      if (rc <= 0)
        goto returnfromblob_malformed;
      sqlite3_result_double(pCtx, r);
      break;
    }
    case 10:
    case 7: {
      sqlite3_result_text(pCtx, (char *)&pParse->aBlob[i + n], sz, ((sqlite3_destructor_type)-1));
      break;
    }
    case 9:
    case 8: {
      u32 iIn, iOut;
      const char *z;
      char *zOut;
      u32 nOut = sz;
      z = (const char *)&pParse->aBlob[i + n];
      zOut = sqlite3DbMallocRaw(db, ((u64)nOut) + 1);
      if (zOut == 0)
        goto returnfromblob_oom;
      for (iIn = iOut = 0; iIn < sz; iIn++) {
        char c = z[iIn];
        if (c == '\\') {
          u32 v;
          u32 szEscape = jsonUnescapeOneChar(&z[iIn], sz - iIn, &v);
          if (v <= 0x7f) {
            zOut[iOut++] = (char)v;
          } else if (v <= 0x7ff) {
            zOut[iOut++] = (char)(0xc0 | (v >> 6));
            zOut[iOut++] = 0x80 | (v & 0x3f);
          } else if (v < 0x10000) {
            zOut[iOut++] = 0xe0 | (v >> 12);
            zOut[iOut++] = 0x80 | ((v >> 6) & 0x3f);
            zOut[iOut++] = 0x80 | (v & 0x3f);
          } else if (v == 0x99999) {
          } else {
            zOut[iOut++] = 0xf0 | (v >> 18);
            zOut[iOut++] = 0x80 | ((v >> 12) & 0x3f);
            zOut[iOut++] = 0x80 | ((v >> 6) & 0x3f);
            zOut[iOut++] = 0x80 | (v & 0x3f);
          }
          iIn += szEscape - 1;
        } else {
          zOut[iOut++] = c;
        }
      }

      zOut[iOut] = 0;
      sqlite3_result_text(pCtx, zOut, iOut, ((sqlite3_destructor_type)sqlite3RowSetClear));
      break;
    }
    case 11:
    case 12: {
      if (eMode == 0) {
        if ((((int)(intptr_t)(sqlite3_user_data(pCtx))) & 0x10) != 0) {
          eMode = 2;
        } else {
          eMode = 1;
        }
      }
      if (eMode == 2) {
        sqlite3_result_blob(pCtx, &pParse->aBlob[i], sz + n, ((sqlite3_destructor_type)-1));
      } else {
        jsonReturnTextJsonFromBlob(pCtx, &pParse->aBlob[i], sz + n);
      }
      break;
    }
    default: {
      goto returnfromblob_malformed;
    }
  }
  return;

returnfromblob_oom:
  sqlite3_result_error_nomem(pCtx);
  return;

returnfromblob_malformed:
  sqlite3_result_error(pCtx, "malformed JSON", -1);
  return;
}

int jsonMergePatch(JsonParse *pTarget, u32 iTarget, const JsonParse *pPatch, u32 iPatch, u32 iDepth) {
  u8 x;
  u32 n, sz = 0;
  u32 iTCursor;
  u32 iTStart;
  u32 iTEndBE;
  u32 iTEnd;
  u8 eTLabel;
  u32 iTLabel = 0;
  u32 nTLabel = 0;
  u32 szTLabel = 0;
  u32 iTValue = 0;
  u32 nTValue = 0;
  u32 szTValue = 0;

  u32 iPCursor;
  u32 iPEnd;
  u8 ePLabel;
  u32 iPLabel;
  u32 nPLabel;
  u32 szPLabel;
  u32 iPValue;
  u32 nPValue;
  u32 szPValue;

  x = pPatch->aBlob[iPatch] & 0x0f;
  if (x != 12) {
    u32 szPatch;
    u32 szTarget;
    n = jsonbPayloadSize(pPatch, iPatch, &sz);
    szPatch = n + sz;
    sz = 0;
    n = jsonbPayloadSize(pTarget, iTarget, &sz);
    szTarget = n + sz;
    jsonBlobEdit(pTarget, iTarget, szTarget, pPatch->aBlob + iPatch, szPatch);
    return pTarget->oom ? 3 : 0;
  }
  x = pTarget->aBlob[iTarget] & 0x0f;
  if (x != 12) {
    n = jsonbPayloadSize(pTarget, iTarget, &sz);
    jsonBlobEdit(pTarget, iTarget + n, sz, 0, 0);
    x = pTarget->aBlob[iTarget];
    pTarget->aBlob[iTarget] = (x & 0xf0) | 12;
  }
  n = jsonbPayloadSize(pPatch, iPatch, &sz);
  if (n == 0)
    return 2;
  iPCursor = iPatch + n;
  iPEnd = iPCursor + sz;
  n = jsonbPayloadSize(pTarget, iTarget, &sz);
  if (n == 0)
    return 1;
  iTStart = iTarget + n;
  iTEndBE = iTStart + sz;

  while (iPCursor < iPEnd) {
    iPLabel = iPCursor;
    ePLabel = pPatch->aBlob[iPCursor] & 0x0f;
    if (ePLabel < 7 || ePLabel > 10) {
      return 2;
    }
    nPLabel = jsonbPayloadSize(pPatch, iPCursor, &szPLabel);
    if (nPLabel == 0)
      return 2;
    iPValue = iPCursor + nPLabel + szPLabel;
    if (iPValue >= iPEnd)
      return 2;
    nPValue = jsonbPayloadSize(pPatch, iPValue, &szPValue);
    if (nPValue == 0)
      return 2;
    iPCursor = iPValue + nPValue + szPValue;
    if (iPCursor > iPEnd)
      return 2;

    iTCursor = iTStart;
    iTEnd = iTEndBE + pTarget->delta;
    while (iTCursor < iTEnd) {
      int isEqual;
      iTLabel = iTCursor;
      eTLabel = pTarget->aBlob[iTCursor] & 0x0f;
      if (eTLabel < 7 || eTLabel > 10) {
        return 1;
      }
      nTLabel = jsonbPayloadSize(pTarget, iTCursor, &szTLabel);
      if (nTLabel == 0)
        return 1;
      iTValue = iTLabel + nTLabel + szTLabel;
      if (iTValue >= iTEnd)
        return 1;
      nTValue = jsonbPayloadSize(pTarget, iTValue, &szTValue);
      if (nTValue == 0)
        return 1;
      if (iTValue + nTValue + szTValue > iTEnd)
        return 1;
      isEqual =
          jsonLabelCompare((const char *)&pPatch->aBlob[iPLabel + nPLabel], szPLabel, (ePLabel == 7 || ePLabel == 10),
                           (const char *)&pTarget->aBlob[iTLabel + nTLabel], szTLabel, (eTLabel == 7 || eTLabel == 10));
      if (isEqual)
        break;
      iTCursor = iTValue + nTValue + szTValue;
    }
    x = pPatch->aBlob[iPValue] & 0x0f;
    if (iTCursor < iTEnd) {
      if (x == 0) {
        jsonBlobEdit(pTarget, iTLabel, nTLabel + szTLabel + nTValue + szTValue, 0, 0);

        if ((pTarget->oom))
          return 3;
      } else {
        int rc, savedDelta = pTarget->delta;
        pTarget->delta = 0;
        if (iDepth >= 1000)
          return 4;
        rc = jsonMergePatch(pTarget, iTValue, pPatch, iPValue, iDepth + 1);
        if (rc)
          return rc;
        pTarget->delta += savedDelta;
      }
    } else if (x > 0) {
      u32 szNew = szPLabel + nPLabel;
      if ((pPatch->aBlob[iPValue] & 0x0f) != 12) {
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szPValue + nPValue + szNew);
        if (pTarget->oom)
          return 3;
        memcpy(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        memcpy(&pTarget->aBlob[iTEnd + szNew], &pPatch->aBlob[iPValue], szPValue + nPValue);
      } else {
        int rc, savedDelta;
        jsonBlobEdit(pTarget, iTEnd, 0, 0, szNew + 1);
        if (pTarget->oom)
          return 3;
        memcpy(&pTarget->aBlob[iTEnd], &pPatch->aBlob[iPLabel], szNew);
        pTarget->aBlob[iTEnd + szNew] = 0x00;
        savedDelta = pTarget->delta;
        pTarget->delta = 0;
        if (iDepth >= 1000)
          return 4;
        rc = jsonMergePatch(pTarget, iTEnd + szNew, pPatch, iPValue, iDepth + 1);
        if (rc)
          return rc;
        pTarget->delta += savedDelta;
      }
    }
  }
  if (pTarget->delta)
    jsonAfterEditSizeAdjust(pTarget, iTarget);
  return pTarget->oom ? 3 : 0;
}