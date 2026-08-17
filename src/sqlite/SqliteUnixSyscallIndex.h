#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*
** Index of each entry in aSyscall[] (unix_syscall.c). Restores the
** naming scheme of the original os_unix.c function-call-style macros
** (osOpen, osClose, ...), which each expanded to a cast of
** aSyscall[N].pCurrent.
*/
enum {
  SQLITE_SYSCALL_OPEN = 0,          /* osOpen */
  SQLITE_SYSCALL_CLOSE = 1,         /* osClose */
  SQLITE_SYSCALL_ACCESS = 2,        /* osAccess */
  SQLITE_SYSCALL_GETCWD = 3,        /* osGetcwd */
  SQLITE_SYSCALL_STAT = 4,          /* osStat */
  SQLITE_SYSCALL_FSTAT = 5,         /* osFstat */
  SQLITE_SYSCALL_FTRUNCATE = 6,     /* osFtruncate */
  SQLITE_SYSCALL_FCNTL = 7,         /* osFcntl */
  SQLITE_SYSCALL_READ = 8,          /* osRead */
  SQLITE_SYSCALL_PREAD = 9,         /* osPread */
  SQLITE_SYSCALL_PREAD64 = 10,      /* osPread64 */
  SQLITE_SYSCALL_WRITE = 11,        /* osWrite */
  SQLITE_SYSCALL_PWRITE = 12,       /* osPwrite */
  SQLITE_SYSCALL_PWRITE64 = 13,     /* osPwrite64 */
  SQLITE_SYSCALL_FCHMOD = 14,       /* osFchmod */
  SQLITE_SYSCALL_FALLOCATE = 15,    /* osFallocate */
  SQLITE_SYSCALL_UNLINK = 16,       /* osUnlink */
  SQLITE_SYSCALL_OPENDIRECTORY = 17, /* osOpenDirectory */
  SQLITE_SYSCALL_MKDIR = 18,        /* osMkdir */
  SQLITE_SYSCALL_RMDIR = 19,        /* osRmdir */
  SQLITE_SYSCALL_FCHOWN = 20,       /* osFchown */
  SQLITE_SYSCALL_GETEUID = 21,      /* osGeteuid */
  SQLITE_SYSCALL_MMAP = 22,         /* osMmap */
  SQLITE_SYSCALL_MUNMAP = 23,       /* osMunmap */
  SQLITE_SYSCALL_MREMAP = 24,       /* osMremap */
  SQLITE_SYSCALL_GETPAGESIZE = 25,  /* osGetpagesize */
  SQLITE_SYSCALL_READLINK = 26,     /* osReadlink */
  SQLITE_SYSCALL_LSTAT = 27,        /* osLstat */
  SQLITE_SYSCALL_IOCTL = 28,        /* osIoctl */
};

#ifdef __cplusplus
}
#endif
