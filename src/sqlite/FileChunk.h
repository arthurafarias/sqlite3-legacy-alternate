
#pragma once

#include "sqlite/u8.h"
  typedef struct FileChunk FileChunk;
  struct FileChunk {
    FileChunk *pNext;
    u8 zChunk[8];
  };

  void memjrnlFreeChunks(FileChunk * pFirst);


