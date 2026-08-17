
#pragma once
#ifdef __cplusplus
extern C {
#endif

  typedef struct fts5_tokenizer_v2 fts5_tokenizer_v2;

#include "sqlite/Fts5Tokenizer.h"

  typedef struct fts5_tokenizer_v2 fts5_tokenizer_v2;
  struct fts5_tokenizer_v2 {
    int iVersion;

    int (*xCreate)(void *, const char **azArg, int nArg, Fts5Tokenizer **ppOut);
    void (*xDelete)(Fts5Tokenizer *);
    int (*xTokenize)(Fts5Tokenizer *, void *pCtx, int flags, const char *pText, int nText, const char *pLocale, int nLocale, int (*xToken)(void *pCtx, int tflags, const char *pToken, int nToken, int iStart, int iEnd));
  };

#ifdef __cplusplus
}
#endif
