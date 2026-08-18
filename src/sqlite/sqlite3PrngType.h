#pragma once

#include "sqlite/u32.h"
#include "sqlite/u8.h"
struct sqlite3PrngType;

struct sqlite3PrngType {
  u32 s[16];
  u8 out[64];
  u8 n;
};


