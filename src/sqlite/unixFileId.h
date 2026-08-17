#pragma once
#ifdef _cplusplus
extern "C" {
#endif
#include <sys/types.h>
#include "sqlite/u64.h"
typedef struct unixFileId unixFileId;
struct unixFileId {
  dev_t dev;
  u64 ino;
};

#ifdef _cplusplus
}
#endif