
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "sqlite/sqlite3_syscall_ptr.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3_int64.h"
#include <sys/types.h>

typedef struct unix_syscall unix_syscall;

struct unix_syscall {
  const char *zName;
  sqlite3_syscall_ptr pCurrent;
  sqlite3_syscall_ptr pDefault;
};

  extern struct unix_syscall aSyscall[29];
  extern const char *azTempDirs[6];
  int robustFchown(int fd, uid_t uid, gid_t gid);
  int robust_open(const char *z, int f, mode_t m);
  void unixEnterMutex(void);
  void unixLeaveMutex(void);
  int robust_ftruncate(int h, sqlite3_int64 sz);
  int unixLogErrorAtLine(int errcode, const char *zFunc, const char *zPath, int iLine);
  int seekAndWriteFd(int fd, i64 iOff, const void *pBuf, int nBuf, int *piErrno);
  int full_fsync(int fd, int fullSync, int dataOnly);
  int unixGetTempname(int nBuf, char *zBuf);

#ifdef __cplusplus
}
#endif
