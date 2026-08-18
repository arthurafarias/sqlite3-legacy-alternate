#define _GNU_SOURCE 1
#include <string.h>
#include "sqlite/FpDecode.h"
#include "sqlite/sqlite3.h"
#include "sqlite/u16.h"
#include "sqlite/u64.h"
int countLeadingZeros(u64 m) {
  return __builtin_clzll(m);
}

void sqlite3FpDecode(FpDecode *p, double r, int iRound, int mxRound) {
  int i;
  int n;
  u64 v;
  int e, exp = 0;
  char *zBuf;
  char *z;

  p->isSpecial = 0;

  if (r < 0.0) {
    p->sign = '-';
    r = -r;
  } else if (r == 0.0) {
    p->sign = '+';
    p->n = 1;
    p->iDP = 1;
    p->z = (char*)("0");
    return;
  } else {
    p->sign = '+';
  }
  memcpy(&v, &r, 8);
  e = (v >> 52) & 0x7ff;
  if (e == 0x7ff) {
    p->isSpecial = 1 + (v != 0x7ff0000000000000LL);
    p->n = 0;
    p->iDP = 0;
    p->z = p->zBuf;
    return;
  }
  v &= 0x000fffffffffffffULL;
  if (e == 0) {
    int nn = countLeadingZeros(v);
    v <<= nn;
    e = -1074 - nn;
  } else {
    v = (v << 11) | (((u64)1) << (63));
    e -= 1086;
  }
  sqlite3Fp2Convert10(v, e, (iRound <= 0 || iRound >= 18) ? 18 : iRound + 1, &v, &exp);

  zBuf = p->zBuf;

  i = 20;
  while (v >= 10) {
    int kk = (v % 100) * 2;

    *(u16 *)(&zBuf[i - 2]) = *(u16 *)&sqlite3DigitPairs.a[kk];
    i -= 2;
    v /= 100;
  }
  if (v) {
    zBuf[--i] = v + '0';
  }

  n = 20 - i;

  p->iDP = n + exp;
  if (iRound <= 0) {
    iRound = p->iDP - iRound;
    if (iRound == 0 && zBuf[i] >= '5') {
      iRound = 1;
      zBuf[--i] = '0';
      n++;
      p->iDP++;
    }
  }
  z = &zBuf[i];
  if (iRound > 0 && (iRound < n || n > mxRound)) {
    if (iRound > mxRound)
      iRound = mxRound;
    if (iRound == 17) {
      if (z[15] == '9' && z[14] == '9') {
        int jj, kk;
        u64 v2;
        for (jj = 14; jj > 0 && z[jj - 1] == '9'; jj--) {
        }
        if (jj == 0) {
          v2 = 1;
        } else {
          v2 = z[0] - '0';
          for (kk = 1; kk < jj; kk++)
            v2 = (v2 * 10) + z[kk] - '0';
          v2++;
        }
        if (r == sqlite3Fp10Convert2(v2, exp + n - jj)) {
          iRound = jj + 1;
        }
      } else if (p->iDP >= n || (z[15] == '0' && z[14] == '0' && z[13] == '0')) {
        int jj, kk;
        u64 v2;

        for (jj = 13; z[jj - 1] == '0'; jj--) {
        }
        v2 = z[0] - '0';
        for (kk = 1; kk < jj; kk++)
          v2 = (v2 * 10) + z[kk] - '0';
        if (r == sqlite3Fp10Convert2(v2, exp + n - jj)) {
          iRound = jj + 1;
        }
      }
    }
    n = iRound;
    if (z[iRound] >= '5') {
      int j = iRound - 1;
      while (1) {
        z[j]++;
        if (z[j] <= '9')
          break;
        z[j] = '0';
        if (j == 0) {
          z--;
          z[0] = '1';
          n++;
          p->iDP++;
          break;
        } else {
          j--;
        }
      }
    }
  }

  while (z[n - 1] == '0') {
    n--;
  }
  p->n = n;
  p->z = z;
}