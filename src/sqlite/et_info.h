
#pragma once

#include "sqlite/etByte.h"
typedef struct et_info et_info;

struct et_info {
  char fmttype;
  etByte base;
  etByte flags;
  etByte type;
  etByte charset;
  etByte prefix;
  char iNxt;
};


