#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "sqlite/u32.h"
#include "sqlite/u8.h"

typedef struct sqlite3PrngType sqlite3PrngType;

struct sqlite3PrngType {
  u32 s[16];
  u8 out[64];
  u8 n;
};

#ifdef __cplusplus
}
#endif
