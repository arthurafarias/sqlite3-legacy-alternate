
#pragma once
#ifdef __cplusplus
extern C {
#endif

#include "sqlite/i64.h"
#include "sqlite/u64.h"
#include "sqlite/u8.h"
  typedef struct sqlite3_file sqlite3_file;
  typedef struct PmaWriter PmaWriter;
  struct PmaWriter {
    int eFWErr;
    u8 *aBuffer;
    int nBuffer;
    int iBufStart;
    int iBufEnd;
    i64 iWriteOff;
    sqlite3_file *pFd;
    u64 nPmaSpill;
  };

  void vdbePmaWriteBlob(PmaWriter * p, u8 * pData, int nData);
  int vdbePmaWriterFinish(PmaWriter * p, i64 * piEof, u64 * pnSpill);
  void vdbePmaWriteVarint(PmaWriter * p, u64 iVal);

#ifdef __cplusplus
}
#endif
