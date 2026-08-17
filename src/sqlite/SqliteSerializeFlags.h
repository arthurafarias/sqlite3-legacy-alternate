#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Flags for sqlite3_serialize (from sqlite3.h) */
enum {
  SQLITE_SERIALIZE_NOCOPY = 0x001,  /* Do no memory allocations */
};

#ifdef __cplusplus
}
#endif
