
#pragma once

#include "sqlite/u8.h"
  struct SubrtnSig;

  struct SubrtnSig {
    int selId;
    u8 bComplete;
    char *zAff;
    int iTable;
    int iAddr;
    int regReturn;
  };


