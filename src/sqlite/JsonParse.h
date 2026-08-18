
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
  typedef struct JsonString JsonString;
  typedef struct sqlite3 sqlite3;
  typedef struct sqlite3_context sqlite3_context;
  typedef struct JsonParse JsonParse;

  struct JsonParse {
    u8 *aBlob;
    u32 nBlob;
    u32 nBlobAlloc;
    char *zJson;
    sqlite3 *db;
    int nJson;
    u32 nJPRef;
    u32 iErr;
    u16 iDepth;
    u8 nErr;
    u8 oom;
    u8 bJsonIsRCStr;
    u8 hasNonstd;
    u8 bReadOnly;

    u8 eEdit;
    int delta;
    u32 nIns;
    u32 iLabel;
    u8 *aIns;
  };

  u32 jsonTranslateBlobToText(JsonParse *, u32, JsonString *);
  void jsonParseFree(JsonParse *);
  u32 jsonbPayloadSize(const JsonParse *, u32, u32 *);
  void jsonParseReset(JsonParse * pParse);
  int jsonBlobExpand(JsonParse * pParse, u32 N);
  int jsonBlobMakeEditable(JsonParse * pParse, u32 nExtra);
  __attribute__((noinline)) void jsonBlobExpandAndAppendOneByte(JsonParse * pParse, u8 c);
  void jsonBlobAppendOneByte(JsonParse * pParse, u8 c);
  void jsonBlobAppendNode(JsonParse *, u8, u64, const void *);
  __attribute__((noinline)) void jsonBlobExpandAndAppendNode(JsonParse * pParse, u8 eType, u64 szPayload,
                                                             const void *aPayload);
  int jsonBlobChangePayloadSize(JsonParse * pParse, u32 i, u32 szPayload);
  u32 jsonbValidityCheck(const JsonParse *pParse, u32 i, u32 iEnd, u32 iDepth);
  int jsonTranslateTextToBlob(JsonParse * pParse, u32 i);
  int jsonConvertTextToBlob(JsonParse * pParse, sqlite3_context * pCtx);
  u32 jsonbArrayCount(JsonParse * pParse, u32 iRoot);
  void jsonAfterEditSizeAdjust(JsonParse * pParse, u32 iRoot);
  void jsonBlobEdit(JsonParse * pParse, u32 iDel, u32 nDel, const u8 *aIns, u32 nIns);
  u32 jsonLookupStep(JsonParse *, u32, const char *, u32);
  u32 jsonCreateEditSubstructure(JsonParse * pParse, JsonParse * pIns, const char *zTail);
  void jsonReturnFromBlob(JsonParse * pParse, u32 i, sqlite3_context * pCtx, int eMode);
  int jsonMergePatch(JsonParse * pTarget, u32 iTarget, const JsonParse *pPatch, u32 iPatch, u32 iDepth);

  extern const char *const jsonbType[17];
  extern const char jsonIsOk[256];
  void jsonPrintf(int N, JsonString *p, const char *zFormat, ...);

#ifdef __cplusplus
}
#endif
