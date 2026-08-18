
#pragma once

#include "sqlite/u32.h"
#include "sqlite/u8.h"
  struct Bitvec;
  struct Bitvec {
    u32 iSize;
    u32 nSet;

    u32 iDivisor;

    union {
      u8 aBitmap[((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(u8))];
      u32 aHash[((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(u32))];
      Bitvec *apSub[((u32)((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(Bitvec *)))];
    } u;
  };

  Bitvec *sqlite3BitvecCreate(u32);
  int sqlite3BitvecTest(Bitvec *, u32);
  int sqlite3BitvecTestNotNull(Bitvec *, u32);
  int sqlite3BitvecSet(Bitvec *, u32);
  void sqlite3BitvecClear(Bitvec *, u32, void *);
  void sqlite3BitvecDestroy(Bitvec *);
  u32 sqlite3BitvecSize(Bitvec *);
  int sqlite3BitvecBuiltinTest(int, int *);


