
#pragma once
#ifdef __cplusplus
extern C {
#endif

  typedef struct Fts5PhraseIter Fts5PhraseIter;

  struct Fts5PhraseIter {
    const unsigned char *a;
    const unsigned char *b;
  };

#ifdef __cplusplus
}
#endif
