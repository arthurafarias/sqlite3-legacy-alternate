
#pragma once

#include "sqlite/Hash.h"
#include "sqlite/u16.h"
#include "sqlite/u8.h"
  struct Table;

  struct Schema;

  struct Schema {
    int schema_cookie;
    int iGeneration;
    Hash tblHash;
    Hash idxHash;
    Hash trigHash;
    Hash fkeyHash;
    Table *pSeqTab;
    u8 file_format;
    u8 enc;
    u16 schemaFlags;
    int cache_size;
  };

  void sqlite3SchemaClear(void *);


