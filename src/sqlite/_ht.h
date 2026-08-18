#pragma once

typedef struct HashElem HashElem;

typedef struct _ht _ht;
struct _ht {
  unsigned int count;
  HashElem *chain;
};


