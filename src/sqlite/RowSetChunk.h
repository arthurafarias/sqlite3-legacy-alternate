
#pragma once

#include "sqlite/RowSetEntry.h"
struct RowSetChunk;

struct RowSetChunk {
  struct RowSetChunk *pNextChunk;
  struct RowSetEntry aEntry[((1024 - 8) / sizeof(struct RowSetEntry))];
};


