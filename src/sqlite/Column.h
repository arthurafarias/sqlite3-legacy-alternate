
#pragma once
#ifdef __cplusplus
extern C {
#endif
#include "sqlite/u16.h"
#include "sqlite/u8.h"
  typedef struct sqlite3_value Mem;

  typedef struct Column Column;

  struct Column {
    char *zCnName;
    unsigned notNull : 4;
    unsigned eCType : 4;
    char affinity;
    u8 szEst;
    u8 hName;
    u16 iDflt;
    u16 colFlags;
  };

  char *sqlite3ColumnType(Column *, char *);
  const char *sqlite3ColumnColl(Column *);
  const Mem *columnNullValue(void);
  char sqlite3AffinityType(const char *, Column *);

#ifdef __cplusplus
}
#endif
