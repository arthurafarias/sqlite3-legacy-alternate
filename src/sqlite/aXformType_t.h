#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "sqlite/u8.h"
typedef struct aXformType_t aXformType_t;

struct aXformType_t {
  u8 nName;
  char zName[7];
  float rLimit;
  float rXform;
};

#ifdef __cplusplus
}
#endif