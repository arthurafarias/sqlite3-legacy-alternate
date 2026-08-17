#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sqlite/u64.h"
#include "sqlite/u8.h"

typedef struct PragmaName {
  const char *const zName;
  u8 ePragTyp;
  u8 mPragFlg;
  u8 iPragCName;
  u8 nPragCName;
  u64 iArg;
} PragmaName;

#ifdef __cplusplus
}
#endif