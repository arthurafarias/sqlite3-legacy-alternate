#define _GNU_SOURCE 1
#include <string.h>
#include "sqlite/sqlite3_vtab_cursor.h"
#include "sqlite/JsonEachCursor.h"
#include "sqlite/JsonParent.h"
#include "sqlite/JsonParse.h"
#include "sqlite/JsonString.h"
#include "sqlite/PragmaName.h"
#include "sqlite/PragmaVtab.h"
#include "sqlite/PragmaVtabCursor.h"
#include "sqlite/StrAccum.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_context.h"
#include "sqlite/sqlite3_destructor_type.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_stmt.h"
#include "sqlite/sqlite3_str.h"
#include "sqlite/sqlite3_uint64.h"
#include "sqlite/sqlite3_value.h"
#include "sqlite/sqlite3_vtab.h"
#include "sqlite/sqlite_int64.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/SqliteLimitCategory.h"
#include "sqlite/SqliteResultCode.h"
#include "sqlite/SqliteTextEncoding.h"
int pragmaVtabClose(sqlite3_vtab_cursor *cur) {
  PragmaVtabCursor *pCsr = (PragmaVtabCursor *)cur;
  pragmaVtabCursorClear(pCsr);
  sqlite3_free(pCsr);
  return SQLITE_OK;
}

int pragmaVtabNext(sqlite3_vtab_cursor *pVtabCursor) {
  PragmaVtabCursor *pCsr = (PragmaVtabCursor *)pVtabCursor;
  int rc = SQLITE_OK;

  pCsr->iRowid++;

  if (SQLITE_ROW != sqlite3_step(pCsr->pPragma)) {
    rc = sqlite3_finalize(pCsr->pPragma);
    pCsr->pPragma = 0;
    pragmaVtabCursorClear(pCsr);
  }
  return rc;
}

int pragmaVtabFilter(sqlite3_vtab_cursor *pVtabCursor, int idxNum, const char *idxStr, int argc, sqlite3_value **argv) {
  PragmaVtabCursor *pCsr = (PragmaVtabCursor *)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab *)(pVtabCursor->pVtab);
  int rc;
  int i, j;
  StrAccum acc;
  char *zSql;

  (void)(idxNum);
  (void)(idxStr);
  pragmaVtabCursorClear(pCsr);
  j = (pTab->pName->mPragFlg & 0x20) != 0 ? 0 : 1;
  for (i = 0; i < argc; i++, j++) {
    const char *zText = (const char *)sqlite3_value_text(argv[i]);

    if (zText) {
      pCsr->azArg[j] = sqlite3_mprintf("%s", zText);
      if (pCsr->azArg[j] == 0) {
        return SQLITE_NOMEM;
      }
    }
  }
  sqlite3StrAccumInit(&acc, 0, 0, 0, pTab->db->aLimit[SQLITE_LIMIT_SQL_LENGTH]);
  sqlite3_str_appendall(&acc, "PRAGMA ");
  if (pCsr->azArg[1]) {
    sqlite3_str_appendf(&acc, "%Q.", pCsr->azArg[1]);
  }
  sqlite3_str_appendall(&acc, pTab->pName->zName);
  if (pCsr->azArg[0]) {
    sqlite3_str_appendf(&acc, "=%Q", pCsr->azArg[0]);
  }
  zSql = sqlite3StrAccumFinish(&acc);
  if (zSql == 0)
    return SQLITE_NOMEM;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pPragma, 0);
  sqlite3_free(zSql);
  if (rc != SQLITE_OK) {
    pTab->base.zErrMsg = sqlite3_mprintf("%s", sqlite3_errmsg(pTab->db));
    return rc;
  }
  return pragmaVtabNext(pVtabCursor);
}

int pragmaVtabEof(sqlite3_vtab_cursor *pVtabCursor) {
  PragmaVtabCursor *pCsr = (PragmaVtabCursor *)pVtabCursor;
  return (pCsr->pPragma == 0);
}

int pragmaVtabColumn(sqlite3_vtab_cursor *pVtabCursor, sqlite3_context *ctx, int i) {
  PragmaVtabCursor *pCsr = (PragmaVtabCursor *)pVtabCursor;
  PragmaVtab *pTab = (PragmaVtab *)(pVtabCursor->pVtab);
  if (i < pTab->iHidden) {
    sqlite3_result_value(ctx, sqlite3_column_value(pCsr->pPragma, i));
  } else {
    sqlite3_result_text(ctx, pCsr->azArg[i - pTab->iHidden], -1, ((sqlite3_destructor_type)-1));
  }
  return SQLITE_OK;
}

int pragmaVtabRowid(sqlite3_vtab_cursor *pVtabCursor, sqlite_int64 *p) {
  PragmaVtabCursor *pCsr = (PragmaVtabCursor *)pVtabCursor;
  *p = pCsr->iRowid;
  return SQLITE_OK;
}

int jsonEachClose(sqlite3_vtab_cursor *cur) {
  JsonEachCursor *p = (JsonEachCursor *)cur;
  jsonEachCursorReset(p);

  sqlite3DbFree(p->db, cur);
  return SQLITE_OK;
}

int jsonEachEof(sqlite3_vtab_cursor *cur) {
  JsonEachCursor *p = (JsonEachCursor *)cur;
  return p->i >= p->iEnd;
}

int jsonEachNext(sqlite3_vtab_cursor *cur) {
  JsonEachCursor *p = (JsonEachCursor *)cur;
  int rc = SQLITE_OK;
  if (p->bRecursive) {
    u8 x;
    u8 levelChange = 0;
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    x = p->sParse.aBlob[i] & 0x0f;
    n = jsonbPayloadSize(&p->sParse, i, &sz);
    if (x == 12 || x == 11) {
      JsonParent *pParent;
      if (p->nParent >= p->nParentAlloc) {
        JsonParent *pNew;
        u64 nNew;
        nNew = p->nParentAlloc * 2 + 3;
        pNew = (JsonParent*)(sqlite3DbRealloc(p->db, p->aParent, sizeof(JsonParent) * nNew));
        if (pNew == 0)
          return SQLITE_NOMEM;
        p->nParentAlloc = (u32)nNew;
        p->aParent = pNew;
      }
      levelChange = 1;
      pParent = &p->aParent[p->nParent];
      pParent->iHead = p->i;
      pParent->iValue = i;
      pParent->iEnd = i + n + sz;
      pParent->iKey = -1;
      pParent->nPath = (u32)p->path.nUsed;
      if (p->eType && p->nParent) {
        jsonAppendPathName(p);
        if (p->path.eErr)
          rc = SQLITE_NOMEM;
      }
      p->nParent++;
      p->i = i + n;
    } else {
      p->i = i + n + sz;
    }
    while (p->nParent > 0 && p->i >= p->aParent[p->nParent - 1].iEnd) {
      p->nParent--;
      p->path.nUsed = p->aParent[p->nParent].nPath;
      levelChange = 1;
    }
    if (levelChange) {
      if (p->nParent > 0) {
        JsonParent *pParent = &p->aParent[p->nParent - 1];
        u32 iVal = pParent->iValue;
        p->eType = p->sParse.aBlob[iVal] & 0x0f;
      } else {
        p->eType = 0;
      }
    }
  } else {
    u32 n, sz = 0;
    u32 i = jsonSkipLabel(p);
    n = jsonbPayloadSize(&p->sParse, i, &sz);
    p->i = i + n + sz;
  }
  if (p->eType == 11 && p->nParent) {
    p->aParent[p->nParent - 1].iKey++;
  }
  p->iRowid++;
  return rc;
}

int jsonEachColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, int iColumn) {
  JsonEachCursor *p = (JsonEachCursor *)cur;
  switch (iColumn) {
    case 0: {
      if (p->nParent == 0) {
        u32 n, j;
        if (p->nRoot == 1)
          break;
        j = jsonEachPathLength(p);
        n = p->nRoot - j;
        if (n == 0) {
          break;
        } else if (p->path.zBuf[j] == '[') {
          i64 x;
          sqlite3Atoi64(&p->path.zBuf[j + 1], &x, n - 1, SQLITE_UTF8);
          sqlite3_result_int64(ctx, x);
        } else if (p->path.zBuf[j + 1] == '"') {
          sqlite3_result_text(ctx, &p->path.zBuf[j + 2], n - 3, ((sqlite3_destructor_type)-1));
        } else {
          sqlite3_result_text(ctx, &p->path.zBuf[j + 1], n - 1, ((sqlite3_destructor_type)-1));
        }
        break;
      }
      if (p->eType == 12) {
        jsonReturnFromBlob(&p->sParse, p->i, ctx, 1);
      } else {
        sqlite3_result_int64(ctx, p->aParent[p->nParent - 1].iKey);
      }
      break;
    }
    case 1: {
      u32 i = jsonSkipLabel(p);
      jsonReturnFromBlob(&p->sParse, i, ctx, p->eMode);
      if ((p->sParse.aBlob[i] & 0x0f) >= 11) {
        sqlite3_result_subtype(ctx, 74);
      }
      break;
    }
    case 2: {
      u32 i = jsonSkipLabel(p);
      u8 eType = p->sParse.aBlob[i] & 0x0f;
      sqlite3_result_text(ctx, jsonbType[eType], -1, ((sqlite3_destructor_type)0));
      break;
    }
    case 3: {
      u32 i = jsonSkipLabel(p);
      if ((p->sParse.aBlob[i] & 0x0f) < 11) {
        jsonReturnFromBlob(&p->sParse, i, ctx, 1);
      }
      break;
    }
    case 4: {
      sqlite3_result_int64(ctx, (sqlite3_int64)p->i);
      break;
    }
    case 5: {
      if (p->nParent > 0 && p->bRecursive) {
        sqlite3_result_int64(ctx, p->aParent[p->nParent - 1].iHead);
      }
      break;
    }
    case 6: {
      u64 nBase = p->path.nUsed;
      if (p->nParent)
        jsonAppendPathName(p);
      sqlite3_result_text64(ctx, p->path.zBuf, p->path.nUsed, ((sqlite3_destructor_type)-1), SQLITE_UTF8);
      p->path.nUsed = nBase;
      break;
    }
    case 7: {
      u32 n = jsonEachPathLength(p);
      sqlite3_result_text64(ctx, p->path.zBuf, n, ((sqlite3_destructor_type)-1), SQLITE_UTF8);
      break;
    }
    default: {
      sqlite3_result_text(ctx, p->path.zBuf, p->nRoot, ((sqlite3_destructor_type)0));
      break;
    }
    case 8: {
      if (p->sParse.zJson == 0) {
        sqlite3_result_blob(ctx, p->sParse.aBlob, p->sParse.nBlob, ((sqlite3_destructor_type)-1));
      } else {
        sqlite3_result_text(ctx, p->sParse.zJson, -1, ((sqlite3_destructor_type)-1));
      }
      break;
    }
  }
  return SQLITE_OK;
}

int jsonEachRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid) {
  JsonEachCursor *p = (JsonEachCursor *)cur;
  *pRowid = p->iRowid;
  return SQLITE_OK;
}

int jsonEachFilter(sqlite3_vtab_cursor *cur, int idxNum, const char *idxStr, int argc, sqlite3_value **argv) {
  JsonEachCursor *p = (JsonEachCursor *)cur;
  const char *zRoot = 0;
  u32 i, n, sz;

  (void)(idxStr);
  (void)(argc);
  jsonEachCursorReset(p);
  if (idxNum == 0)
    return SQLITE_OK;
  memset(&p->sParse, 0, sizeof(p->sParse));
  p->sParse.nJPRef = 1;
  p->sParse.db = p->db;
  if (jsonArgIsJsonb(argv[0], &p->sParse)) {
  } else {
    p->sParse.zJson = (char *)sqlite3_value_text(argv[0]);
    p->sParse.nJson = sqlite3_value_bytes(argv[0]);
    if (p->sParse.zJson == 0) {
      p->i = p->iEnd = 0;
      return SQLITE_OK;
    }
    if (jsonConvertTextToBlob(&p->sParse, 0)) {
      if (p->sParse.oom) {
        return SQLITE_NOMEM;
      }
      goto json_each_malformed_input;
    }
  }
  if (idxNum == 3) {
    zRoot = (const char *)sqlite3_value_text(argv[1]);
    if (zRoot == 0)
      return SQLITE_OK;
    if (zRoot[0] != '$') {
      sqlite3_free(cur->pVtab->zErrMsg);
      cur->pVtab->zErrMsg = jsonBadPathError(0, zRoot, 0);
      jsonEachCursorReset(p);
      return cur->pVtab->zErrMsg ? SQLITE_ERROR : SQLITE_NOMEM;
    }
    p->nRoot = sqlite3Strlen30(zRoot);
    if (zRoot[1] == 0) {
      i = p->i = 0;
      p->eType = 0;
    } else {
      i = jsonLookupStep(&p->sParse, 0, zRoot + 1, 0);
      if (((i) >= 0xfffffffb)) {
        if (i == 0xfffffffe) {
          p->i = 0;
          p->eType = 0;
          p->iEnd = 0;
          return SQLITE_OK;
        }
        sqlite3_free(cur->pVtab->zErrMsg);
        cur->pVtab->zErrMsg = jsonBadPathError(0, zRoot, 0);
        jsonEachCursorReset(p);
        return cur->pVtab->zErrMsg ? SQLITE_ERROR : SQLITE_NOMEM;
      }
      if (p->sParse.iLabel) {
        p->i = p->sParse.iLabel;
        p->eType = 12;
      } else {
        p->i = i;
        p->eType = 11;
      }
    }
    jsonAppendRaw(&p->path, zRoot, p->nRoot);
  } else {
    i = p->i = 0;
    p->eType = 0;
    p->nRoot = 1;
    jsonAppendRaw(&p->path, "$", 1);
  }
  p->nParent = 0;
  n = jsonbPayloadSize(&p->sParse, i, &sz);
  p->iEnd = i + n + sz;
  if ((p->sParse.aBlob[i] & 0x0f) >= 11 && !p->bRecursive) {
    p->i = i + n;
    p->eType = p->sParse.aBlob[i] & 0x0f;
    p->aParent = (JsonParent*)(sqlite3DbMallocZero(p->db, sizeof(JsonParent)));
    if (p->aParent == 0)
      return SQLITE_NOMEM;
    p->nParent = 1;
    p->nParentAlloc = 1;
    p->aParent[0].iKey = 0;
    p->aParent[0].iEnd = p->iEnd;
    p->aParent[0].iHead = p->i;
    p->aParent[0].iValue = i;
  }
  return SQLITE_OK;

json_each_malformed_input:
  sqlite3_free(cur->pVtab->zErrMsg);
  cur->pVtab->zErrMsg = sqlite3_mprintf("malformed JSON");
  jsonEachCursorReset(p);
  return cur->pVtab->zErrMsg ? SQLITE_ERROR : SQLITE_NOMEM;
}
