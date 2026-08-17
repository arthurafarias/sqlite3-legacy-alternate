#define _GNU_SOURCE 1

#include "sqlite/JsonEachCursor.h"

#include "sqlite/JsonParent.h"
#include "sqlite/JsonParse.h"
#include "sqlite/JsonString.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
void jsonEachCursorReset(JsonEachCursor *p) {
  jsonParseReset(&p->sParse);
  jsonStringReset(&p->path);
  sqlite3DbFree(p->db, p->aParent);
  p->iRowid = 0;
  p->i = 0;
  p->aParent = 0;
  p->nParent = 0;
  p->nParentAlloc = 0;
  p->iEnd = 0;
  p->eType = 0;
}

int jsonSkipLabel(JsonEachCursor *p) {
  if (p->eType == 12) {
    u32 sz = 0;
    u32 n = jsonbPayloadSize(&p->sParse, p->i, &sz);
    sz += p->i + n;
    if (sz >= p->sParse.nBlob)
      sz = p->i;
    return sz;
  } else {
    return p->i;
  }
}

void jsonAppendPathName(JsonEachCursor *p) {

  if (p->eType == 11) {
    jsonPrintf(30, &p->path, "[%lld]", p->aParent[p->nParent - 1].iKey);
  } else {
    u32 n, sz = 0, k, i;
    const char *z;
    int needQuote = 0;
    n = jsonbPayloadSize(&p->sParse, p->i, &sz);
    k = p->i + n;
    z = (const char *)&p->sParse.aBlob[k];
    if (sz == 0 || !(sqlite3CtypeMap[(unsigned char)(z[0])] & 0x02)) {
      needQuote = 1;
    } else {
      for (i = 0; i < sz; i++) {
        if (!(sqlite3CtypeMap[(unsigned char)(z[i])] & 0x06)) {
          needQuote = 1;
          break;
        }
      }
    }
    if (needQuote) {
      jsonPrintf(sz + 4, &p->path, ".\"%.*s\"", sz, z);
    } else {
      jsonPrintf(sz + 2, &p->path, ".%.*s", sz, z);
    }
  }
}

int jsonEachPathLength(JsonEachCursor *p) {
  u32 n = p->path.nUsed;
  char *z = p->path.zBuf;
  if (p->iRowid == 0 && p->bRecursive && n >= 2) {
    while (n > 1) {
      n--;
      if (z[n] == '[' || z[n] == '.') {
        u32 x, sz = 0;
        char cSaved = z[n];
        z[n] = 0;


        x = jsonLookupStep(&p->sParse, 0, z + 1, 0);
        z[n] = cSaved;
        if (((x) >= 0xfffffffb))
          continue;
        if (x + jsonbPayloadSize(&p->sParse, x, &sz) == p->i)
          break;
      }
    }
  }
  return n;
}
