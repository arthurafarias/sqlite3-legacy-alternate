#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sqlite/i64.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite_int64.h"
#include "sqlite/u32.h"
#include "sqlite/u64.h"
typedef struct PmaWriter PmaWriter;

typedef struct sqlite3_file sqlite3_file;
typedef struct sqlite3_io_methods sqlite3_io_methods;

struct sqlite3_file {
  const struct sqlite3_io_methods *pMethods;
};

sqlite3_file *sqlite3_database_file_object(const char *);

void sqlite3OsClose(sqlite3_file *);
int sqlite3OsRead(sqlite3_file *, void *, int amt, i64 offset);
int sqlite3OsWrite(sqlite3_file *, const void *, int amt, i64 offset);
int sqlite3OsTruncate(sqlite3_file *, i64 size);
int sqlite3OsSync(sqlite3_file *, int);
int sqlite3OsFileSize(sqlite3_file *, i64 *pSize);
int sqlite3OsLock(sqlite3_file *, int);
int sqlite3OsUnlock(sqlite3_file *, int);
int sqlite3OsCheckReservedLock(sqlite3_file *id, int *pResOut);
int sqlite3OsFileControl(sqlite3_file *, int, void *);
void sqlite3OsFileControlHint(sqlite3_file *, int, void *);
int sqlite3OsSectorSize(sqlite3_file *id);
int sqlite3OsDeviceCharacteristics(sqlite3_file *id);
int sqlite3OsShmMap(sqlite3_file *, int, int, int, void volatile **);
int sqlite3OsShmLock(sqlite3_file *id, int, int, int);
void sqlite3OsShmBarrier(sqlite3_file *id);
int sqlite3OsShmUnmap(sqlite3_file *id, int);
int sqlite3OsFetch(sqlite3_file *id, i64, int, void **);
int sqlite3OsUnfetch(sqlite3_file *, i64, void *);
void sqlite3OsCloseFree(sqlite3_file *);
int sqlite3SectorSize(sqlite3_file *);
int sqlite3JournalIsInMemory(sqlite3_file *p);
void sqlite3MemJournalOpen(sqlite3_file *);

int unixCheckReservedLock(sqlite3_file *id, int *pResOut);
int unixLock(sqlite3_file *id, int eFileLock);
int posixUnlock(sqlite3_file *id, int eFileLock, int handleNFSUnlock);
int unixUnlock(sqlite3_file *id, int eFileLock);
int closeUnixFile(sqlite3_file *id);
int unixClose(sqlite3_file *id);
int nolockCheckReservedLock(sqlite3_file *NotUsed, int *pResOut);
int nolockLock(sqlite3_file *NotUsed, int NotUsed2);
int nolockUnlock(sqlite3_file *NotUsed, int NotUsed2);
int nolockClose(sqlite3_file *id);
int dotlockCheckReservedLock(sqlite3_file *id, int *pResOut);
int dotlockLock(sqlite3_file *id, int eFileLock);
int dotlockUnlock(sqlite3_file *id, int eFileLock);
int dotlockClose(sqlite3_file *id);
int unixRead(sqlite3_file *id, void *pBuf, int amt, sqlite3_int64 offset);
int unixWrite(sqlite3_file *id, const void *pBuf, int amt, sqlite3_int64 offset);
int unixSync(sqlite3_file *id, int flags);
int unixTruncate(sqlite3_file *id, i64 nByte);
int unixFileSize(sqlite3_file *id, i64 *pSize);
int unixFileControl(sqlite3_file *id, int op, void *pArg);
int unixSectorSize(sqlite3_file *id);
int unixDeviceCharacteristics(sqlite3_file *id);
int unixShmMap(sqlite3_file *fd, int iRegion, int szRegion, int bExtend, void volatile **pp);
int unixShmLock(sqlite3_file *fd, int ofst, int n, int flags);
void unixShmBarrier(sqlite3_file *fd);
int unixShmUnmap(sqlite3_file *fd, int deleteFlag);
int unixFetch(sqlite3_file *fd, i64 iOff, int nAmt, void **pp);
int unixUnfetch(sqlite3_file *fd, i64 iOff, void *p);
int memdbClose(sqlite3_file *);
int memdbRead(sqlite3_file *, void *, int iAmt, sqlite3_int64 iOfst);
int memdbWrite(sqlite3_file *, const void *, int iAmt, sqlite3_int64 iOfst);
int memdbTruncate(sqlite3_file *, sqlite3_int64 size);
int memdbSync(sqlite3_file *, int flags);
int memdbFileSize(sqlite3_file *, sqlite3_int64 *pSize);
int memdbLock(sqlite3_file *, int);
int memdbUnlock(sqlite3_file *, int);
int memdbFileControl(sqlite3_file *, int op, void *pArg);
int memdbDeviceCharacteristics(sqlite3_file *);
int memdbFetch(sqlite3_file *, sqlite3_int64 iOfst, int iAmt, void **pp);
int memdbUnfetch(sqlite3_file *, sqlite3_int64 iOfst, void *p);
int read32bits(sqlite3_file *fd, i64 offset, u32 *pRes);
int write32bits(sqlite3_file *fd, i64 offset, u32 val);
int readSuperJournal(sqlite3_file *pJrnl, u64 nSuper, char **pzSuper);
int backupTruncateFile(sqlite3_file *pFile, i64 iSize);
void vdbePmaWriterInit(sqlite3_file *pFd, PmaWriter *p, int nBuf, i64 iStart);
int memjrnlRead(sqlite3_file *pJfd, void *zBuf, int iAmt, sqlite_int64 iOfst);
int memjrnlTruncate(sqlite3_file *pJfd, sqlite_int64 size);
int memjrnlWrite(sqlite3_file *pJfd, const void *zBuf, int iAmt, sqlite_int64 iOfst);
int memjrnlClose(sqlite3_file *pJfd);
int memjrnlSync(sqlite3_file *pJfd, int flags);
int memjrnlFileSize(sqlite3_file *pJfd, sqlite_int64 *pSize);

#ifdef __cplusplus
}
#endif