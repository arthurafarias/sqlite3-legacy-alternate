#pragma once
#ifdef __cplusplus
extern "C" {
#endif
typedef struct sqlite3_io_methods sqlite3_io_methods;
typedef struct sqlite3_file sqlite3_file;
typedef struct sqlite3_filetypedef sqlite3_filetypedef;

#include "sqlite/sqlite3_int64.h"
struct sqlite3_io_methods {
  int iVersion;
  int (*xClose)(sqlite3_file *);
  int (*xRead)(sqlite3_file *, void *, int iAmt, sqlite3_int64 iOfst);
  int (*xWrite)(sqlite3_file *, const void *, int iAmt, sqlite3_int64 iOfst);
  int (*xTruncate)(sqlite3_file *, sqlite3_int64 size);
  int (*xSync)(sqlite3_file *, int flags);
  int (*xFileSize)(sqlite3_file *, sqlite3_int64 *pSize);
  int (*xLock)(sqlite3_file *, int);
  int (*xUnlock)(sqlite3_file *, int);
  int (*xCheckReservedLock)(sqlite3_file *, int *pResOut);
  int (*xFileControl)(sqlite3_file *, int op, void *pArg);
  int (*xSectorSize)(sqlite3_file *);
  int (*xDeviceCharacteristics)(sqlite3_file *);

  int (*xShmMap)(sqlite3_file *, int iPg, int pgsz, int, void volatile **);
  int (*xShmLock)(sqlite3_file *, int offset, int n, int flags);
  void (*xShmBarrier)(sqlite3_file *);
  int (*xShmUnmap)(sqlite3_file *, int deleteFlag);

  int (*xFetch)(sqlite3_file *, sqlite3_int64 iOfst, int iAmt, void **pp);
  int (*xUnfetch)(sqlite3_file *, sqlite3_int64 iOfst, void *p);
};

extern const sqlite3_io_methods posixIoMethods;
extern const sqlite3_io_methods nolockIoMethods;
extern const sqlite3_io_methods dotlockIoMethods;
extern const sqlite3_io_methods memdb_io_methods;
extern const struct sqlite3_io_methods MemJournalMethods;

#ifdef __cplusplus
}
#endif