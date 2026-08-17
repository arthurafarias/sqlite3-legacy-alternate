
#pragma once
#ifdef __cplusplus
extern C {
#endif

  typedef struct HashElem HashElem;

  struct HashElem {
    HashElem *next, *prev;
    void *data;
    const char *pKey;
    unsigned int h;
  };

#ifdef __cplusplus
}
#endif
