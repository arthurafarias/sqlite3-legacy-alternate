
#pragma once

#include "sqlite/u32.h"
  struct JsonParse;
  struct JsonString;
  struct JsonPretty;

  struct JsonPretty {
    JsonParse *pParse;
    JsonString *pOut;
    const char *zIndent;
    u32 szIndent;
    u32 nIndent;
  };

  void jsonPrettyIndent(JsonPretty * pPretty);
  u32 jsonTranslateBlobToPrettyText(JsonPretty * pPretty, u32 i);


