#pragma once
#ifdef __cplusplus
extern "C" {
#endif
typedef struct HashElem HashElem;

typedef struct _ht _ht;
struct _ht {
  unsigned int count;
  HashElem *chain;
};

#ifdef __cplusplus
}
#endif
