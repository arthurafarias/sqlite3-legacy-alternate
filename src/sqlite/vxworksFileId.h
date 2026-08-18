
#pragma once

struct vxworksFileId;

struct vxworksFileId {
  struct vxworksFileId *pNext;
  int nRef;
  int nName;
  char *zCanonicalName;
};


