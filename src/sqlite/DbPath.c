#define _GNU_SOURCE 1
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "sqlite/DbPath.h"
#include "sqlite/SqliteUnixSyscallIndex.h"
#include "sqlite/sqlite3.h"
#include "sqlite/sqlite3_syscall_ptr.h"
#include "sqlite/unix_syscall.h"
#include "sqlite/SqliteResultCode.h"
void appendOnePathElement(DbPath *pPath, const char *zName, int nName) {
  if (zName[0] == '.') {
    if (nName == 1)
      return;
    if (zName[1] == '.' && nName == 2) {
      if (pPath->nUsed > 1) {
        while (pPath->zOut[--pPath->nUsed] != '/') {
        }
      }
      return;
    }
  }
  if (pPath->nUsed + nName + 2 >= pPath->nOut) {
    pPath->rc = SQLITE_ERROR;
    return;
  }
  pPath->zOut[pPath->nUsed++] = '/';
  memcpy(&pPath->zOut[pPath->nUsed], zName, nName);
  pPath->nUsed += nName;

  if (pPath->rc == SQLITE_OK) {
    const char *zIn;
    struct stat buf;
    pPath->zOut[pPath->nUsed] = 0;
    zIn = pPath->zOut;
    if (((int (*)(const char *, struct stat *))aSyscall[SQLITE_SYSCALL_LSTAT].pCurrent)(zIn, &buf) != 0) {
      if ((*__errno_location()) != 2) {
        pPath->rc = unixLogErrorAtLine(sqlite3CantopenError(47152), "lstat", zIn, 47152);
      }
    } else if (((((buf.st_mode)) & 0170000) == (0120000))) {
      ssize_t got;
      char zLnk[4096 + 2];
      if (pPath->nSymlink++ > 200) {
        pPath->rc = sqlite3CantopenError(47158);
        return;
      }
      got = ((ssize_t (*)(const char *, char *, size_t))aSyscall[SQLITE_SYSCALL_READLINK].pCurrent)(zIn, zLnk,
                                                                                                    sizeof(zLnk) - 2);
      if (got <= 0 || got >= (ssize_t)sizeof(zLnk) - 2) {
        pPath->rc = unixLogErrorAtLine(sqlite3CantopenError(47163), "readlink", zIn, 47163);
        return;
      }
      zLnk[got] = 0;
      if (zLnk[0] == '/') {
        pPath->nUsed = 0;
      } else {
        pPath->nUsed -= nName + 1;
      }
      appendAllPathElements(pPath, zLnk);
    }
  }
}

void appendAllPathElements(DbPath *pPath, const char *zPath) {
  int i = 0;
  int j = 0;
  do {
    while (zPath[i] && zPath[i] != '/') {
      i++;
    }
    if (i > j) {
      appendOnePathElement(pPath, &zPath[j], i - j);
    }
    j = i + 1;
  } while (zPath[i++]);
}
