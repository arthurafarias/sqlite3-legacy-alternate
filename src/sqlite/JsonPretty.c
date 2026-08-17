#define _GNU_SOURCE 1

#include "sqlite/JsonPretty.h"

#include "sqlite/JsonParse.h"
#include "sqlite/JsonString.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
void jsonPrettyIndent(JsonPretty *pPretty) {
  u32 jj;
  for (jj = 0; jj < pPretty->nIndent; jj++) {
    jsonAppendRaw(pPretty->pOut, pPretty->zIndent, pPretty->szIndent);
  }
}

u32 jsonTranslateBlobToPrettyText(JsonPretty *pPretty, u32 i) {
  u32 sz, n, j, iEnd;
  JsonParse *pParse = pPretty->pParse;
  JsonString *pOut = pPretty->pOut;
  n = jsonbPayloadSize(pParse, i, &sz);
  if (n == 0) {
    pOut->eErr |= 0x02;
    return pParse->nBlob + 1;
  }
  switch (pParse->aBlob[i] & 0x0f) {
  case 11: {
    j = i + n;
    iEnd = j + sz;
    jsonAppendChar(pOut, '[');
    if (j < iEnd) {
      jsonAppendChar(pOut, '\n');
      pPretty->nIndent++;
      if (pPretty->nIndent >= 1000) {
        jsonStringTooDeep(pOut);
      }
      while (pOut->eErr == 0) {
        jsonPrettyIndent(pPretty);
        j = jsonTranslateBlobToPrettyText(pPretty, j);
        if (j >= iEnd)
          break;
        jsonAppendRawNZ(pOut, ",\n", 2);
      }
      jsonAppendChar(pOut, '\n');
      pPretty->nIndent--;
      jsonPrettyIndent(pPretty);
    }
    jsonAppendChar(pOut, ']');
    i = iEnd;
    break;
  }
  case 12: {
    j = i + n;
    iEnd = j + sz;
    jsonAppendChar(pOut, '{');
    if (j < iEnd) {
      jsonAppendChar(pOut, '\n');
      pPretty->nIndent++;
      if (pPretty->nIndent >= 1000) {
        jsonStringTooDeep(pOut);
      }
      pParse->iDepth = pPretty->nIndent;
      while (pOut->eErr == 0) {
        jsonPrettyIndent(pPretty);
        j = jsonTranslateBlobToText(pParse, j, pOut);
        if (j > iEnd) {
          pOut->eErr |= 0x02;
          break;
        }
        jsonAppendRawNZ(pOut, ": ", 2);
        j = jsonTranslateBlobToPrettyText(pPretty, j);
        if (j >= iEnd)
          break;
        jsonAppendRawNZ(pOut, ",\n", 2);
      }
      jsonAppendChar(pOut, '\n');
      pPretty->nIndent--;
      jsonPrettyIndent(pPretty);
    }
    jsonAppendChar(pOut, '}');
    i = iEnd;
    break;
  }
  default: {
    i = jsonTranslateBlobToText(pParse, i, pOut);
    break;
  }
  }
  return i;
}
