
#pragma once

struct HashElem {
  struct HashElem *next, *prev;
  void *data;
  const char *pKey;
  unsigned int h;
};
