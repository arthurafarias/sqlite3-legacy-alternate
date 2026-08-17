
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
typedef struct vxworksFileId vxworksFileId;

struct vxworksFileId {
  struct vxworksFileId *pNext;
  int nRef;
  int nName;
  char *zCanonicalName;
};

#ifdef __cplusplus
}
#endif
