
#pragma once

#include "sqlite/Fts5Tokenizer.h"
  typedef struct fts5_tokenizer fts5_tokenizer;

  struct fts5_tokenizer {
    int (*xCreate)(void *, const char **azArg, int nArg, Fts5Tokenizer **ppOut);
    void (*xDelete)(Fts5Tokenizer *);
    int (*xTokenize)(Fts5Tokenizer *, void *pCtx, int flags, const char *pText, int nText,
                     int (*xToken)(void *pCtx, int tflags, const char *pToken, int nToken, int iStart, int iEnd));
  };


