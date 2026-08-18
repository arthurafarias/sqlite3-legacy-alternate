
#pragma once

#include "sqlite/i64.h"
typedef struct RowSetEntry RowSetEntry;

struct RowSetEntry {
  i64 v;
  struct RowSetEntry *pRight;
  struct RowSetEntry *pLeft;
};

struct RowSetEntry *rowSetEntryMerge(struct RowSetEntry *pA, struct RowSetEntry *pB);
struct RowSetEntry *rowSetEntrySort(struct RowSetEntry *pIn);
void rowSetTreeToList(struct RowSetEntry *pIn, struct RowSetEntry **ppFirst, struct RowSetEntry **ppLast);
struct RowSetEntry *rowSetNDeepTree(struct RowSetEntry **ppList, int iDepth);
struct RowSetEntry *rowSetListToTree(struct RowSetEntry *pList);


