#pragma once
#ifdef __cplusplus
extern "C" {
#endif

typedef struct MemStore MemStore;

typedef struct sqlite3_vfs MemVfs;
typedef struct MemFile MemFile;

typedef struct MemFS MemFS;

struct MemFS {
  int nMemStore;
  MemStore **apMemStore;
};

extern MemFS memdb_g;

#ifdef __cplusplus
}
#endif
