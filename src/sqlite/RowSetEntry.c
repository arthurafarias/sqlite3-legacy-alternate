#define _GNU_SOURCE 1

#include <string.h>

#include "sqlite/RowSetEntry.h"

#include "sqlite/i64.h"
struct RowSetEntry *rowSetEntryMerge(struct RowSetEntry *pA, struct RowSetEntry *pB) {
  struct RowSetEntry head;
  struct RowSetEntry *pTail;

  pTail = &head;

  for (;;) {




    if (pA->v <= pB->v) {
      if (pA->v < pB->v)
        pTail = pTail->pRight = pA;
      pA = pA->pRight;
      if (pA == 0) {
        pTail->pRight = pB;
        break;
      }
    } else {
      pTail = pTail->pRight = pB;
      pB = pB->pRight;
      if (pB == 0) {
        pTail->pRight = pA;
        break;
      }
    }
  }
  return head.pRight;
}

struct RowSetEntry *rowSetEntrySort(struct RowSetEntry *pIn) {
  unsigned int i;
  struct RowSetEntry *pNext, *aBucket[40];

  memset(aBucket, 0, sizeof(aBucket));
  while (pIn) {
    pNext = pIn->pRight;
    pIn->pRight = 0;
    for (i = 0; aBucket[i]; i++) {
      pIn = rowSetEntryMerge(aBucket[i], pIn);
      aBucket[i] = 0;
    }
    aBucket[i] = pIn;
    pIn = pNext;
  }
  pIn = aBucket[0];
  for (i = 1; i < sizeof(aBucket) / sizeof(aBucket[0]); i++) {
    if (aBucket[i] == 0)
      continue;
    pIn = pIn ? rowSetEntryMerge(pIn, aBucket[i]) : aBucket[i];
  }
  return pIn;
}

void rowSetTreeToList(struct RowSetEntry *pIn, struct RowSetEntry **ppFirst, struct RowSetEntry **ppLast) {

  if (pIn->pLeft) {
    struct RowSetEntry *p;
    rowSetTreeToList(pIn->pLeft, ppFirst, &p);
    p->pRight = pIn;
  } else {
    *ppFirst = pIn;
  }
  if (pIn->pRight) {
    rowSetTreeToList(pIn->pRight, &pIn->pRight, ppLast);
  } else {
    *ppLast = pIn;
  }
}

struct RowSetEntry *rowSetNDeepTree(struct RowSetEntry **ppList, int iDepth) {
  struct RowSetEntry *p;
  struct RowSetEntry *pLeft;
  if (*ppList == 0) {

    return 0;
  }
  if (iDepth > 1) {

    pLeft = rowSetNDeepTree(ppList, iDepth - 1);
    p = *ppList;
    if (p == 0) {

      return pLeft;
    }
    p->pLeft = pLeft;
    *ppList = p->pRight;
    p->pRight = rowSetNDeepTree(ppList, iDepth - 1);
  } else {
    p = *ppList;
    *ppList = p->pRight;
    p->pLeft = p->pRight = 0;
  }
  return p;
}

struct RowSetEntry *rowSetListToTree(struct RowSetEntry *pList) {
  int iDepth;
  struct RowSetEntry *p;
  struct RowSetEntry *pLeft;

  p = pList;
  pList = p->pRight;
  p->pLeft = p->pRight = 0;
  for (iDepth = 1; pList; iDepth++) {
    pLeft = p;
    p = pList;
    pList = p->pRight;
    p->pLeft = pLeft;
    p->pRight = rowSetNDeepTree(&pList, iDepth);
  }
  return p;
}
