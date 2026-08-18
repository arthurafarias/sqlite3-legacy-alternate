
#pragma once

typedef struct vxworksFileId vxworksFileId;

struct vxworksFileId {
  struct vxworksFileId *pNext;
  int nRef;
  int nName;
  char *zCanonicalName;
};


