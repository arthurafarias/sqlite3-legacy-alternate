#define _GNU_SOURCE 1

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "sqlite/unix_syscall.h"

#include "sqlite/SqliteUnixSyscallIndex.h"
#include "sqlite/i64.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_int64.h"
#include "sqlite/sqlite3_mutex.h"
#include "sqlite/sqlite3_syscall_ptr.h"
#include "sqlite/u64.h"
/* Private helpers, formerly declared in _Uncategorized.h. */
static int openDirectory(const char *zFilename, int *pFd);
static int posixOpen(const char *zFile, int flags, int mode);
static int unixGetpagesize(void);
static const char *unixTempFileDir(void);

static int posixOpen(const char *zFile, int flags, int mode);
static int openDirectory(const char *, int *);
static int unixGetpagesize(void);

static int posixOpen(const char *zFile, int flags, int mode) { return open(zFile, flags, mode); }

unix_syscall aSyscall[] = {
    {"open", (sqlite3_syscall_ptr)posixOpen, 0},

    {"close", (sqlite3_syscall_ptr)close, 0},

    {"access", (sqlite3_syscall_ptr)access, 0},

    {"getcwd", (sqlite3_syscall_ptr)getcwd, 0},

    {"stat", (sqlite3_syscall_ptr)stat, 0},

    {"fstat", (sqlite3_syscall_ptr)fstat, 0},

    {"ftruncate", (sqlite3_syscall_ptr)ftruncate, 0},

    {"fcntl", (sqlite3_syscall_ptr)fcntl, 0},

    {"read", (sqlite3_syscall_ptr)read, 0},

    {"pread", (sqlite3_syscall_ptr)0, 0},

    {"pread64", (sqlite3_syscall_ptr)pread64, 0},

    {"write", (sqlite3_syscall_ptr)write, 0},

    {"pwrite", (sqlite3_syscall_ptr)0, 0},

    {"pwrite64", (sqlite3_syscall_ptr)pwrite64, 0},

    {"fchmod", (sqlite3_syscall_ptr)fchmod, 0},

    {"fallocate", (sqlite3_syscall_ptr)0, 0},

    {"unlink", (sqlite3_syscall_ptr)unlink, 0},

    {"openDirectory", (sqlite3_syscall_ptr)openDirectory, 0},

    {"mkdir", (sqlite3_syscall_ptr)mkdir, 0},

    {"rmdir", (sqlite3_syscall_ptr)rmdir, 0},

    {"fchown", (sqlite3_syscall_ptr)fchown, 0},

    {"geteuid", (sqlite3_syscall_ptr)geteuid, 0},

    {"mmap", (sqlite3_syscall_ptr)mmap, 0},

    {"munmap", (sqlite3_syscall_ptr)munmap, 0},

    {"mremap", (sqlite3_syscall_ptr)mremap, 0},

    {"getpagesize", (sqlite3_syscall_ptr)unixGetpagesize, 0},

    {"readlink", (sqlite3_syscall_ptr)readlink, 0},

    {"lstat", (sqlite3_syscall_ptr)lstat, 0},

    {"ioctl", (sqlite3_syscall_ptr)0, 0},

};

int robustFchown(int fd, uid_t uid, gid_t gid) { return ((uid_t (*)(void))aSyscall[SQLITE_SYSCALL_GETEUID].pCurrent)() ? 0 : ((int (*)(int, uid_t, gid_t))aSyscall[SQLITE_SYSCALL_FCHOWN].pCurrent)(fd, uid, gid); }

int robust_open(const char *z, int f, mode_t m) {
  int fd;
  mode_t m2 = m ? m : 0644;
  while (1) {

    fd = ((int (*)(const char *, int, int))aSyscall[SQLITE_SYSCALL_OPEN].pCurrent)(z, f | O_CLOEXEC, m2);

    if (fd < 0) {
      if (errno == EINTR)
        continue;
      break;
    }
    if (fd >= 3)
      break;
    if ((f & (O_EXCL | O_CREAT)) == (O_EXCL | O_CREAT)) {
      (void)((int (*)(const char *))aSyscall[SQLITE_SYSCALL_UNLINK].pCurrent)(z);
    }
    ((int (*)(int))aSyscall[SQLITE_SYSCALL_CLOSE].pCurrent)(fd);
    sqlite3_log(SQLITE_WARNING, "attempt to open \"%s\" as file descriptor %d", z, fd);
    fd = -1;
    if (((int (*)(const char *, int, int))aSyscall[SQLITE_SYSCALL_OPEN].pCurrent)("/dev/null", O_RDONLY, m) < 0)
      break;
  }
  if (fd >= 0) {
    if (m != 0) {
      struct stat statbuf;
      if (((int (*)(int, struct stat *))aSyscall[SQLITE_SYSCALL_FSTAT].pCurrent)(fd, &statbuf) == 0 && statbuf.st_size == 0 && (statbuf.st_mode & 0777) != m) {
        ((int (*)(int, mode_t))aSyscall[SQLITE_SYSCALL_FCHMOD].pCurrent)(fd, m);
      }
    }
  }
  return fd;
}

void unixEnterMutex(void) { sqlite3_mutex_enter(unixBigLock); }

void unixLeaveMutex(void) { sqlite3_mutex_leave(unixBigLock); }

int robust_ftruncate(int h, sqlite3_int64 sz) {
  int rc;

  do {
    rc = ((int (*)(int, off_t))aSyscall[SQLITE_SYSCALL_FTRUNCATE].pCurrent)(h, sz);
  } while (rc < 0 && errno == EINTR);
  return rc;
}

int unixLogErrorAtLine(int errcode, const char *zFunc, const char *zPath, int iLine) {
  char *zErr;
  int iErrno = errno;

  zErr = "";

  if (zPath == 0)
    zPath = "";
  sqlite3_log(errcode, "os_unix.c:%d: (%d) %s(%s) - %s", iLine, iErrno, zFunc, zPath, zErr);

  return errcode;
}

int seekAndWriteFd(int fd, i64 iOff, const void *pBuf, int nBuf, int *piErrno) {
  int rc = 0;

  nBuf &= 0x1ffff;
  ;

  do {
    rc = (int)((ssize_t (*)(int, const void *, size_t, off64_t))aSyscall[SQLITE_SYSCALL_PWRITE64].pCurrent)(fd, pBuf, nBuf, iOff);
  } while (rc < 0 && errno == EINTR);

  if (rc < 0)
    *piErrno = errno;
  return rc;
}

int full_fsync(int fd, int fullSync, int dataOnly) {
  int rc;

  (void)(fullSync);
  (void)(dataOnly);

  rc = fdatasync(fd);

  if (0 && rc != -1) {
    rc = 0;
  }
  return rc;
}

static int openDirectory(const char *zFilename, int *pFd) {
  int ii;
  int fd = -1;
  char zDirname[512 + 1];

  sqlite3_snprintf(512, zDirname, "%s", zFilename);
  for (ii = (int)strlen(zDirname); ii > 0 && zDirname[ii] != '/'; ii--)
    ;
  if (ii > 0) {
    zDirname[ii] = '\0';
  } else {
    if (zDirname[0] != '/')
      zDirname[0] = '.';
    zDirname[1] = 0;
  }
  fd = robust_open(zDirname, O_RDONLY, 0);
  if (fd >= 0) {
    ;
  }
  *pFd = fd;
  if (fd >= 0)
    return SQLITE_OK;
  return unixLogErrorAtLine(sqlite3CantopenError(44090), "openDirectory", zDirname, 44090);
}

static int unixGetpagesize(void) {

  return (int)sysconf(

      _SC_PAGESIZE

  );
}

static const char *unixTempFileDir(void) {
  unsigned int i = 0;
  struct stat buf;
  const char *zDir = sqlite3_temp_directory;

  while (1) {
    if (zDir != 0

        && ((int (*)(const char *, struct stat *))aSyscall[SQLITE_SYSCALL_STAT].pCurrent)(zDir, &buf) == 0 &&

        S_ISDIR(buf.st_mode)

        && ((int (*)(const char *, int))aSyscall[SQLITE_SYSCALL_ACCESS].pCurrent)(zDir, 03) == 0) {
      return zDir;
    }
    if (i >= sizeof(azTempDirs) / sizeof(azTempDirs[0]))
      break;
    zDir = azTempDirs[i++];
  }
  return 0;
}

int unixGetTempname(int nBuf, char *zBuf) {
  const char *zDir;
  int iLimit = 0;
  int rc = 0;

  zBuf[0] = 0;
  ;

  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  zDir = unixTempFileDir();
  if (zDir == 0) {
    rc = SQLITE_IOERR_GETTEMPPATH;
  } else {
    do {
      u64 r;
      sqlite3_randomness(sizeof(r), &r);


      zBuf[nBuf - 2] = 0;
      sqlite3_snprintf(nBuf, zBuf,
                       "%s/"
                       "etilqs_"
                       "%llx%c",
                       zDir, r, 0);
      if (zBuf[nBuf - 2] != 0 || (iLimit++) > 10) {
        rc = SQLITE_ERROR;
        break;
      }
    } while (((int (*)(const char *, int))aSyscall[SQLITE_SYSCALL_ACCESS].pCurrent)(zBuf, 0) == 0);
  }
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  return rc;
}

unix_syscall unix_syscall_stub;