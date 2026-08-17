#define _GNU_SOURCE 1
#include <string.h>
#include "sqlite/Hash.h"
#include "sqlite/HashElem.h"
#include "sqlite/sqlite3.h"
#include "sqlite/u64.h"
/* Private helpers, formerly declared in _Uncategorized.h. */
static unsigned int strHash(const char *z);

static unsigned int strHash(const char *z) {
  unsigned int h = 0;
  while (z[0]) {
    h += 0xdf & (unsigned char)*(z++);

    h *= 0x9e3779b1;
  }
  return h;
}

void sqlite3HashInit(Hash *pNew) {
  pNew->first = 0;
  pNew->count = 0;
  pNew->htsize = 0;
  pNew->ht = 0;
}

void sqlite3HashClear(Hash *pH) {
  HashElem *elem;

  elem = pH->first;
  pH->first = 0;
  sqlite3_free(pH->ht);
  pH->ht = 0;
  pH->htsize = 0;
  while (elem) {
    HashElem *next_elem = elem->next;
    sqlite3_free(elem);
    elem = next_elem;
  }
  pH->count = 0;
}

void insertElement(Hash *pH, struct _ht *pEntry, HashElem *pNew) {
  HashElem *pHead;
  if (pEntry) {
    pHead = pEntry->count ? pEntry->chain : 0;
    pEntry->count++;
    pEntry->chain = pNew;
  } else {
    pHead = 0;
  }
  if (pHead) {
    pNew->next = pHead;
    pNew->prev = pHead->prev;
    if (pHead->prev) {
      pHead->prev->next = pNew;
    } else {
      pH->first = pNew;
    }
    pHead->prev = pNew;
  } else {
    pNew->next = pH->first;
    if (pH->first) {
      pH->first->prev = pNew;
    }
    pNew->prev = 0;
    pH->first = pNew;
  }
}

int rehash(Hash *pH, unsigned int new_size) {
  struct _ht *new_ht;
  HashElem *elem, *next_elem;

  if (new_size * sizeof(struct _ht) > 1024) {
    new_size = 1024 / sizeof(struct _ht);
  }
  if (new_size == pH->htsize)
    return 0;

  sqlite3BeginBenignMalloc();
  new_ht = (struct _ht *)sqlite3Malloc(new_size * sizeof(struct _ht));
  sqlite3EndBenignMalloc();

  if (new_ht == 0)
    return 0;
  sqlite3_free(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_size = sqlite3MallocSize(new_ht) / sizeof(struct _ht);
  memset(new_ht, 0, new_size * sizeof(struct _ht));
  for (elem = pH->first, pH->first = 0; elem; elem = next_elem) {
    next_elem = elem->next;
    insertElement(pH, &new_ht[elem->h % new_size], elem);
  }
  return 1;
}

HashElem *findElementWithHash(const Hash *pH, const char *pKey, unsigned int *pHash) {
  HashElem *elem;
  unsigned int count;
  unsigned int h;
  static HashElem nullElement = {0, 0, 0, 0, 0};

  h = strHash(pKey);
  if (pH->ht) {
    struct _ht *pEntry;
    pEntry = &pH->ht[h % pH->htsize];
    elem = pEntry->chain;
    count = pEntry->count;
  } else {
    elem = pH->first;
    count = pH->count;
  }
  if (pHash)
    *pHash = h;
  while (count) {
    if (h == elem->h && sqlite3StrICmp(elem->pKey, pKey) == 0) {
      return elem;
    }
    elem = elem->next;
    count--;
  }
  return &nullElement;
}

void removeElement(Hash *pH, HashElem *elem) {
  struct _ht *pEntry;
  if (elem->prev) {
    elem->prev->next = elem->next;
  } else {
    pH->first = elem->next;
  }
  if (elem->next) {
    elem->next->prev = elem->prev;
  }
  if (pH->ht) {
    pEntry = &pH->ht[elem->h % pH->htsize];
    if (pEntry->chain == elem) {
      pEntry->chain = elem->next;
    }

    pEntry->count--;
  }
  sqlite3_free(elem);
  pH->count--;
  if (pH->count == 0) {
    sqlite3HashClear(pH);
  }
}

void *sqlite3HashFind(const Hash *pH, const char *pKey) {
  return findElementWithHash(pH, pKey, 0)->data;
}

void *sqlite3HashInsert(Hash *pH, const char *pKey, void *data) {
  unsigned int h;
  HashElem *elem;
  HashElem *new_elem;

  elem = findElementWithHash(pH, pKey, &h);
  if (elem->data) {
    void *old_data = elem->data;
    if (data == 0) {
      removeElement(pH, elem);
    } else {
      elem->data = data;
      elem->pKey = pKey;
    }
    return old_data;
  }
  if (data == 0)
    return 0;
  new_elem = (HashElem *)sqlite3Malloc(sizeof(HashElem));
  if (new_elem == 0)
    return data;
  new_elem->pKey = pKey;
  new_elem->h = h;
  new_elem->data = data;
  pH->count++;
  if (pH->count >= 5 && pH->count > 2 * pH->htsize) {
    rehash(pH, pH->count * 3);
  }
  insertElement(pH, pH->ht ? &pH->ht[new_elem->h % pH->htsize] : 0, new_elem);
  return 0;
}