#pragma once

#include "sqlite/u8.h"
struct aXformType_t;

struct aXformType_t {
  u8 nName;
  char zName[7];
  float rLimit;
  float rXform;
};

