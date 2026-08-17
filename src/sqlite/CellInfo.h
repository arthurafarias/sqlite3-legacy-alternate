
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/i64.h"
#include "sqlite/u16.h"
#include "sqlite/u32.h"
#include "sqlite/u8.h"
  typedef struct CellInfo CellInfo;

  struct CellInfo {
    i64 nKey;
    u8 *pPayload;
    u32 nPayload;
    u16 nLocal;
    u16 nSize;
  };

#ifdef __cplusplus
}
#endif
