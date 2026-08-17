#pragma once
#ifdef __cplusplus
extern "C" {
#endif
typedef struct sqlite3_index_orderby sqlite3_index_orderby;
struct sqlite3_index_orderby {
  int iColumn;
  unsigned char desc;
};

#ifdef __cplusplus
}
#endif
