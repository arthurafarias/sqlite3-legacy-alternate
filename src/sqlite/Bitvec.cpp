#define _GNU_SOURCE 1
#include <string.h>
#include "sqlite/Bitvec.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_uint64.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
#include "sqlite/SqliteResultCode.h"
Bitvec *sqlite3BitvecCreate(u32 iSize) {
  Bitvec *p;

  p = (Bitvec*)(sqlite3MallocZero(sizeof(*p)));
  if (p) {
    p->iSize = iSize;
  }
  return p;
}

int sqlite3BitvecTestNotNull(Bitvec *p, u32 i) {
  i--;
  if (i >= p->iSize)
    return 0;
  while (p->iDivisor) {
    u32 bin = i / p->iDivisor;
    i = i % p->iDivisor;
    p = p->u.apSub[bin];
    if (!p) {
      return 0;
    }
  }
  if (p->iSize <= (((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(u8)) * 8)) {
    return (p->u.aBitmap[i / 8] & (1 << (i & (8 - 1)))) != 0;
  } else {
    u32 h = (((i++) * 1) % ((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(u32)));
    while (p->u.aHash[h]) {
      if (p->u.aHash[h] == i)
        return 1;
      h = (h + 1) % ((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(u32));
    }
    return 0;
  }
}

int sqlite3BitvecTest(Bitvec *p, u32 i) {
  return p != 0 && sqlite3BitvecTestNotNull(p, i);
}

int sqlite3BitvecSet(Bitvec *p, u32 i) {
  u32 h;
  if (p == 0)
    return 0;

  i--;
  while ((p->iSize > (((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(u8)) * 8)) &&
         p->iDivisor) {
    u32 bin = i / p->iDivisor;
    i = i % p->iDivisor;
    if (p->u.apSub[bin] == 0) {
      p->u.apSub[bin] = sqlite3BitvecCreate(p->iDivisor);
      if (p->u.apSub[bin] == 0)
        return 7;
    }
    p = p->u.apSub[bin];
  }
  if (p->iSize <= (((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(u8)) * 8)) {
    p->u.aBitmap[i / 8] |= 1 << (i & (8 - 1));
    return SQLITE_OK;
  }
  h = (((i++) * 1) % ((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(u32)));

  if (!p->u.aHash[h]) {
    if (p->nSet < (((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(u32)) - 1)) {
      goto bitvec_set_end;
    } else {
      goto bitvec_set_rehash;
    }
  }

  do {
    if (p->u.aHash[h] == i)
      return SQLITE_OK;
    h++;
    if (h >= ((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(u32)))
      h = 0;
  } while (p->u.aHash[h]);

bitvec_set_rehash:
  if (p->nSet >= (((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(u32)) / 2)) {
    unsigned int j;
    int rc;
    u32 *aiValues = (u32*)(sqlite3DbMallocRaw(0, sizeof(p->u.aHash)));
    if (aiValues == 0) {
      return 7;
    } else {
      memcpy(aiValues, p->u.aHash, sizeof(p->u.aHash));
      memset(p->u.apSub, 0, sizeof(p->u.apSub));
      p->iDivisor =
          p->iSize / ((u32)((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(Bitvec *)));
      if ((p->iSize %
           ((u32)((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(Bitvec *)))) != 0)
        p->iDivisor++;
      if (p->iDivisor < (((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(u8)) * 8))
        p->iDivisor = (((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(u8)) * 8);
      rc = sqlite3BitvecSet(p, i);
      for (j = 0; j < ((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(u32)); j++) {
        if (aiValues[j])
          rc |= sqlite3BitvecSet(p, aiValues[j]);
      }
      sqlite3DbFree(0, aiValues);
      return rc;
    }
  }
bitvec_set_end:
  p->nSet++;
  p->u.aHash[h] = i;
  return SQLITE_OK;
}

void sqlite3BitvecClear(Bitvec *p, u32 i, void *pBuf) {
  if (p == 0)
    return;

  i--;
  while (p->iDivisor) {
    u32 bin = i / p->iDivisor;
    i = i % p->iDivisor;
    p = p->u.apSub[bin];
    if (!p) {
      return;
    }
  }
  if (p->iSize <= (((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(u8)) * 8)) {
    p->u.aBitmap[i / 8] &= ~(u8)(1 << (i & (8 - 1)));
  } else {
    unsigned int j;
    u32 *aiValues = (u32*)(pBuf);
    memcpy(aiValues, p->u.aHash, sizeof(p->u.aHash));
    memset(p->u.aHash, 0, sizeof(p->u.aHash));
    p->nSet = 0;
    for (j = 0; j < ((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(u32)); j++) {
      if (aiValues[j] && aiValues[j] != (i + 1)) {
        u32 h = (((aiValues[j] - 1) * 1) %
                 ((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(u32)));
        p->nSet++;
        while (p->u.aHash[h]) {
          h++;
          if (h >= ((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(u32)))
            h = 0;
        }
        p->u.aHash[h] = aiValues[j];
      }
    }
  }
}

void sqlite3BitvecDestroy(Bitvec *p) {
  if (p == 0)
    return;
  if (p->iDivisor) {
    unsigned int i;
    for (i = 0; i < ((u32)((((512 - (3 * sizeof(u32))) / sizeof(Bitvec *)) * sizeof(Bitvec *)) / sizeof(Bitvec *)));
         i++) {
      sqlite3BitvecDestroy(p->u.apSub[i]);
    }
  }
  sqlite3_free(p);
}

u32 sqlite3BitvecSize(Bitvec *p) {
  return p->iSize;
}

int sqlite3BitvecBuiltinTest(int sz, int *aOp) {
  Bitvec *pBitvec = 0;
  unsigned char *pV = 0;
  int rc = -1;
  int i, nx, pc, op;
  void *pTmpSpace;

  if (sz <= 0) {
    pBitvec = sqlite3BitvecCreate(2 * (unsigned)(-sz));
    pV = 0;
  } else {
    pBitvec = sqlite3BitvecCreate(sz);
    pV = (unsigned char*)(sqlite3MallocZero((7 + (i64)sz) / 8 + 1));
  }
  pTmpSpace = sqlite3_malloc64(512);
  if (pBitvec == 0 || pTmpSpace == 0 || (pV == 0 && sz > 0))
    goto bitvec_end;

  sqlite3BitvecSet(0, 1);
  sqlite3BitvecClear(0, 1, pTmpSpace);

  pc = i = 0;
  while ((op = aOp[pc]) != 0) {
    if (op >= 6) {
      pc++;
      continue;
    }
    switch (op) {
      case 1:
      case 2:
      case 5: {
        nx = 4;
        i = aOp[pc + 2] - 1;
        aOp[pc + 2] += aOp[pc + 3];
        break;
      }
      case 3:
      case 4:
      default: {
        nx = 2;
        sqlite3_randomness(sizeof(i), &i);
        break;
      }
    }
    if ((--aOp[pc + 1]) > 0)
      nx = 0;
    pc += nx;
    i = (i & 0x7fffffff) % sz;
    if ((op & 1) != 0) {
      if (pV)
        pV[(i + 1) >> 3] |= (1 << ((i + 1) & 7));
      if (op != 5) {
        if (sqlite3BitvecSet(pBitvec, i + 1))
          goto bitvec_end;
      }
    } else {
      if (pV)
        pV[(i + 1) >> 3] &= ~(u8)(1 << ((i + 1) & 7));
      sqlite3BitvecClear(pBitvec, i + 1, pTmpSpace);
    }
  }

  if (pV) {
    rc = sqlite3BitvecTest(0, 0) + sqlite3BitvecTest(pBitvec, sz + 1) + sqlite3BitvecTest(pBitvec, 0) +
         (sqlite3BitvecSize(pBitvec) - sz);
    for (i = 1; i <= sz; i++) {
      if (((pV[i >> 3] & (1 << (i & 7))) != 0) != sqlite3BitvecTest(pBitvec, i)) {
        rc = i;
        break;
      }
    }
  } else {
    rc = 0;
  }

bitvec_end:
  sqlite3_free(pTmpSpace);
  sqlite3_free(pV);
  sqlite3BitvecDestroy(pBitvec);
  return rc;
}
