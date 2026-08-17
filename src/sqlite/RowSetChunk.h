
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "sqlite/RowSetEntry.h"

typedef struct RowSetChunk RowSetChunk;

struct RowSetChunk {
  struct RowSetChunk *pNextChunk;
  struct RowSetEntry aEntry[((1024 - 8) / sizeof(struct RowSetEntry))];
};

#ifdef __cplusplus
}
#endif
