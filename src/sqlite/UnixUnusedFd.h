
#pragma once

  struct UnixUnusedFd;

  struct UnixUnusedFd {
    int fd;
    int flags;
    UnixUnusedFd *pNext;
  };


