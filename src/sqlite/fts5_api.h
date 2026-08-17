
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/fts5_extension_function.h"
#include "sqlite/fts5_tokenizer.h"
#include "sqlite/fts5_tokenizer_v2.h"
  typedef struct fts5_api fts5_api;

  struct fts5_api {
    int iVersion;

    int (*xCreateTokenizer)(fts5_api *pApi, const char *zName, void *pUserData, fts5_tokenizer *pTokenizer,
                            void (*xDestroy)(void *));

    int (*xFindTokenizer)(fts5_api *pApi, const char *zName, void **ppUserData, fts5_tokenizer *pTokenizer);

    int (*xCreateFunction)(fts5_api *pApi, const char *zName, void *pUserData, fts5_extension_function xFunction,
                           void (*xDestroy)(void *));

    int (*xCreateTokenizer_v2)(fts5_api *pApi, const char *zName, void *pUserData, fts5_tokenizer_v2 *pTokenizer,
                               void (*xDestroy)(void *));

    int (*xFindTokenizer_v2)(fts5_api *pApi, const char *zName, void **ppUserData, fts5_tokenizer_v2 **ppTokenizer);
  };

#ifdef __cplusplus
}
#endif
