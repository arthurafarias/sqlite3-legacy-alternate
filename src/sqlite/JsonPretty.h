
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/u32.h"
  typedef struct JsonParse JsonParse;
  typedef struct JsonString JsonString;
  typedef struct JsonPretty JsonPretty;

  struct JsonPretty {
    JsonParse *pParse;
    JsonString *pOut;
    const char *zIndent;
    u32 szIndent;
    u32 nIndent;
  };

  void jsonPrettyIndent(JsonPretty * pPretty);
  u32 jsonTranslateBlobToPrettyText(JsonPretty * pPretty, u32 i);

#ifdef __cplusplus
}
#endif
