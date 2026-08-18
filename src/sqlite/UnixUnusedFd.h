
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
  typedef struct UnixUnusedFd UnixUnusedFd;

  struct UnixUnusedFd {
    int fd;
    int flags;
    UnixUnusedFd *pNext;
  };

#ifdef __cplusplus
}
#endif
