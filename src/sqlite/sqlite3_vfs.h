#pragma once

#ifdef __cplusplus

#extern "C" {
#endif

#include "sqlite/Btree.h"
#include "sqlite/Pager.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3_backup.h"
#include "sqlite/sqlite3_filename.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_syscall_ptr.h"
typedef struct Wal Wal;

typedef struct sqlite3_vfs sqlite3_vfs;
typedef struct sqlite3_file sqlite3_file;
typedef struct sqlite3 sqlite3;
typedef struct PgHdr DbPage;

struct sqlite3_vfs {
  int iVersion;
  int szOsFile;
  int mxPathname;
  sqlite3_vfs *pNext;
  const char *zName;
  void *pAppData;

  int (*xOpen)(sqlite3_vfs *, sqlite3_filename zName, sqlite3_file *, int flags, int *pOutFlags);
  int (*xDelete)(sqlite3_vfs *, const char *zName, int syncDir);
  int (*xAccess)(sqlite3_vfs *, const char *zName, int flags, int *pResOut);
  int (*xFullPathname)(sqlite3_vfs *, const char *zName, int nOut, char *zOut);
  void *(*xDlOpen)(sqlite3_vfs *, const char *zFilename);
  void (*xDlError)(sqlite3_vfs *, int nByte, char *zErrMsg);
  void (*(*xDlSym)(sqlite3_vfs *, void *, const char *zSymbol))(void);
  void (*xDlClose)(sqlite3_vfs *, void *);
  int (*xRandomness)(sqlite3_vfs *, int nByte, char *zOut);
  int (*xSleep)(sqlite3_vfs *, int microseconds);
  int (*xCurrentTime)(sqlite3_vfs *, double *);
  int (*xGetLastError)(sqlite3_vfs *, int, char *);

  int (*xCurrentTimeInt64)(sqlite3_vfs *, sqlite3_int64 *);

  int (*xSetSystemCall)(sqlite3_vfs *, const char *zName, sqlite3_syscall_ptr);
  sqlite3_syscall_ptr (*xGetSystemCall)(sqlite3_vfs *, const char *zName);
  const char *(*xNextSystemCall)(sqlite3_vfs *, const char *zName);
};

sqlite3_vfs *sqlite3_vfs_find(const char *zVfsName);
int sqlite3_vfs_register(sqlite3_vfs *, int makeDflt);
int sqlite3_vfs_unregister(sqlite3_vfs *);

int sqlite3OsOpen(sqlite3_vfs *, const char *, sqlite3_file *, int, int *);
int sqlite3OsDelete(sqlite3_vfs *, const char *, int);
int sqlite3OsAccess(sqlite3_vfs *, const char *, int, int *pResOut);
int sqlite3OsFullPathname(sqlite3_vfs *, const char *, int, char *);
void *sqlite3OsDlOpen(sqlite3_vfs *, const char *);
void sqlite3OsDlError(sqlite3_vfs *, int, char *);
void (*sqlite3OsDlSym(sqlite3_vfs *, void *, const char *))(void);
void sqlite3OsDlClose(sqlite3_vfs *, void *);
int sqlite3OsRandomness(sqlite3_vfs *, int, char *);
int sqlite3OsSleep(sqlite3_vfs *, int);
int sqlite3OsGetLastError(sqlite3_vfs *);
int sqlite3OsCurrentTimeInt64(sqlite3_vfs *, sqlite3_int64 *);
int sqlite3OsOpenMalloc(sqlite3_vfs *, const char *, sqlite3_file **, int, int *);
int sqlite3PagerOpen(sqlite3_vfs *, Pager **ppPager, const char *, int, int, int, void (*)(DbPage *));
int sqlite3BtreeOpen(sqlite3_vfs *pVfs, const char *zFilename, sqlite3 *db, Btree **ppBtree, int flags, int vfsFlags);
int sqlite3JournalOpen(sqlite3_vfs *, const char *, sqlite3_file *, int, int);
int sqlite3JournalSize(sqlite3_vfs *);

int sqlite3IsMemdb(const sqlite3_vfs *);
extern sqlite3_vfs *vfsList;
void vfsUnlink(sqlite3_vfs *pVfs);
int unixSetSystemCall(sqlite3_vfs *pNotUsed, const char *zName, sqlite3_syscall_ptr pNewFunc);
sqlite3_syscall_ptr unixGetSystemCall(sqlite3_vfs *pNotUsed, const char *zName);
const char *unixNextSystemCall(sqlite3_vfs *p, const char *zName);
int unixSleep(sqlite3_vfs *, int);
int fillInUnixFile(sqlite3_vfs *pVfs, int h, sqlite3_file *pId, const char *zFilename, int ctrlFlags);
int unixOpen(sqlite3_vfs *pVfs, const char *zPath, sqlite3_file *pFile, int flags, int *pOutFlags);
int unixDelete(sqlite3_vfs *NotUsed, const char *zPath, int dirSync);
int unixAccess(sqlite3_vfs *NotUsed, const char *zPath, int flags, int *pResOut);
int unixFullPathname(sqlite3_vfs *pVfs, const char *zPath, int nOut, char *zOut);
void *unixDlOpen(sqlite3_vfs *NotUsed, const char *zFilename);
void unixDlError(sqlite3_vfs *NotUsed, int nBuf, char *zBufOut);
void (*unixDlSym(sqlite3_vfs *NotUsed, void *p, const char *zSym))(void);
void unixDlClose(sqlite3_vfs *NotUsed, void *pHandle);
int unixRandomness(sqlite3_vfs *NotUsed, int nBuf, char *zBuf);
int unixCurrentTimeInt64(sqlite3_vfs *NotUsed, sqlite3_int64 *piNow);
int unixCurrentTime(sqlite3_vfs *NotUsed, double *prNow);
int unixGetLastError(sqlite3_vfs *NotUsed, int NotUsed2, char *NotUsed3);
int memdbOpen(sqlite3_vfs *, const char *, sqlite3_file *, int, int *);
int memdbAccess(sqlite3_vfs *, const char *zName, int flags, int *);
int memdbFullPathname(sqlite3_vfs *, const char *zName, int, char *zOut);
void *memdbDlOpen(sqlite3_vfs *, const char *zFilename);
void memdbDlError(sqlite3_vfs *, int nByte, char *zErrMsg);
void (*memdbDlSym(sqlite3_vfs *pVfs, void *p, const char *zSym))(void);
void memdbDlClose(sqlite3_vfs *, void *);
int memdbRandomness(sqlite3_vfs *, int nByte, char *zOut);
int memdbSleep(sqlite3_vfs *, int microseconds);
int memdbGetLastError(sqlite3_vfs *, int, char *);
int memdbCurrentTimeInt64(sqlite3_vfs *, sqlite3_int64 *);
extern sqlite3_vfs memdb_vfs;
int sqlite3WalOpen(sqlite3_vfs *, sqlite3_file *, const char *, int, i64, Wal **);

#ifdef __cplusplus
}
#endif
