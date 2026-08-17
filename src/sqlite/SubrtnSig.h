
#pragma once
#ifdef __cplusplus
extern C {
#endif

#include "sqlite/u8.h"
  typedef struct SubrtnSig SubrtnSig;

  struct SubrtnSig {
    int selId;
    u8 bComplete;
    char *zAff;
    int iTable;
    int iAddr;
    int regReturn;
  };

#ifdef __cplusplus
}
#endif
