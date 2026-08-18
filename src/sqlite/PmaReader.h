
#pragma once

#include "sqlite/i64.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
  typedef struct IncrMerger IncrMerger;
  typedef struct sqlite3_file sqlite3_file;

  typedef struct PmaReader PmaReader;
  struct PmaReader {
    i64 iReadOff;
    i64 iEof;
    int nAlloc;
    int nKey;
    sqlite3_file *pFd;
    u8 *aAlloc;
    u8 *aKey;
    u8 *aBuffer;
    int nBuffer;
    u8 *aMap;
    IncrMerger *pIncr;
  };

  void vdbePmaReaderClear(PmaReader * pReadr);
  int vdbePmaReadBlob(PmaReader * p, int nByte, u8 **ppOut);
  int vdbePmaReadVarint(PmaReader * p, u64 * pnOut);
  int vdbePmaReaderNext(PmaReader * pReadr);
  int vdbePmaReaderIncrInit(PmaReader * pReadr, int eMode);
  int vdbePmaReaderIncrMergeInit(PmaReader * pReadr, int eMode);


