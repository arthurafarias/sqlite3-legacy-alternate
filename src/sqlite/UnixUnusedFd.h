
#pragma once

  typedef struct UnixUnusedFd UnixUnusedFd;

  struct UnixUnusedFd {
    int fd;
    int flags;
    UnixUnusedFd *pNext;
  };


