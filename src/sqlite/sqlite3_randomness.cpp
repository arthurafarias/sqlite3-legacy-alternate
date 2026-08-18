#define _GNU_SOURCE 1
#include "sqlite/sqlite3_randomness.h"
#include <string.h>
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/sqlite3_vfs.h"
#include "sqlite/sqlite3PrngType.h"
#include "sqlite/u32.h"
#include "sqlite/SqliteMutexType.h"
static void chacha_block(u32 *out, const u32 *in) {
  int i;
  u32 x[16];
  memcpy(x, in, 64);
  for (i = 0; i < 10; i++) {
    (x[0] += x[4], x[12] ^= x[0], x[12] = (((x[12]) << (16)) | ((x[12]) >> (32 - (16)))), x[8] += x[12], x[4] ^= x[8],
     x[4] = (((x[4]) << (12)) | ((x[4]) >> (32 - (12)))), x[0] += x[4], x[12] ^= x[0],
     x[12] = (((x[12]) << (8)) | ((x[12]) >> (32 - (8)))), x[8] += x[12], x[4] ^= x[8],
     x[4] = (((x[4]) << (7)) | ((x[4]) >> (32 - (7)))));
    (x[1] += x[5], x[13] ^= x[1], x[13] = (((x[13]) << (16)) | ((x[13]) >> (32 - (16)))), x[9] += x[13], x[5] ^= x[9],
     x[5] = (((x[5]) << (12)) | ((x[5]) >> (32 - (12)))), x[1] += x[5], x[13] ^= x[1],
     x[13] = (((x[13]) << (8)) | ((x[13]) >> (32 - (8)))), x[9] += x[13], x[5] ^= x[9],
     x[5] = (((x[5]) << (7)) | ((x[5]) >> (32 - (7)))));
    (x[2] += x[6], x[14] ^= x[2], x[14] = (((x[14]) << (16)) | ((x[14]) >> (32 - (16)))), x[10] += x[14], x[6] ^= x[10],
     x[6] = (((x[6]) << (12)) | ((x[6]) >> (32 - (12)))), x[2] += x[6], x[14] ^= x[2],
     x[14] = (((x[14]) << (8)) | ((x[14]) >> (32 - (8)))), x[10] += x[14], x[6] ^= x[10],
     x[6] = (((x[6]) << (7)) | ((x[6]) >> (32 - (7)))));
    (x[3] += x[7], x[15] ^= x[3], x[15] = (((x[15]) << (16)) | ((x[15]) >> (32 - (16)))), x[11] += x[15], x[7] ^= x[11],
     x[7] = (((x[7]) << (12)) | ((x[7]) >> (32 - (12)))), x[3] += x[7], x[15] ^= x[3],
     x[15] = (((x[15]) << (8)) | ((x[15]) >> (32 - (8)))), x[11] += x[15], x[7] ^= x[11],
     x[7] = (((x[7]) << (7)) | ((x[7]) >> (32 - (7)))));
    (x[0] += x[5], x[15] ^= x[0], x[15] = (((x[15]) << (16)) | ((x[15]) >> (32 - (16)))), x[10] += x[15], x[5] ^= x[10],
     x[5] = (((x[5]) << (12)) | ((x[5]) >> (32 - (12)))), x[0] += x[5], x[15] ^= x[0],
     x[15] = (((x[15]) << (8)) | ((x[15]) >> (32 - (8)))), x[10] += x[15], x[5] ^= x[10],
     x[5] = (((x[5]) << (7)) | ((x[5]) >> (32 - (7)))));
    (x[1] += x[6], x[12] ^= x[1], x[12] = (((x[12]) << (16)) | ((x[12]) >> (32 - (16)))), x[11] += x[12], x[6] ^= x[11],
     x[6] = (((x[6]) << (12)) | ((x[6]) >> (32 - (12)))), x[1] += x[6], x[12] ^= x[1],
     x[12] = (((x[12]) << (8)) | ((x[12]) >> (32 - (8)))), x[11] += x[12], x[6] ^= x[11],
     x[6] = (((x[6]) << (7)) | ((x[6]) >> (32 - (7)))));
    (x[2] += x[7], x[13] ^= x[2], x[13] = (((x[13]) << (16)) | ((x[13]) >> (32 - (16)))), x[8] += x[13], x[7] ^= x[8],
     x[7] = (((x[7]) << (12)) | ((x[7]) >> (32 - (12)))), x[2] += x[7], x[13] ^= x[2],
     x[13] = (((x[13]) << (8)) | ((x[13]) >> (32 - (8)))), x[8] += x[13], x[7] ^= x[8],
     x[7] = (((x[7]) << (7)) | ((x[7]) >> (32 - (7)))));
    (x[3] += x[4], x[14] ^= x[3], x[14] = (((x[14]) << (16)) | ((x[14]) >> (32 - (16)))), x[9] += x[14], x[4] ^= x[9],
     x[4] = (((x[4]) << (12)) | ((x[4]) >> (32 - (12)))), x[3] += x[4], x[14] ^= x[3],
     x[14] = (((x[14]) << (8)) | ((x[14]) >> (32 - (8)))), x[9] += x[14], x[4] ^= x[9],
     x[4] = (((x[4]) << (7)) | ((x[4]) >> (32 - (7)))));
  }
  for (i = 0; i < 16; i++)
    out[i] = x[i] + in[i];
}

void sqlite3_randomness(int N, void *pBuf) {
  unsigned char *zBuf = (unsigned char*)(pBuf);

  sqlite3_mutex *mutex;

  if (sqlite3_initialize())
    return;

  mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_PRNG);

  sqlite3_mutex_enter(mutex);
  if (N <= 0 || pBuf == 0) {
    sqlite3Prng.s[0] = 0;
    sqlite3_mutex_leave(mutex);
    return;
  }

  if (sqlite3Prng.s[0] == 0) {
    sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
    static const u32 chacha20_init[] = {0x61707865, 0x3320646e, 0x79622d32, 0x6b206574};
    memcpy(&sqlite3Prng.s[0], chacha20_init, 16);
    if (pVfs == 0) {
      memset(&sqlite3Prng.s[4], 0, 44);
    } else {
      sqlite3OsRandomness(pVfs, 44, (char *)&sqlite3Prng.s[4]);
    }
    sqlite3Prng.s[15] = sqlite3Prng.s[12];
    sqlite3Prng.s[12] = 0;
    sqlite3Prng.n = 0;
  }

  while (1) {
    if (N <= sqlite3Prng.n) {
      memcpy(zBuf, &sqlite3Prng.out[sqlite3Prng.n - N], N);
      sqlite3Prng.n -= N;
      break;
    }
    if (sqlite3Prng.n > 0) {
      memcpy(zBuf, sqlite3Prng.out, sqlite3Prng.n);
      N -= sqlite3Prng.n;
      zBuf += sqlite3Prng.n;
    }
    sqlite3Prng.s[12]++;
    chacha_block((u32 *)sqlite3Prng.out, sqlite3Prng.s);
    sqlite3Prng.n = 64;
  }
  sqlite3_mutex_leave(mutex);
}
