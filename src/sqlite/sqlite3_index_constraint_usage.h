#pragma once
#ifdef __cplusplus
extern "C" {
#endif

typedef struct sqlite3_index_constraint_usage sqlite3_index_constraint_usage;
struct sqlite3_index_constraint_usage {
  int argvIndex;
  unsigned char omit;
};

#ifdef __cplusplus
}
#endif
