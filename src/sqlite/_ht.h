#pragma once

struct HashElem;

struct _ht;
struct _ht {
  unsigned int count;
  HashElem *chain;
};


