
#pragma once

#include "sqlite/RowSetEntry.h"
typedef struct RowSetChunk RowSetChunk;

struct RowSetChunk {
  struct RowSetChunk *pNextChunk;
  struct RowSetEntry aEntry[((1024 - 8) / sizeof(struct RowSetEntry))];
};


