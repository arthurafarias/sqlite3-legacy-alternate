#pragma once

struct MemStore;

struct sqlite3_vfs;
struct MemVfs;
struct MemFile;

struct MemFS;

struct MemFS {
  int nMemStore;
  MemStore **apMemStore;
};

extern MemFS memdb_g;


