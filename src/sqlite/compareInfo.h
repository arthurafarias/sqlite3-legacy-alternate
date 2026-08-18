
#pragma once

#include "sqlite/u8.h"
typedef struct compareInfo compareInfo;

struct compareInfo {
  u8 matchAll;
  u8 matchOne;
  u8 matchSet;
  u8 noCase;
};

extern const struct compareInfo globInfo;
extern const struct compareInfo likeInfoNorm;
extern const struct compareInfo likeInfoAlt;


