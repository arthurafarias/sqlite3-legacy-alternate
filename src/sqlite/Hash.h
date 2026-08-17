
#pragma once
#ifdef __cplusplus
extern C {
#endif

  typedef struct HashElem HashElem;
  typedef struct Hash Hash;
  struct Hash {
    unsigned int htsize;
    unsigned int count;
    HashElem *first;
    struct _ht {
      unsigned int count;
      HashElem *chain;
    } *ht;
  };

  void sqlite3HashInit(Hash *);
  void *sqlite3HashInsert(Hash *, const char *pKey, void *pData);
  void *sqlite3HashFind(const Hash *, const char *pKey);
  void sqlite3HashClear(Hash *);

  void insertElement(Hash * pH, struct _ht * pEntry, HashElem * pNew);
  int rehash(Hash * pH, unsigned int new_size);
  HashElem *findElementWithHash(const Hash *pH, const char *pKey, unsigned int *pHash);
  void removeElement(Hash * pH, HashElem * elem);

#ifdef __cplusplus
}
#endif
