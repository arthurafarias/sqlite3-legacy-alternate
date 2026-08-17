#define _GNU_SOURCE 1

#include <string.h>

#include "sqlite/Column.h"

#include "sqlite/Mem.h"
#include "sqlite/sqlite3.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
char *sqlite3ColumnType(Column *pCol, char *zDflt) {
  if (pCol->colFlags & 0x0004) {
    return pCol->zCnName + strlen(pCol->zCnName) + 1;
  } else if (pCol->eCType) {


    return (char *)sqlite3StdType[pCol->eCType - 1];
  } else {
    return zDflt;
  }
}

const Mem *columnNullValue(void) {

  static const Mem nullMem

      = {
          {0}, (char *)0, (int)0, (u16)0x0001, (u8)0, (u8)0, (sqlite3 *)0, (int)0, (u32)0, (char *)0, (void (*)(void *))0,

      };
  return &nullMem;
}

const char *sqlite3ColumnColl(Column *pCol) {
  const char *z;
  if ((pCol->colFlags & 0x0200) == 0)
    return 0;
  z = pCol->zCnName;
  while (*z) {
    z++;
  }
  if (pCol->colFlags & 0x0004) {
    do {
      z++;
    } while (*z);
  }
  return z + 1;
}
