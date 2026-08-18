
#pragma once

  typedef struct HashElem HashElem;

  struct HashElem {
    HashElem *next, *prev;
    void *data;
    const char *pKey;
    unsigned int h;
  };


