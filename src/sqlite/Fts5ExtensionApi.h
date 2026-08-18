
#pragma once

#include "sqlite/Fts5Context.h"
#include "sqlite/Fts5PhraseIter.h"
#include "sqlite/sqlite3_int64.h"
  typedef struct Fts5ExtensionApi Fts5ExtensionApi;

  struct Fts5ExtensionApi {
    int iVersion;

    void *(*xUserData)(Fts5Context *);

    int (*xColumnCount)(Fts5Context *);
    int (*xRowCount)(Fts5Context *, sqlite3_int64 *pnRow);
    int (*xColumnTotalSize)(Fts5Context *, int iCol, sqlite3_int64 *pnToken);

    int (*xTokenize)(Fts5Context *, const char *pText, int nText, void *pCtx,
                     int (*xToken)(void *, int, const char *, int, int, int));

    int (*xPhraseCount)(Fts5Context *);
    int (*xPhraseSize)(Fts5Context *, int iPhrase);

    int (*xInstCount)(Fts5Context *, int *pnInst);
    int (*xInst)(Fts5Context *, int iIdx, int *piPhrase, int *piCol, int *piOff);

    sqlite3_int64 (*xRowid)(Fts5Context *);
    int (*xColumnText)(Fts5Context *, int iCol, const char **pz, int *pn);
    int (*xColumnSize)(Fts5Context *, int iCol, int *pnToken);

    int (*xQueryPhrase)(Fts5Context *, int iPhrase, void *pUserData,
                        int (*)(const Fts5ExtensionApi *, Fts5Context *, void *));
    int (*xSetAuxdata)(Fts5Context *, void *pAux, void (*xDelete)(void *));
    void *(*xGetAuxdata)(Fts5Context *, int bClear);

    int (*xPhraseFirst)(Fts5Context *, int iPhrase, Fts5PhraseIter *, int *, int *);
    void (*xPhraseNext)(Fts5Context *, Fts5PhraseIter *, int *piCol, int *piOff);

    int (*xPhraseFirstColumn)(Fts5Context *, int iPhrase, Fts5PhraseIter *, int *);
    void (*xPhraseNextColumn)(Fts5Context *, Fts5PhraseIter *, int *piCol);

    int (*xQueryToken)(Fts5Context *, int iPhrase, int iToken, const char **ppToken, int *pnToken);
    int (*xInstToken)(Fts5Context *, int iIdx, int iToken, const char **, int *);

    int (*xColumnLocale)(Fts5Context *, int iCol, const char **pz, int *pn);
    int (*xTokenize_v2)(Fts5Context *, const char *pText, int nText, const char *pLocale, int nLocale, void *pCtx,
                        int (*xToken)(void *, int, const char *, int, int, int));
  };


